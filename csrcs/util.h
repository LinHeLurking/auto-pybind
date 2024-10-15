#pragma once
#include <pybind11/pybind11.h>

#include <array>
#include <boost/callable_traits.hpp>
#include <boost/describe.hpp>
#include <boost/mp11.hpp>
#include <filesystem>

class NamePythonize {
 public:
  consteval auto operator()(const char* name) const {
    int size = 0;
    for (int i = 0; name[i] != 0; ++i) {
      size++;
    }
    std::array<char, 128> buffer{};
    std::ranges::fill(buffer, 0);
    int j = 0;
    for (int i = 0; i < size; ++i) {
      if (i + 1 == size && name[i] == '_') {
        // remove tailing '_'
        break;
      }
      if (name[i] >= 'A' && name[i] <= 'Z') {
        buffer[j++] = static_cast<char>(name[i] + 'a' - 'A');
      } else {
        buffer[j++] = name[i];
      }
      if (i + 1 < size && name[i] >= 'a' && name[i] <= 'z' &&
          name[i + 1] >= 'A' && name[i + 1] <= 'Z') {
        // insert '_', ex. insert '_' between 't' and 'I' in "GetId"
        buffer[j++] = '_';
      }
    }
    return buffer;
  }
};

class NameAsIs {
 public:
  consteval auto operator()(const char* name) const {
    std::array<char, 128> buffer{};
    std::ranges::fill(buffer, 0);
    for (int i = 0; name[i] != 0; ++i) {
      buffer[i] = name[i];
    }
    return buffer;
  }
};

class AddDefaultCtor {};

template <typename T>
class IsNameConverter : public std::false_type {};

template <typename T>
  requires std::is_default_constructible_v<T> && requires {
    // it returns a string container
    { T{}("").data() } -> std::convertible_to<const char*>;
    // it's totally constexpr
    {
      std::bool_constant<(T{}("").data(), true)>()
    } -> std::same_as<std::true_type>;
  }

class IsNameConverter<T> : public std::true_type {};

static_assert(IsNameConverter<NamePythonize>::value);

namespace details {
template <typename Tuple>
concept FoundNameConverter =
    (boost::mp11::mp_find_if<Tuple, IsNameConverter>::value <
     std::tuple_size_v<Tuple>) &&
    (std::tuple_size_v<Tuple> > 0);

template <typename>
struct GetNameConverter {
  using type = NamePythonize;
};

template <typename Tuple>
  requires FoundNameConverter<Tuple>
struct GetNameConverter<Tuple> {
  static constexpr size_t idx_ =
      boost::mp11::mp_find_if<Tuple, IsNameConverter>::value;
  using type = typename std::tuple_element_t<idx_, Tuple>;
};

template <typename D1, typename D2>
struct IsSameDescriptor {
 public:
  using D1P = std::integral_constant<int, D1::pointer>;
  using D2P = std::integral_constant<int, D2::pointer>;
  static constexpr bool value = std::is_same_v<D1, D2>;
};

template <typename D1>
struct IsSameDescriptorQ {
 public:
  template <typename D2>
  using fn = IsSameDescriptor<D1, D2>;
};
}  // namespace details

template <typename T, typename... Options>
  requires boost::describe::has_describe_members<T>::value &&
           (!std::is_union_v<T>)
auto Bind(pybind11::module& m, std::string_view class_name) {
  namespace py = pybind11;
  using namespace boost::mp11;
  using namespace boost::describe;
  using namespace boost::callable_traits;
  using namespace details;

  //
  // check options
  //
  using OptionTuple = std::tuple<Options...>;
  constexpr size_t num_options = sizeof...(Options);

  //
  // actual binder code
  //

  // prepare
  auto pyclazz = py::class_<T>(m, class_name.data());
  using PubVar = describe_members<T, mod_public>;
  // including static functions
  using PubMemberFunc = describe_members<T, mod_public | mod_function>;
  // using PrivateVar = describe_members<T, mod_private>;
  using NameConverter = typename GetNameConverter<OptionTuple>::type;
  constexpr auto name_converter = NameConverter{};

  // member variable
  mp_for_each<PubVar>([&](auto Desc) {
    constexpr auto buffer = name_converter(Desc.name);
    auto var_name = buffer.data();
    pyclazz.def_readwrite(var_name, Desc.pointer);
  });
  // member function
  mp_for_each<PubMemberFunc>([&](auto Desc) {
    constexpr auto buffer = name_converter(Desc.name);
    auto func_name = buffer.data();
    pyclazz.def(func_name, Desc.pointer);
  });

  return pyclazz;
}
