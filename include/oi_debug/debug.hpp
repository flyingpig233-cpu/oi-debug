#ifndef _DEBUG__
#define _DEBUG__

#include <bg_helper/output.hpp>
#include <cxxabi.h>
#include <iostream>
#include <queue>

#define debug(var) _debug((var), (#var))

template <typename T> std::string type_name() {
    int status;
    std::string tname = typeid(T).name();
    char *demangled_name =
        abi::__cxa_demangle(tname.c_str(), NULL, NULL, &status);
    if (status == 0) {
        tname = demangled_name;
        std::free(demangled_name);
    }
    return tname;
}

template <typename T> void _debug(T &t, const char *var_name) {
    std::cout << type_name<T>() << " " << var_name << " = "
              << bg_helper::to_string(t) << std::endl;
}

template <typename T> void _debug(std::queue<T> q, const char *var_name) {
    std::cout << "std::queue<" << type_name<T>() << "> " << var_name << " = [";
    while (!q.empty()) {
        std::cout << bg_helper::quoted_if_string(q.front());
        q.pop();
        if (!q.empty()) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

#endif // _DEBUG__
