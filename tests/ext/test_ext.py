import unittest

from python_template.ext import Greeter as ExtGreeter
from python_template.hello_world.greeter import Greeter


class TestGreeter(unittest.TestCase):
    def setUp(self) -> None:
        self.greeter = Greeter()
        self.ext_greeter = ExtGreeter()
        return super().setUp()

    def test_simple(self):
        assert self.greeter.simple_greet() == self.ext_greeter.simple_greet()

    def test_complex(self):
        common_message = "RUA!"
        names = ["Cloud", "Aerith", "Tifa"]
        expected = self.greeter.complex_greet(
            names=names, common_message=common_message
        )
        result = self.ext_greeter.complex_greet(names, common_message)
        assert result == expected
