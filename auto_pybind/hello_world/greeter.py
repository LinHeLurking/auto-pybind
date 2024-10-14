class Greeter:
    def __init__(self) -> None: ...

    def simple_greet(self) -> str:
        return "Hello world!"

    def complex_greet(self, names: list[str], common_message: str) -> dict[str, str]:
        res = {}
        for name in names:
            res[name] = f"Hello, {name}! {common_message}"
        return res
