# oi-debug

[![CI](https://github.com/flyingpig233-cpu/oi-debug/actions/workflows/ci.yml/badge.svg)](https://github.com/flyingpig233-cpu/oi-debug/actions/workflows/ci.yml)

面向算法竞赛的 C++20 调试库。用 `debug(expr)` 查看变量，用带下标的矩阵和状态差异定位 DP 算错的格子。项目采用 **header-only** 分发，核心功能仅依赖标准库。

## 用 0/1 背包定位错误

只有一件重量为 2、价值为 3 的物品，背包容量为 4，答案应为 3。转移时误读当前行 `dp[1][capacity - weight]`，程序却算出了 6。用 `debug_diff(expected, dp)` 比较参考表和实际表，再用 `debug_matrix` 高亮差异对应的坐标。

<!-- knapsack-output -->
```text
expected -> dp:
  [1][4]: 3 -> 6
dp =
   0 1 2 3   4
0  0 0 0 0   0
1  0 0 3 3 [6]
```

`[1][4]` 表示考虑第一件物品、容量为 4 时的状态。这里读到了本行容量为 2 时已经选过物品的结果。把转移来源改成上一行 `dp[0][capacity - weight]`，就能恢复每件物品只选一次的语义。

在仓库根目录编译并运行[完整错例](examples/knapsack.cpp)。

```bash
g++ -std=c++20 -Iinclude examples/knapsack.cpp -o /tmp/oi-debug-knapsack
/tmp/oi-debug-knapsack
```

## 快速开始

将仓库的 `include/` 加入头文件搜索路径，或将完整的 `oi_debug/` 目录复制到源文件旁。使用 GCC 或 Clang，以 C++20 编译：

```bash
g++ -std=c++20 -I/path/to/oi-debug/include main.cpp -o main
```

<!-- quickstart-source -->
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

<!-- quickstart-output -->
```text
graph = {"A": [1, 2], "B": [3]}
std::map<std::string, std::vector<int>> graph = {"A": [1, 2], "B": [3]}
```

`ONLINE_JUDGE` 用于隔离本地调试代码，需同时保护头文件和调用点；提交前按评测平台的宏配置关闭调试输出。

## 打印数组区间

`debug.hpp` 提供 `debug_slice(a, l, r)`，按下标打印闭区间 `[l, r]`。适合容量为 `MAXN`、实际只使用一部分的数组。

```c++
constexpr int MAXN = 200'005;
int a[MAXN] = {0, 10, 20, 30};
int n = 3;
debug_slice(a, 1, n);  // a[1..3] = [10, 20, 30]
```

支持 C 数组和可通过 const 对象随机访问、具有尺寸的范围，例如 `std::array`、`std::vector` 和 `std::span`。仅遍历区间内的元素，无需复制。当 `0 <= l <= size` 且 `r == l - 1` 时输出 `[]`，因此 `debug_slice(a, 1, 0)` 可以展示从下标 1 开始的空区间。其他越界或反向区间会输出 `slice error`。

二维数组用 `debug_slice(dp, 首行, 末行, 首列, 末列)`，行和列都按闭区间处理。

```c++
int dp[3][4] = {{}, {0, 10, 20, 30}, {0, 40, 50, 60}};
debug_slice(dp, 1, 2, 1, 3);  // dp[1..2][1..3] = [[10, 20, 30], [40, 50, 60]]
```

例如，从下标 1 开始的 `n × m` 有效区域可用 `debug_slice(dp, 1, n, 1, m)`。二维形式也支持 `std::vector<std::vector<T>>` 等嵌套范围。列下标按选中的每一行检查，空行区间输出 `[]`，空列区间将选中的每行显示为 `[]`。

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

另有两个按需包含的工具：`timer.hpp` 提供 `get_time_us/ms/s(fn, args...)` 调用计时；`output.hpp` 提供 `oi_debug::print/println(...)`，将参数按顺序直接拼接输出。

## 结构体字段注册

在全局作用域注册公开数据成员，普通 `debug` 即可递归打印结构体及其嵌套容器。`debug.hpp` 已间接包含 `fields.hpp`，无需单独包含。

```c++
struct Edge {
    int to;
    long long weight;
};
OI_DEBUG_FIELDS(Edge, to, weight);

// debug(Edge{3, 10});
// Edge{3, 10} = Edge{to: 3, weight: 10}
```

- `OI_DEBUG_FIELDS(Type, field1, ...)` 支持 0～16 个字段，按注册顺序输出；命名空间中的类型写完整限定名。
- 字段通过 const 引用访问，不要求对象或字段可复制。可在公共头文件中注册，支持多个翻译单元包含。
- 已有的 `to_string() const` 优先于字段打印；差异比较仍按注册字段展开。
- 注册的字符串／字符字段使用嵌套元素的引号和转义规则。`visit_fields(value, visitor)` 将字段名称与 const 字段引用传给 visitor。
- 只注册自定义类型。私有成员、位域和静态成员不能直接注册；模板类型中的逗号可通过类型别名避开。

超过 16 个字段或需要模板偏特化时，直接实现 `oi_debug::field_traits<T>`，提供 `name` 和 `fields()`。例如：

```c++
template <typename T>
struct Box { T value; };

template <typename T>
struct oi_debug::field_traits<Box<T>> {
    inline static constexpr std::string_view name = "Box";
    static constexpr auto fields() {
        return std::make_tuple(oi_debug::make_field("value", &Box<T>::value));
    }
};
```

## 矩阵展示

按需包含 `oi_debug/matrix.hpp`，使用 `debug_matrix(matrix)` 打印名称和表格，或使用 `oi_debug::matrix_to_string(matrix, options)` 获取不带末尾换行的文本。

支持二维 C 数组，以及 `vector`、`array`、`span` 等可通过 const 对象访问、具有尺寸和随机访问能力的二维结构；不会复制矩阵。单元格复用已有序列化规则，字符串和字符会转义。

```c++
// dp 为已有的二维数组或容器。
debug_matrix(dp);
debug_matrix(dp, {.row_begin = 1, .col_begin = 2,
                  .max_rows = 5, .max_cols = 6,
                  .highlight = std::pair<std::size_t, std::size_t>{2, 3}});
auto text = oi_debug::matrix_to_string(dp, {.show_indices = false});
```

| `matrix_options` 字段 | 默认值 | 含义 |
| --- | --- | --- |
| `row_begin`、`col_begin` | `0` | 查看窗口的起始坐标 |
| `max_rows`、`max_cols` | `20`、`12` | 最大显示行数、列数；`0` 显示空窗口 |
| `show_indices` | `true` | 显示原始行列下标 |
| `highlight` | `std::nullopt` | 用 `[value]` 标记指定坐标，参与列宽计算 |

裁剪后会注明原矩阵尺寸和实际窗口的半开区间，下标保留原始坐标。空矩阵显示 `[]`；有空行时附尺寸说明。不规则行长、超出尺寸的起始坐标会返回 `matrix error: ...` 文本，不引入新的运行时异常。起始坐标等于对应尺寸表示空窗口。

布局先检查所有行的长度，再缓存可见单元格文本并计算列宽：行长检查为 O(R)，可见区域内每个单元格只序列化一次。控制字符不会产生额外终端行；当前按字节计算宽度，保证 ASCII 对齐，中文等宽字符的终端对齐尚未处理。暂不提供颜色、自定义 INF 哨兵或单元格格式化回调。

## 状态差异比较

按需包含 `oi_debug/diff.hpp`。用户显式保存前一份状态，调用 `debug_diff(before, after)` 输出变化；`oi_debug::diff(before, after, options)` 返回结果，便于自行处理。

| 类型 | 比较方式 |
| --- | --- |
| 整数、枚举、布尔、字符、字符串 | 叶子值比较，字符串整体比较 |
| 浮点数 | 精确比较，两个 NaN 视为同一状态，`+0` 与 `-0` 相同 |
| C 数组、vector、array、span | 按下标递归，长度变化报告新增或删除 |
| `std::map` | 按 key 查找匹配；双方应具有一致的 key 等价语义 |
| 注册结构体 | 按字段递归，无需结构体实现 `operator==` |
| 其他自定义叶子 | 需提供 `to_string() const` 和可用于 const 对象的 `operator==` |
| 非字符串裸指针 | 只比较地址，不展开所指对象 |

第一版比较的前后对象须为同一类型；不支持智能指针、无序容器、集合和其他尚未接入的复合类型。不可比较的叶子会产生编译期诊断，不会用打印文本代替值比较。

```c++
auto before = dp;
update(dp);
debug_diff(before, dp);
debug_diff(before, dp, {.max_changes = 10, .max_nodes = 1000, .max_depth = 8});

auto report = oi_debug::diff(before, dp);
auto text = oi_debug::diff_to_string(report);
```

`diff_report` 包含：

- `changes`：变化列表，每项有 `path`、`kind`、`before`、`after`；`kind` 为 `added`／`removed`／`modified`，前后文本通过 `optional<string>` 区分缺失和值。
- `visited_nodes`：访问过的结构节点与叶子总数，包含根节点。
- `stop_reason`：`none`／`max_changes`／`max_nodes`／`max_depth`；`complete()` 表示是否完成全部比较。

路径用 `.field`、`[index]`、`["key"]` 组合，根叶子用 `$` 表示。新增或删除的复合值继续展开到叶子，空结构单独记录。map 先按旧 map 顺序比较已有 key，再按新 map 顺序报告新增 key。序列采用下标对齐，头部插入可能报告多个位置变化。

`diff_options` 默认最多保存 100 项变化、访问 100000 个节点、递归深度 64（根深度为 0）。达到变化条数上限立即停止，即使最后一项恰好是最后一个变化，也保守标记为未完成；节点或深度限制在尝试继续访问时生效。变化上限为 `0` 时，发现第一个变化便停止且不保存。停止时明确显示 `remaining values not compared`，不会报告未经遍历的变化总数。节点预算约束结构比较的访问量，不限制字符串本身的长度或用户比较／序列化函数的耗时。

未变化的叶子不会序列化值，比较过程不复制输入对象。快照由用户管理：复制 C 数组需自行使用 `std::array`、`vector` 等拥有数据的容器，复制 `span` 或含共享指针的对象也不构成独立快照。用户比较和序列化函数的异常向外传播。

## 完整示例：观察一次 DP 状态更新

<!-- showcase-source -->
```c++
#include <vector>
#ifndef ONLINE_JUDGE
#include "oi_debug/debug.hpp"
#include "oi_debug/matrix.hpp"
#include "oi_debug/diff.hpp"
#endif

struct State {
    int round;
    std::vector<std::vector<int>> dp;
};
#ifndef ONLINE_JUDGE
OI_DEBUG_FIELDS(State, round, dp);
#endif

int main() {
    State state{0, {{0, 0}, {0, 2}}};
#ifndef ONLINE_JUDGE
    const auto before = state;
#endif
    state.round = 1;
    state.dp[1][1] = 3;
#ifndef ONLINE_JUDGE
    debug(state);
    debug_matrix(state.dp);
    debug_diff(before, state);
#endif
}
```

输出：

<!-- showcase-output -->
```text
state = State{round: 1, dp: [[0, 0], [0, 3]]}
state.dp =
   0 1
0  0 0
1  0 3
before -> state:
  .round: 0 -> 1
  .dp[1][1]: 2 -> 3
```

## 设计思路

1. **在编译期选择格式。** 通过 concepts 和模板重载区分字符串、映射、容器等类型，递归组合输出。表达式名称来自宏字符串化；类型名称由 `TypeParseTraits` 结合编译器签名和常见类型特化生成。
2. **共享缓冲区，递归追加。** 嵌套值直接追加到同一个 `std::string`，减少中间字符串的构造与复制；整数和浮点数优先用 `std::to_chars` 转换，标准库缺少 long double 重载时使用 `snprintf`。
3. **让约束对应实际调用。** 自定义序列化和 range 按 `const` 访问能力检查，类型约束错误在编译期报告；分配异常与用户序列化异常向外传播，交由调用方统一处理。
4. **通过公开接口保留原对象。** 对 queue、stack、priority_queue 复制副本，再按访问顺序弹出并打印。这个方案遵循标准接口，元素须可复制；副本的时间与空间开销随容器大小增长。

## 使用约定

- 输出到 **`std::cout`** 并追加换行；交互调试需要立即显示时，可显式调用 `std::cout.flush()`。
- 每次传一个表达式，支持花括号和模板逗号；`debug(a, b)` 按逗号运算符求值，只打印 `b`。
- `debug_matrix(matrix, options)`、`debug_diff(before, after, options)` 支持可选配置，对每个输入表达式只求值一次；输入表达式自身含模板／花括号逗号时加一层圆括号，或使用普通函数接口。两项输入的求值先后顺序不作保证。
- range 必须可通过 const 对象遍历；单遍输入 range 会消耗输入，C++20 的 `filter_view` 等仅提供非 const 遍历的 view 需先收集为容器。
- 无序容器按自身迭代顺序输出，类型名称采用当前编译器的拼写；智能指针的递归展开适用于无环对象图。

## 质量验证

在仓库根目录运行，构建产物均放在临时目录中：

```bash
python3 tests/run.py
python3 tests/run.py --sanitize
```

已在 **GCC 15、Clang 21（均使用 libstdc++）** 下以 C++20、`-Wall -Wextra -pedantic -Werror` 验证。回归覆盖嵌套格式、字符边界、异常传播、单遍 range、适配器顺序与复制、计时器引用返回值、字段注册与引用访问、矩阵布局与裁剪、递归差异与预算停止，以及文档示例、头文件独立包含和双翻译单元链接；同时通过 ASan、UBSan 与泄漏检查。

计时工具 `timer.hpp` 使用 Itanium ABI 的 `__cxa_demangle` 解析可调用对象的类型名称。

## 贡献与许可

欢迎提交 Issue 或 PR。问题报告请附最小复现、编译器版本和预期输出；新增类型或修改格式时，请补充对应回归用例。

项目采用 [GPL-3.0](LICENSE) 许可证。
