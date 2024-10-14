# Python Template Project

This is a python template project managed using [poetry](https://python-poetry.org/).

Features:

- [pybind11](https://pybind11.readthedocs.io/en/stable/index.html) powered C++ extension with full type hints(thanks to [mypy stubgen](https://mypy.readthedocs.io/en/stable/stubgen.html))
- [pytest](https://pytest.org/).
- [pre-commit](https://pre-commit.com/) hooks with isort, black, clang-format, etc.

## First Clone

Install pre-commit hooks on your first clone.

```bash
pre-commit install
```

You only need to run it once.

## Build from Source

Run `poetry build` and everything's done.

- C++ extension will be built and copied into source tree.
- Mypy will generate `*.pyi` files for C++ extension in source tree.
- All python files and C++ libraries will be packaged as a `.whl` file.

## Development

Run `poetry install --with dev` to install all development dependencies.

## Run Tests

Running `pytest tests` is enough. Everything will be automatically discoverable in IDE.
