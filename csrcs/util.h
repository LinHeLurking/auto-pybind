#pragma once
#include <pybind11/pybind11.h>

#include <boost/describe.hpp>
#include <boost/mp11.hpp>

class NameAsIs {
 public:
  std::string operator()(const char* name) const { return std::string{name}; }
};

template <typename T, typename... Options>
  requires boost::describe::has_describe_members<T>::value &&
           (!std::is_union_v<T>)
auto Bind(pybind11::module& m, std::string_view class_name) {
  namespace py = pybind11;
  using namespace boost::mp11;
  using namespace boost::describe;
  using PubMembers = describe_members<T, mod_public>;
  using HiddenMembers = describe_members<T, mod_private | mod_protected>;
  auto bind = py::class_<T>(m, class_name.data());
  auto name_convertor = NameAsIs();
  mp_for_each<PubMembers>([&](auto Desc) {
    std::string member_name = name_convertor(Desc.name);
    if (std::is_member_function_pointer_v<decltype(Desc.pointer)>) {
      // member function
      bind.def(member_name.c_str(), Desc.pointer);
    } else {
      // member variable
      bind.def_readwrite(member_name.c_str(), Desc.pointer);
    }
  });
  return bind;
}
