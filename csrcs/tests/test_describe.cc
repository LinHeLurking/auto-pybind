#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <boost/describe.hpp>
#include <boost/mp11.hpp>

#include "obj.h"
#include "util.h"

using namespace boost::describe;
using namespace boost::mp11;
using namespace details;

TEST(DescribeTest, A) {
  using PubMemberVariables = describe_members<BigObj, mod_public>;
  using PubMemberFunctions =
      describe_members<BigObj, mod_public | mod_function>;
  using PubMemberStaticFunctions =
      describe_members<BigObj, mod_public | mod_function | mod_static>;
  std::set<std::string> variables{}, functions{}, static_functions{};
  mp_for_each<PubMemberVariables>(
      [&](auto Desc) { variables.insert(Desc.name); });
  mp_for_each<PubMemberFunctions>(
      [&](auto Desc) { functions.insert(Desc.name); });
  mp_for_each<PubMemberStaticFunctions>(
      [&](auto Desc) { static_functions.insert(Desc.name); });
  std::set<std::string> expected_variables{"str_", "i1_", "i2_",
                                           "ia_",  "b1_", "b2_"},
      expected_functions{"GetId"}, expected_static_functions{"Greet"};
  EXPECT_EQ(variables, expected_variables);
  EXPECT_EQ(functions, expected_functions);
  EXPECT_EQ(static_functions, expected_static_functions);
}

TEST(DescribeTest, B) {
  using PubMemberFunc = describe_members<BigObj, mod_public | mod_function>;
  using PubGetters = mp_filter<DescriptorIsGetter, PubMemberFunc>;
  using PubSetters = mp_filter<DescriptorIsSetter, PubMemberFunc>;
  std::set<std::string> expected_getters{"GetId", "GetReadOnlyStr"},
      expected_setters{"SetId"};
  mp_for_each<PubGetters>([&](auto Desc) {
    using SetterIdx = mp_find_if_q<PubSetters, NameEqualQ<decltype(Desc), 3>>;
    constexpr bool has_setter = SetterIdx::value < mp_size<PubSetters>::value;
    EXPECT_EQ(std::string{Desc.name} == "GetId", has_setter)
        << "setter idx: " << SetterIdx::value << std::endl
        << "num setters: " << mp_size<PubSetters>::value;
    EXPECT_TRUE(expected_getters.contains(Desc.name));
  });
  mp_for_each<PubSetters>(
      [&](auto Desc) { EXPECT_TRUE(expected_setters.contains(Desc.name)); });
  {
    auto buffer =
        details::GetPropertyNameFromGetterName(NamePythonize{}("GetId"));
    EXPECT_EQ(std::string{buffer.data()}, std::string{"id"});
  }
}
