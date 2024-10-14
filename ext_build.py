#!/usr/bin/env python3
import argparse
import os
import subprocess
from os import path as osp
from typing import Optional

import cmake
import pybind11

# CMAKE binary
CMAKE_BIN = "cmake"
if os.name == "nt":
    CMAKE_BIN = f"{CMAKE_BIN}.exe"
if os.name != "nt":
    import ninja

    # setup ninja path
    os.environ["PATH"] += f"{os.pathsep}{ninja.BIN_DIR}"
CMAKE_BIN = osp.join(cmake.CMAKE_BIN_DIR, CMAKE_BIN)

# `pybind11-config.cmake` dir
PYBIND11_CMAKE_DIR = pybind11.get_cmake_dir()

PROJECT_ROOT = osp.abspath(osp.dirname(__file__))

VCPKG_ROOT = os.getenv("VCPKG_ROOT", None)
assert VCPKG_ROOT is not None, "env var VCPKG_ROOT must be set"

VCPKG_CMAKE_TOOLCHAIN_FILE = osp.join(
    VCPKG_ROOT, "scripts", "buildsystems", "vcpkg.cmake"
)
assert osp.exists(VCPKG_CMAKE_TOOLCHAIN_FILE), "cannot find vcpkg cmake toolchain file"

# may be overridden
CMAKE_DEFAULT_DEF: dict[str, str] = {
    "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
    "pybind11_DIR": PYBIND11_CMAKE_DIR,
    "CMAKE_TOOLCHAIN_FILE": VCPKG_CMAKE_TOOLCHAIN_FILE,
    "CMAKE_BUILD_TYPE": "Release",
}


class SubCommandError(Exception): ...


class CmakeExtension:
    def __init__(
        self,
        module_name: str,
        cmake_src_dir: str,
        cmake_bin_dir: Optional[str] = None,
        cmake_extr_def: Optional[dict[str, str]] = None,
        dry_run: bool = False,
    ) -> None:
        if cmake_extr_def is None:
            cmake_extr_def = {}
        self.module_name = module_name
        self.cmake_src_dir = osp.abspath(cmake_src_dir)
        self.cmake_def = cmake_extr_def
        # vcpkg chain load if toolchain is set
        if "CMAKE_TOOLCHAIN_FILE" in cmake_extr_def:
            self.cmake_def["VCPKG_CHAINLOAD_TOOLCHAIN_FILE"] = cmake_extr_def[
                "CMAKE_TOOLCHAIN_FILE"
            ]
        # add some default definitions if not presented
        for k, v in CMAKE_DEFAULT_DEF.items():
            if k not in self.cmake_def:
                self.cmake_def[k] = v
        assert "CMAKE_BUILD_TYPE" in self.cmake_def
        if cmake_bin_dir is None:
            build_type = self.cmake_def["CMAKE_BUILD_TYPE"].lower()
            self.cmake_bin_dir = osp.join(
                self.cmake_src_dir, f"cmake-build-{build_type}"
            )
        else:
            self.cmake_bin_dir = osp.abspath(cmake_bin_dir)

        self.dry_run = dry_run
        self.cmd_in_shell = False
        self.rm_cmd = "rm"
        self.rmdir_cmd = "rm"
        if os.name == "nt":
            self.cmd_in_shell = True
            self.rm_cmd = "del"
            self.rmdir_cmd = "rmdir"

        self._cmd_term_headers = {
            "configure": "[CMAKE]",
            "build": "[BUILD]",
            "clean": "[CLEAN]",
            "stubgen": "[STUB]",
        }

    def _run_single_cmd(self, cmd: list[str], term_header: str):
        print(f"\u001b[34m{term_header}\u001b[0m\t\u001b[32m{' '.join(cmd)}\u001b[0m")
        if self.dry_run:
            return 0
        with subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            shell=self.cmd_in_shell,
        ) as proc:
            if proc.stdout is not None:
                for line_bytes in proc.stdout:
                    line_str = line_bytes.decode().strip()
                    print(f"\u001b[34m{term_header}\t\u001b[0m{line_str}")
            if proc.stderr is not None:
                for line_bytes in proc.stderr:
                    line_str = line_bytes.decode().strip()
                    print(f"\u001b[31m{term_header}\t\u001b[0m{line_str}")
            code = proc.wait()
            if code != 0:
                raise SubCommandError

    def _run_cmd(self, cmd: list[str] | list[list[str]], term_header: str = ""):
        assert len(cmd) > 0
        if isinstance(cmd[0], str):
            self._run_single_cmd(cmd=cmd, term_header=term_header)  # type: ignore
        else:
            for _cmd in cmd:
                self._run_single_cmd(cmd=_cmd, term_header=term_header)  # type: ignore

    def _get_configure_cmd(self) -> list[str]:
        cmd = [
            CMAKE_BIN,
            "-S",
            self.cmake_src_dir,
            "-B",
            self.cmake_bin_dir,
        ]
        if os.name != "nt":
            cmd.extend(["-G", "Ninja"])
        for k, v in self.cmake_def.items():
            cmd.append(f"-D{k}={v}")
        return cmd

    def configure(self):
        cmd = self._get_configure_cmd()
        term_header = self._cmd_term_headers["configure"]
        self._run_cmd(cmd=cmd, term_header=term_header)

    def _get_build_cmd(self) -> list[str]:
        build_type = self.cmake_def["CMAKE_BUILD_TYPE"]
        cmd = [CMAKE_BIN, "--build", self.cmake_bin_dir, "--config", build_type]
        return cmd

    def _get_post_build_cmd(self) -> list[str]:
        cmd: list[str] = [CMAKE_BIN, "--install", self.cmake_bin_dir]
        return cmd

    def build(self):
        cmd = self._get_build_cmd()
        term_header = self._cmd_term_headers["build"]
        self._run_cmd(cmd=cmd, term_header=term_header)
        post_build_cmd = self._get_post_build_cmd()
        if len(post_build_cmd) > 0:
            self._run_cmd(post_build_cmd, term_header=term_header)

    def _get_clean_cmd(self) -> list[list[str]]:
        cmd: list[list[str]] = [
            [CMAKE_BIN, "--build", self.cmake_bin_dir, "--target", "clean"]
        ]
        manifest_path = osp.join(self.cmake_bin_dir, "install_manifest.txt")
        if not osp.exists(manifest_path):
            return cmd
        with open(manifest_path, "r") as f:
            for line in f.readlines():
                lib_path = line.replace("/", osp.sep)
                if osp.exists(lib_path):
                    cmd.append([self.rm_cmd, lib_path])
                base_name = lib_path.split(".", 1)[0]
                all_pyi = True
                new_cmd = []
                if osp.isdir(base_name):
                    for root, _, file_names in os.walk(base_name):
                        for file_name in file_names:
                            if not file_name.endswith(".pyi"):
                                all_pyi = False
                                continue
                            path = osp.join(root, file_name)
                            if osp.exists(path):
                                new_cmd.append([self.rm_cmd, path])
                    if all_pyi:
                        flag = ["/s", "/q"] if os.name == "nt" else ["-r"]
                        cmd.append([self.rmdir_cmd, *flag, base_name])
                    else:
                        cmd.extend(new_cmd)

                else:
                    pyi_path = f"{base_name}.pyi".replace("/", osp.sep)
                    if osp.exists(pyi_path):
                        cmd.append([self.rm_cmd, pyi_path])
        return cmd

    def clean(self):
        cmd = self._get_clean_cmd()
        if len(cmd) == 0:
            print(f"\u001b[34mNothing to clean.\t\u001b[0m")
            return
        self._run_cmd(cmd=cmd, term_header=self._cmd_term_headers["clean"])

    def _get_stub_gen_cmd(self) -> list[list[str]]:
        cmd = [
            [
                "stubgen",
                "--package",
                self.module_name,
                "--output",
                PROJECT_ROOT,
                "--include-docstrings",
                "--include-private",
            ]
        ]
        return cmd

    def stubgen(self):
        cmd = self._get_stub_gen_cmd()
        term_header = self._cmd_term_headers["stubgen"]
        self._run_cmd(cmd, term_header=term_header)


def main():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument(
        "--clean", action="store_true", default=False, help="clean all built output"
    )
    arg_parser.add_argument(
        "--debug", action="store_true", default=False, help="debug build"
    )
    arg_parser.add_argument(
        "--dry-run",
        action="store_true",
        default=False,
        help="show commands that will be executed without actually running them",
    )
    args = arg_parser.parse_args()
    if getattr(args, "help", False):
        arg_parser.print_usage()
        return
    cmake_extra_def = {}
    if getattr(args, "debug", False):
        cmake_extra_def["CMAKE_BUILD_TYPE"] = "Debug"

    ext_pybind11 = CmakeExtension(
        module_name="auto_pybind.ext",
        cmake_src_dir=osp.join(PROJECT_ROOT, "csrcs"),
        dry_run=args.dry_run,
        cmake_extr_def=cmake_extra_def,
    )

    extensions = [ext_pybind11]
    try:
        for ext in extensions:
            if args.clean:
                ext.clean()
                return
            ext.configure()
            ext.build()
            ext.stubgen()
    except SubCommandError:
        pass


if __name__ == "__main__":
    main()
