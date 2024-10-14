#pragma once
#include <boost/describe.hpp>
#include <string>

class BigObj {
 public:
  std::string str_{};
  int i1_{}, i2_{};
  int ia_[4]{};
  bool b1_{}, b2_{};

  std::string GetId() const { return id_; }
  static std::string Greet(const std::string& name) {
    return "Hello " + name + "!";
  }

 private:
  std::string id_;
  BOOST_DESCRIBE_CLASS(BigObj, (),
                       (str_, i1_, i2_, ia_, b1_, b2_, GetId, Greet), (), (id_))
};
