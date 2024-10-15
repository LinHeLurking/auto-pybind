#pragma once

#include <pybind11/pybind11.h>

#include <boost/describe.hpp>
#include <boost/mp11.hpp>

#include "util.h"

template <typename T, typename... Options>
  requires boost::describe::has_describe_members<T>::value &&
           (!std::is_union_v<T>)
auto Bind(pybind11::module& m, std::string_view class_name) {
  namespace py = pybind11;
  using namespace boost::mp11;
  using namespace boost::describe;
  using namespace details;

  //
  // check options
  //
  using OptionTuple = std::tuple<Options...>;
  constexpr bool guess_setter_getter =
      !mp_any_of_q<OptionTuple, IsSameQ<NoGuessSetterGetter>>::value;

  //
  // actual binder code
  //

  // prepare
  auto pyclazz = py::class_<T, std::shared_ptr<T>>(m, class_name.data());
  using PubVar = describe_members<T, mod_public>;
  // not including static functions
  using PubMemberFunc = describe_members<T, mod_public | mod_function>;
  using PubGetters = mp_filter<DescriptorIsGetter, PubMemberFunc>;
  using PubSetters = mp_filter<DescriptorIsSetter, PubMemberFunc>;
  using NameConverter = typename GetNameConverter<OptionTuple>::type;
  constexpr auto name_converter = NameConverter{};

  // member variable
  mp_for_each<PubVar>([&](auto Desc) {
    constexpr auto buffer = name_converter(Desc.name);
    auto var_name = buffer.data();
    pyclazz.def_readwrite(var_name, Desc.pointer);
  });
  // member function (excluding setter/getter)
  mp_for_each<PubMemberFunc>([&](auto Desc) {
    if constexpr (guess_setter_getter) {
      if (IsGetterName(Desc.name) || IsSetterName(Desc.name)) {
        return;
      }
    }
    constexpr auto buffer = name_converter(Desc.name);
    auto func_name = buffer.data();
    pyclazz.def(func_name, Desc.pointer);
  });
  // getter/setter
  if constexpr (guess_setter_getter) {
    mp_for_each<PubGetters>([&](auto Desc) {
      constexpr auto buffer =
          GetPropertyNameFromGetterName(name_converter(Desc.name));
      auto property_name = buffer.data();
      using SetterIdx = mp_find_if_q<PubSetters, NameEqualQ<decltype(Desc), 3>>;
      constexpr bool has_setter = SetterIdx::value < mp_size<PubSetters>::value;
      if constexpr (has_setter) {
        using SetterDesc = mp_at<PubSetters, SetterIdx>;
        pyclazz.def_property(property_name, Desc.pointer, SetterDesc::pointer);
      } else {
        pyclazz.def_property_readonly(property_name, Desc.pointer);
      }
    });
  }

  return pyclazz;
}
