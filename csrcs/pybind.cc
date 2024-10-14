#include "greeter.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#ifndef MODULE_NAME
#define MODULE_NAME _ext
#endif

#define __STR(x) #x
#define STR(x) __STR(x)

namespace py = pybind11;

PYBIND11_MODULE(MODULE_NAME, m) {
  m.doc() = "Sample pybind11 C++ extension";
  py::class_<Greeter>(m, "Greeter")
      .def(py::init<>())
      .def("simple_greet", &Greeter::SimpleGreet)
      .def("complex_greet", &Greeter::ComplexGreet,
           R"(greet many targets with common message
Args:
    arg0(list[str]): targets you want to greet.
    arg1(str): the common message.
Returns:
    a dict whose keys are different targets and values are greeting messages.

Examples:
    Here is an example:

    >>> greeter = Greeter()
    >>> greeter.complex_greet(["Tifa", "Aerith"], "You are pretty!")
    >>> {"Tifa": "Hello Tifa! You are pretty!", "Aerith": "Hello Aerith! You are pretty!"}
)");
}
