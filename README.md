# oi-debug

面向算法开发的 C++20 调试打印库。用 `debug(expr)` 查看表达式和值，用 `debug_with_type(expr)` 同时查看类型；容器、元组和数组可以递归嵌套，无需逐个编写输出循环。

项目采用 **header-only** 分发，核心打印功能仅依赖标准库。它专注于开发时的数据检查，不替代断点调试器，也不追求日志框架的配置与管理能力。

## 快速开始

将仓库的 `include/` 加入头文件搜索路径，或将完整的 `oi_debug/` 目录复制到源文件旁。使用 GCC 或 Clang，以 C++20 编译：

```bash
g++ -std=c++20 -I/path/to/oi-debug/include main.cpp -o main
```

```c++
#include <map>
#include <string>
#include <vector>
#ifndef ONLINE_JUDGE
#include "oi_debug/debug.hpp"
#endif

int main() {
    std::map<std::string, std::vector<int>> graph{
        {"A", {1, 2}}, {"B", {3}}
    };
#ifndef ONLINE_JUDGE
    debug(graph);
    debug_with_type(graph);
#endif
}
```

输出：

```text
graph = {"A": [1, 2], "B": [3]}
std::map<std::string, std::vector<int>> graph = {"A": [1, 2], "B": [3]}
```

`ONLINE_JUDGE` 用于隔离本地调试代码，需同时保护头文件和调用点；若评测平台不定义该宏，应在提交前自行关闭调试代码。

## 类型支持

| 类型 | 输出方式 |
| --- | --- |
| 基本类型、字符串、枚举 | 数值或文本；枚举输出底层数值 |
| 顺序容器、集合、嵌套 C 数组 | `[a, b]`，递归处理元素 |
| map、pair、tuple | 分别使用 `{key: value}`、`(a, b)`、`(a, b, …)` |
| queue、stack、priority_queue | queue 从队头开始，其余从 top 开始 |
| optional、variant、complex | 保留状态或结构，如 `optional(v)`、`variant(v)`、`(real, imag)` |
| 指针、智能指针 | 非字符串裸指针显示地址，智能指针显示所指值；数组／void 智能指针显示地址，空指针统一为 `nullptr` |

嵌套字符串和字符会加引号，并转义引号、反斜杠及控制字符，避免换行或特殊字符破坏结构。顶层字符串与字符保留原文。自定义类型可提供 `std::string to_string() const`，接入同一套递归输出规则。

另有两个按需包含的工具：`timer.hpp` 提供 `get_time_us/ms/s(fn, args...)` 调用计时；`output.hpp` 提供 `oi_debug::print/println(...)`，连续输出参数而不自动插入空格。

## 设计与取舍

1. **在编译期选择格式。** 通过 concepts 和模板重载区分字符串、映射、容器等类型，递归组合，而非维护运行时类型分支。表达式名称来自宏字符串化；类型名称由 `TypeParseTraits` 结合编译器签名和常见类型特化生成，核心打印路径不依赖运行时 demangle。
2. **共享缓冲区，控制中间开销。** 嵌套值直接追加到同一个 `std::string`，避免逐层创建结果字符串；整数和浮点数优先用 `std::to_chars` 转换，标准库缺少 long double 重载时回退到 `snprintf`。结果字符串仍有分配成本，打印也需要实际遍历数据。
3. **约束与调用保持一致。** 自定义序列化和 range 按 `const` 访问能力检查，不支持的类型给出编译期诊断；分配异常与用户序列化异常向外传播，不用无条件 `noexcept` 掩盖失败。
4. **明确非侵入式访问的代价。** 标准容器适配器没有公开迭代接口，因此复制后弹出遍历，保留原容器而不访问其内部实现。元素须可复制，打印大队列或堆时应考虑这份时间与空间开销。

## 使用边界

- 输出到 **`std::cout`**，末尾换行但不强制刷新；交互调试需要立即显示时，可显式调用 `std::cout.flush()`。
- 每次传一个表达式，支持花括号和模板逗号；`debug(a, b)` 按逗号运算符求值，只打印 `b`。
- range 必须可通过 const 对象遍历；单遍输入 range 会消耗输入，C++20 的 `filter_view` 等仅提供非 const 遍历的 view 需先收集为容器。
- 无序容器的顺序不保证稳定，类型名称的拼写可能随编译器变化；递归展开的智能指针对象图应无引用环。

## 质量验证

在仓库根目录运行，构建产物均放在临时目录中：

```bash
python3 tests/run.py
python3 tests/run.py --sanitize
```

已在 **GCC 15、Clang 21（均使用 libstdc++）** 下以 C++20、`-Wall -Wextra -pedantic -Werror` 验证。回归覆盖嵌套格式、字符边界、异常传播、单遍 range、适配器顺序与复制、计时器引用返回值，以及文档示例、头文件独立包含和双翻译单元链接；同时通过 ASan、UBSan 与泄漏检查。

MSVC 和 libc++ 尚未验证；`timer.hpp` 另依赖 Itanium ABI 的 `__cxa_demangle`。目前未提供性能基准，性能设计不等于已测量的性能优势。

## 贡献与许可

欢迎提交 Issue 或 PR。问题报告请附最小复现、编译器版本和预期输出；新增类型或修改格式时，请补充对应回归用例。

项目采用 [GPL-3.0](LICENSE) 许可证。
