#ifndef OI_DEBUG_DEBUG_HPP
#define OI_DEBUG_DEBUG_HPP

#include "get_type.hpp"
#include "to_string.hpp"
#include <iostream>

namespace oi_debug {

template <typename T>
inline void debug_with_type_impl(const T &t, const char *var_name) {
    std::cout << TypeParseTraits<T>::get_name() << " " << var_name << " = "
              << oi_debug::to_string(t) << std::endl;
}

template <typename T>
inline void debug_impl(const T &t, const char *var_name) {
    std::cout << var_name << " = " << oi_debug::to_string(t) << std::endl;
}

} // namespace oi_debug

#define debug_with_type(...)                                                   \
    oi_debug::debug_with_type_impl((__VA_ARGS__), #__VA_ARGS__)
#define debug(...) oi_debug::debug_impl((__VA_ARGS__), #__VA_ARGS__)

#endif // OI_DEBUG_DEBUG_HPP
