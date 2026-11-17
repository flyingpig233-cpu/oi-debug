#ifndef OI_DEBUG_DEBUG_HPP
#define OI_DEBUG_DEBUG_HPP

#include "get_type.hpp"
#include "to_string.hpp"
#include <iostream>
#include <string>

namespace oi_debug {

template <typename T>
inline void debug_with_type_impl(const T &t, const char *var_name) {
    std::string content(TypeParseTraits<T>::get_name());
    content.push_back(' ');
    content.append(var_name).append(" = ");
    detail::append_to(content, t);
    content.push_back('\n');
    std::cout.write(content.data(),
                    static_cast<std::streamsize>(content.size()));
}

template <typename T>
inline void debug_impl(const T &t, const char *var_name) {
    std::string content(var_name);
    content.append(" = ");
    detail::append_to(content, t);
    content.push_back('\n');
    std::cout.write(content.data(),
                    static_cast<std::streamsize>(content.size()));
}

} // namespace oi_debug

#define debug_with_type(...)                                                   \
    oi_debug::debug_with_type_impl((__VA_ARGS__), #__VA_ARGS__)
#define debug(...) oi_debug::debug_impl((__VA_ARGS__), #__VA_ARGS__)

#endif // OI_DEBUG_DEBUG_HPP
