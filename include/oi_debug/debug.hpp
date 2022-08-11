#ifndef _DEBUG__
#define _DEBUG__

#include "to_string.hpp"
#include <cxxabi.h>
#include <iostream>
#include <queue>

#define debug_with_type(var) _debug_with_type((var), (#var))
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

template <typename T> void _debug_with_type(T &t, const char *var_name) {
    std::cout << type_name<T>() << " " << var_name << " = "
              << oi_debug::to_string(t) << std::endl;
}

template <typename T> void _debug(T &t, const char *var_name) {
    std::cout << var_name << " = "
              << oi_debug::to_string(t) << std::endl;
}

#endif // _DEBUG__
