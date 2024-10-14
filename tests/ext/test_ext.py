from auto_pybind.ext import BigObj, NoPyInitObj, SmallObj


def test_no_py_init():
    try:
        _ = NoPyInitObj()
    except TypeError as e:
        msg = str(e).split(":")[-1].strip()
        assert msg == "No constructor defined!"
    except:
        assert False


def test_can_init_obj():
    obj = SmallObj()
    # SmallObj names are passed as-is.
    assert obj.v_ == 3


def test_big_obj():
    obj = BigObj("py")
    assert obj.str == ""
    assert obj.get_id() == "py"
    assert obj.ia == [1, 2, 3, 4, 5]

    # static member function
    assert BigObj.greet("world") == "Hello world!"
