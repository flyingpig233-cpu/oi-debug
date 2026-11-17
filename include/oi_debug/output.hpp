#ifndef OI_DEBUG_OUTPUT_HPP
#define OI_DEBUG_OUTPUT_HPP

#include "core.hpp"
#include "to_string.hpp"
#include <iostream>

namespace oi_debug {

template <typename... Args>
void print(const Args &...args) {
    const auto content = oi_debug::concat(args...);
    std::cout.write(content.data(),
                    static_cast<std::streamsize>(content.size()));
}

template <typename... Args>
void println(const Args &...args) {
    auto content = oi_debug::concat(args...);
    content.push_back('\n');
    std::cout.write(content.data(),
                    static_cast<std::streamsize>(content.size()));
}

} // namespace oi_debug

#endif // OI_DEBUG_OUTPUT_HPP
