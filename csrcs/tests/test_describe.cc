#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <boost/describe.hpp>
#include <boost/mp11.hpp>

#include "obj.h"

using namespace boost::describe;
using namespace boost::mp11;

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
