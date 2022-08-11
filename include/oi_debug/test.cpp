#include "debug.hpp"
#include <bits/stdc++.h>

int main() {

    std::cout << "=========Without type=========" << std::endl;
    std::vector<std::string> strs {"Hello", ",", " world!"};
    debug(strs);
    std::tuple<int, double, std::vector<std::string>> tup(114514, 3.1415926535897, strs);
    debug(tup);
    std::map<std::string, int> m { {"China", 1}, {"America", 2}, {"Rossia", 3} };
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
