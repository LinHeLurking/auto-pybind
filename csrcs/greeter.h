#pragma once

#include <map>
#include <string>
#include <vector>

class Greeter {
public:
  std::string SimpleGreet();
  std::map<std::string, std::string>
  ComplexGreet(const std::vector<std::string> &names,
               const std::string &common_message);
};
