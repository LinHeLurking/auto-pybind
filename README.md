# Automatic Pybind 

Bind everything to python with a few lines of codes!

## Example

First, use `boost::describe` to define static reflection information.

```c++
class BigObj {
 public:
  BigObj() = default;

  explicit BigObj(std::string id) : id_{std::move(id)} {}

  std::string str_{};
  int i1_{}, i2_{};
  std::vector<int> ia_{{1, 2, 3, 4, 5}};
  bool b1_{}, b2_{};

  [[nodiscard]] std::string GetId() const { return id_; }
  void SetId(std::string id) { id_ = std::move(id); }

  [[nodiscard]] std::string GetReadOnlyStr() const { return read_only_str_; }

  static std::string Greet(const std::string& name) {
    return "Hello " + name + "!";
  }

 private:
  std::string id_{};
  std::string read_only_str_{"123"};
  // Here's what you add in addition to original codes.
  BOOST_DESCRIBE_CLASS(BigObj, (),
                       (str_, i1_, i2_, ia_, b1_, b2_, GetId, SetId, Greet,
                        GetReadOnlyStr),
                       (), (id_, read_only_str_))
};
```

Then, single line of binder code is enough.

```c++
// `m` is a `pybind11::module`
Bind<BigObj, NamePythonize>(m, "BigObj").def(py::init<std::string>());
```

Everthing is done by static reflection and metaprogramming.

This repo also has a helper script which leverages `stubgen` from the `mypy` project, helping you to generate `*.pyi` stub files. Everything is generated automatically. You add a few lines of C++ codes aforementioned and run `poetry build`. Then everything is done. Some python files will be generated and placed to correct directories.

```python
class BigObj:
    b1: bool
    b2: bool
    i1: int
    i2: int
    ia: list[int]
    id: str
    str: str
    def __init__(self, arg0: str) -> None:
        """__init__(self: auto_pybind.ext.BigObj, arg0: str) -> None"""
    def _pybind11_conduit_v1_(self, *args, **kwargs): ...
    @property
    def read_only_str(self) -> str: ...
```
