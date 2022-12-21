#ifndef _DEBUG__
#define _DEBUG__

#include "get_type.hpp"
#include "to_string.hpp"
#include "output.hpp"
#include <cxxabi.h>
#include <iostream>
#include <queue>


#define debug(var) _debug((var), (#var))

template <typename T> void _Variable_debug(const T &t, const char *var_name, const bool HaveType) {
    if(HaveType == true){
        oi_debug::println(TypeParseTraits<T>::get_name(), " ", var_name, " = ", t);
    }
    else if(HaveType == false){
        oi_debug::println(var_name, " = ", t);
    }
}

#endif // _DEBUG__
