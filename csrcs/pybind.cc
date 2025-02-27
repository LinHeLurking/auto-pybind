#include "pybind.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "obj.h"
#include "util.h"

#ifndef MODULE_NAME
#define MODULE_NAME _ext
#endif

#define STR_(x) #x
#define STR(x) STR_(x)

namespace py = pybind11;

PYBIND11_MODULE(MODULE_NAME, m) {
  py::init();
  m.doc() = "Sample pybind11 C++ extension";
  Bind<BigObj, NamePythonize>(m, "BigObj").def(py::init<std::string>());
  Bind<SmallObj, NameAsIs>(m, "SmallObj").def(py::init<>());
  // NamePythonize is default!
  Bind<NoPyInitObj>(m, "NoPyInitObj");
}
