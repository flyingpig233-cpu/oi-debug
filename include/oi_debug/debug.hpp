#ifndef _DEBUG__
#define _DEBUG__

#include "get_type.hpp"
#include "to_string.hpp"
#include "output.hpp"
#include <cxxabi.h>
#include <iostream>
#include <queue>

#define debug_with_type(var) _debug_with_type((var), (#var))
#define debug(var) _debug((var), (#var))

template <typename T> void _debug_with_type(const T &t, const char *var_name) {
    oi_debug::println(TypeParseTraits<T>::get_name(), " ", var_name, " = ", t);
}

template <typename T> void _debug(const T &t, const char *var_name) {
    oi_debug::println(var_name, " = ", t);
}

#endif // _DEBUG__
