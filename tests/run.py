#!/usr/bin/env python3
"""Run consumer regressions without placing build artifacts in the repository."""

import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HEADERS = ("core", "debug", "fields", "matrix", "diff", "get_type", "output",
           "string_concat", "timer", "to_string")


def execute(command, *, source=None, env=None):
    result = subprocess.run(command, input=source, text=True, capture_output=True,
                            env=env, timeout=60)
    if result.returncode:
        raise RuntimeError(f"{' '.join(map(str, command))}\n{result.stdout}{result.stderr}")
    return result.stdout


def readme_block(readme, marker, language):
    pattern = rf"<!-- {marker} -->\n```{re.escape(language)}\n(.*?)```"
    match = re.search(pattern, readme, re.S)
    assert match, f"README block missing: {marker}"
    return match.group(1)


def check_compiler(compiler, directory, sanitize):
    flags = [compiler, "-std=c++20", "-Wall", "-Wextra", "-pedantic", "-Werror",
             "-I", str(ROOT / "include")]
    env = os.environ.copy()
    env["ASAN_OPTIONS"] = "detect_leaks=1:halt_on_error=1"
    env["UBSAN_OPTIONS"] = "halt_on_error=1"
    sanitizer_flags = (["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
                       if sanitize else [])
    for name in ("regression", "timer", "fields", "matrix", "diff"):
        binary = directory / name
        execute([*flags, *sanitizer_flags, str(ROOT / "tests" / f"{name}.cpp"),
                 "-o", str(binary)])
        output = execute([str(binary)], env=env)
        if name == "regression":
            assert output == "serialization checks passed\n", output
        elif name == "timer":
            assert re.search(r"call return_reference<.*> spent \d+us, return value: 42\n", output), output
            assert re.search(r"call return_owned<.*> spent \d+us, return value: 7\n", output), output
            assert re.search(r"call take_owned<.*> spent \d+ms\n", output), output
            assert re.search(r"call return_number<.*> spent \d+s, return value: 3\n", output), output
            assert output.endswith("timer checks passed\n"), output
        else:
            assert output == f"{name} checks passed\n", output
        print(f"{compiler}: {name} passed", flush=True)

    readme = (ROOT / "README.md").read_text(encoding="utf-8")
    golden = directory / "sample"
    execute([*flags, str(ROOT / "include/oi_debug/test.cpp"), "-o", str(golden)])
    expected = (ROOT / "tests/sample.txt").read_text()
    assert execute([str(golden)]) == expected, "Golden sample output mismatch"

    for header in HEADERS:
        execute([*flags, "-fsyntax-only", "-x", "c++", "-"],
                source=f'#include "oi_debug/{header}.hpp"\nint main() {{}}\n')

    for defines in ([], ["-DONLINE_JUDGE"]):
        for example in ("quickstart", "showcase"):
            source = readme_block(readme, f"{example}-source", "c++")
            expected = "" if defines else readme_block(readme, f"{example}-output", "text")
            binary = directory / example
            execute([*flags, *defines, "-x", "c++", "-", "-o", str(binary)], source=source)
            assert execute([str(binary)]) == expected, f"README {example} output mismatch"

        binary = directory / "knapsack"
        execute([*flags, *defines, str(ROOT / "examples/knapsack.cpp"), "-o", str(binary)])
        expected = "" if defines else readme_block(readme, "knapsack-output", "text")
        assert execute([str(binary)]) == expected, "Knapsack example output mismatch"

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

    unsupported_diff = {
        "uncomparable": 'struct X { std::string to_string() const { return "x"; } };\nint main(){(void)oi_debug::diff(X{},X{});}',
        "smart_pointer": 'int main(){std::shared_ptr<int> p; (void)oi_debug::diff(p,p);}',
        "nested_uncomparable": 'struct X { std::string to_string() const { return "x"; } };\nint main(){std::vector<X> v; (void)oi_debug::diff(v,v);}',
    }
    for name, body in unsupported_diff.items():
        result = subprocess.run([*flags, "-fsyntax-only", "-x", "c++", "-"],
                                input='#include "oi_debug/diff.hpp"\n' + body,
                                text=True, capture_output=True, timeout=60)
        assert result.returncode != 0, f"{name}: unsupported diff unexpectedly accepted"
        assert "oi_debug::diff: unsupported leaf type" in result.stderr, result.stderr

    includes = "".join(f'#include "oi_debug/{header}.hpp"\n' for header in HEADERS)
    first = directory / "first.cpp"
    second = directory / "second.cpp"
    fixture_header = f'#include "{ROOT / "tests/registered.hpp"}"\n'
    first.write_text(includes + fixture_header +
                     'std::string other(){return oi_debug::to_string(fixture::Record{1,{2,3}});}\n')
    reverse_includes = "".join(f'#include "oi_debug/{header}.hpp"\n' for header in reversed(HEADERS))
    second.write_text(reverse_includes + fixture_header +
                      'std::string other();\nint main(){fixture::Record a{1,{2,3}}, b{2,{2,3}}; '
                      'const int grid[1][1]={{1}}; '
                      'return other() == "fixture::Record{value: 1, samples: [2, 3]}" && '
                      'oi_debug::diff(a,b).changes.size() == 1 && '
                      'oi_debug::matrix_to_string(grid).size() > 0 ? 0 : 1;}\n')
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
