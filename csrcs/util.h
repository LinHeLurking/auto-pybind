#pragma once

#include <array>
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

class NoGuessSetterGetter {};

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

template <typename T>
struct IsSameQ {
  template <typename U>
  using fn = std::is_same<U, T>;
};

consteval bool IsGetterName(const char* name) {
  bool flag1 =
      name[0] == 'g' && name[1] == 'e' && name[2] == 't' && name[4] == '_';
  bool flag2 = name[0] == 'G' && name[1] == 'e' && name[2] == 't';
  return flag1 || flag2;
}

consteval bool IsSetterName(const char* name) {
  bool flag1 =
      name[0] == 's' && name[1] == 'e' && name[2] == 't' && name[4] == '_';
  bool flag2 = name[0] == 'S' && name[1] == 'e' && name[2] == 't';
  return flag1 || flag2;
}

template <typename T>
struct DescriptorIsGetter {
  static constexpr bool value = IsGetterName(T::name);
};

template <typename T>
struct DescriptorIsSetter {
  static constexpr bool value = IsSetterName(T::name);
};

consteval auto GetSetterNameFromGetterName(const char* name) {
  std::array<char, 128> buffer{};
  std::ranges::fill(buffer, 0);
  std::copy_if(name, name + 128, buffer.begin(), [](char c) { return c == 0; });
  if (buffer[0] == 'G') {
    buffer[0] = 'S';
  } else if (buffer[0] == 'g') {
    buffer[0] = 's';
  }
  return buffer;
}

consteval auto GetPropertyNameFromGetterName(std::array<char, 128> name) {
  std::array<char, 128> buffer{};
  int offset = 3 + int(name[3] == '_');
  std::copy(name.begin() + offset, name.end(), buffer.begin());
  return buffer;
}

consteval bool StrEq(const char* a, const char* b, size_t offset) {
  size_t i = offset;
  for (; a[i] != 0 && b[i] != 0 && a[i] == b[i]; ++i) {
  }
  return a[i] == 0 && b[i] == 0;
}

template <typename D1, typename D2, size_t offset>
struct NameEqual {
  static constexpr bool value = StrEq(D1::name, D2::name, offset);
};

template <typename D1, size_t offset = 0>
struct NameEqualQ {
  template <typename D2>
  using fn = NameEqual<D1, D2, offset>;
};

}  // namespace details
