#ifndef _OIDEBUG_OUTPUT_HPP__
#define _OIDEBUG_OUTPUT_HPP__

#include "core.hpp"
#include "to_string.hpp"

namespace oi_debug {

template <typename T>
void __FORCE_INLINE__ inline put_elem(const T &elem) {
    auto content = to_string(elem);
    fwrite(content.c_str(), 1, content.size(), stdout);
}

template <Character char_type_t = oi_debug::char_type, typename... Args>
constexpr void print(const Args &...args) {
    (put_elem(args), ...);
}

template <Character char_type_t = oi_debug::char_type, typename... Args>
constexpr void println(const Args &...args) {
    (put_elem(args), ...);
    put_elem(new_line());
}



} // namespace oi_debug

#endif // _OIDEBUG_OUTPUT_HPP__