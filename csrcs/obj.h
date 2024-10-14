#pragma once
#include <boost/describe.hpp>
#include <string>
#include <utility>

class BigObj {
 public:
  BigObj() = default;

  explicit BigObj(std::string id) : id_{std::move(id)} {}

  std::string str_{};
  int i1_{}, i2_{};
  std::vector<int> ia_{{1, 2, 3, 4, 5}};
  bool b1_{}, b2_{};

  [[nodiscard]] std::string GetId() const { return id_; }

  static std::string Greet(const std::string& name) {
    return "Hello " + name + "!";
  }

 private:
  std::string id_{};
  BOOST_DESCRIBE_CLASS(BigObj, (), (str_, i1_, i2_, ia_, b1_, b2_, GetId), (),
                       (id_))
};

class NoPyInitObj {
 public:
  NoPyInitObj() = default;
  int v_{};
  BOOST_DESCRIBE_CLASS(NoPyInitObj, (), (v_), (), ())
};

class SmallObj : public NoPyInitObj {
 public:
  SmallObj() { v_ = 3; }

  BOOST_DESCRIBE_CLASS(SmallObj, (), (v_), (), ())
};
