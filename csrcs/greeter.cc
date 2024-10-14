#include "greeter.h"
#include <sstream>

std::string Greeter::SimpleGreet() { return "Hello world!"; }

std::map<std::string, std::string>
Greeter::ComplexGreet(const std::vector<std::string> &names,
                      const std::string &common_message) {
  std::map<std::string, std::string> ret;
  for (auto &name : names) {
    std::stringstream ss;
    ss << "Hello, " << name << "! " << common_message;
    ret[name] = ss.str();
  }
  return ret;
}
