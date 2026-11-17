#!/usr/bin/env python3
"""Run consumer regressions without placing build artifacts in the repository."""

import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HEADERS = ("core", "debug", "get_type", "output", "string_concat", "timer", "to_string")


def execute(command, *, source=None, env=None):
    result = subprocess.run(command, input=source, text=True, capture_output=True,
                            env=env, timeout=60)
    if result.returncode:
        raise RuntimeError(f"{' '.join(map(str, command))}\n{result.stdout}{result.stderr}")
    return result.stdout


def check_compiler(compiler, directory, sanitize):
    flags = [compiler, "-std=c++20", "-Wall", "-Wextra", "-pedantic", "-Werror",
             "-I", str(ROOT / "include")]
    env = os.environ.copy()
    env["ASAN_OPTIONS"] = "detect_leaks=1:halt_on_error=1"
    env["UBSAN_OPTIONS"] = "halt_on_error=1"
    sanitizer_flags = (["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
                       if sanitize else [])
    for name in ("regression", "timer"):
        binary = directory / name
        execute([*flags, *sanitizer_flags, str(ROOT / "tests" / f"{name}.cpp"),
                 "-o", str(binary)])
        output = execute([str(binary)], env=env)
        if name == "regression":
            assert output == "serialization checks passed\n", output
        else:
            assert re.search(r"call return_reference<.*> spent \d+us, return value: 42\n", output), output
            assert re.search(r"call return_owned<.*> spent \d+us, return value: 7\n", output), output
            assert re.search(r"call take_owned<.*> spent \d+ms\n", output), output
            assert re.search(r"call return_number<.*> spent \d+s, return value: 3\n", output), output
            assert output.endswith("timer checks passed\n"), output
        print(f"{compiler}: {name} passed", flush=True)

    readme = (ROOT / "README.md").read_text()
    golden = directory / "sample"
    execute([*flags, str(ROOT / "include/oi_debug/test.cpp"), "-o", str(golden)])
    expected = (ROOT / "tests/sample.txt").read_text()
    assert execute([str(golden)]) == expected, "Golden sample output mismatch"

    for header in HEADERS:
        execute([*flags, "-fsyntax-only", "-x", "c++", "-"],
                source=f'#include "oi_debug/{header}.hpp"\nint main() {{}}\n')

    judge_sample = re.search(r"```c\+\+\n(.*?)```", readme, re.S).group(1)
    documented_output = re.search(r"```text\n(.*?)```", readme, re.S).group(1)
    for defines in ([], ["-DONLINE_JUDGE"]):
        binary = directory / "readme_sample"
        execute([*flags, *defines, "-x", "c++", "-", "-o", str(binary)], source=judge_sample)
        assert execute([str(binary)]) == ("" if defines else documented_output), "README example output mismatch"

    unsupported = {
        "mutable_member": 'struct X { std::string to_string() { return "x"; } };\nint main(){(void)oi_debug::to_string(X{});}',
        "invalid_member_result": 'struct X { int to_string() const { return 1; } };\nint main(){(void)oi_debug::to_string(X{});}',
        "non_const_range": 'int main(){std::vector<int> v{1}; auto r=v | std::views::filter([](int x){return x>0;}); (void)oi_debug::to_string(r);}',
        "noncopyable_queue": 'int main(){std::queue<std::unique_ptr<int>> q; (void)oi_debug::to_string(q);}',
        "noncopyable_stack": 'int main(){std::stack<std::unique_ptr<int>> s; (void)oi_debug::to_string(s);}',
    }
    for name, body in unsupported.items():
        result = subprocess.run([*flags, "-fsyntax-only", "-x", "c++", "-"],
                                input='#include "oi_debug/debug.hpp"\n' + body,
                                text=True, capture_output=True, timeout=60)
        assert result.returncode != 0, f"{name}: unsupported input unexpectedly accepted"
        assert "some types could not convert" in result.stderr, result.stderr

    includes = "".join(f'#include "oi_debug/{header}.hpp"\n' for header in HEADERS)
    first = directory / "first.cpp"
    second = directory / "second.cpp"
    first.write_text(includes + '#include <vector>\nstd::string other(){return oi_debug::to_string(std::vector<int>{1,2});}\n')
    second.write_text(includes + 'std::string other();\nint main(){return other() == "[1, 2]" ? 0 : 1;}\n')
    binary = directory / "two_tu"
    execute([*flags, str(first), str(second), "-o", str(binary)])
    execute([str(binary)])
    print(f"{compiler}: golden output, headers, ONLINE_JUDGE, diagnostics and two-TU linking passed", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("compilers", nargs="*", default=["g++", "clang++"])
    parser.add_argument("--sanitize", action="store_true", help="enable ASan, UBSan and leak detection")
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix="oi-debug-tests-") as temp:
        directory = Path(temp)
        for compiler in args.compilers:
            check_compiler(compiler, directory, args.sanitize)
    print("All checks passed.")


if __name__ == "__main__":
    main()
