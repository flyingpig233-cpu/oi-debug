#ifndef OI_DEBUG_OUTPUT_HPP
#define OI_DEBUG_OUTPUT_HPP

#include "core.hpp"
#include "to_string.hpp"
#include <iostream>

namespace oi_debug {

template <typename T>
void put_elem(const T &elem) {
    const auto content = to_string(elem);
    std::cout.write(content.data(),
                    static_cast<std::streamsize>(content.size()));
}

template <typename... Args>
void print(const Args &...args) {
    (put_elem(args), ...);
}

template <typename... Args>
void println(const Args &...args) {
    (put_elem(args), ...);
    put_elem(new_line());
}

} // namespace oi_debug

#endif // OI_DEBUG_OUTPUT_HPP
