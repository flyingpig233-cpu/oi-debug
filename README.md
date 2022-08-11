# oi-debug
一套代码debug库，方便查看和调试变量

## 预览
### 示例代码(在`./include/oi_debug/test.cpp`)

```c++
#include "debug.hpp"
#include <bits/stdc++.h>

int main() {

    std::cout << "=========Without type=========" << std::endl;
    std::vector<std::string> strs {"Hello", ",", " world!"};
    debug(strs);
    std::tuple<int, double, std::vector<std::string>> tup(114514, 3.1415926535897, strs);
    debug(tup);
    std::map<std::string, int> m { {"China", 1}, {"America", 2}, {"Russia", 3} };
    debug(m);
    bool ok = true;
    debug(ok);
    std::cout << "==============================" << std::endl;
    std::cout << std::endl;
    std::cout << "=========With type=========" << std::endl;
    debug_with_type(strs);
    debug_with_type(tup);
    debug_with_type(m);
    debug_with_type(ok);
    std::cout << "==============================" << std::endl;
}

```

### 输出
```c++
=========Without type=========
strs = ["Hello", ",", " world!"]
tup = (114514, 3.1415926535897, ["Hello", ",", " world!"])
m = {"America": 2, "China": 1, "Russia": 3}
ok = true
==============================

=========With type=========
std::vector<std::string> strs = ["Hello", ",", " world!"]
std::tuple<int, double, std::vector<std::string>> tup = (114514, 3.1415926535897, ["Hello", ",", " world!"])
std::map<std::string, int> m = {"America": 2, "China": 1, "Russia": 3}
bool ok = true
==============================

```

## 特点
支持几乎所有STL容器（包括pair和tuple）以及基本类型，甚至是形如`int a[][][][][][]`这样的无限嵌套

## 提示
您在写题目的时候可以这样
```c++
#ifndef ONLINE_JUDGE
#include "oi_debug/debug.hpp"
#endif // ONLINE_JUDGE
```

## 使用
只需将`include`下面的目录放到题目目录下面，或者将其放进编译器会自动包含的目录下面，如`/usr/include/`，或是`windows`下的`mingw/include`等

## 注意
本代码使用了`concept`等只有**C++20**及以上版本才支持的新特性，在编译代码时必须加上`-std=c++20`选项

## 警告
本库目前不支持`MSVC`编译器（原因是调用了`CXXABI`库来获取变量名）

## 贡献
如果您对这个库满意的话，可以留下您的star哦，希望能帮到您～
如果对这个库有什么好的建议，欢迎提PR～
