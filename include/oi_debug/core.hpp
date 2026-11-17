#ifndef OI_DEBUG_CORE_HPP
#define OI_DEBUG_CORE_HPP

#include <concepts>
#include <cstddef>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

namespace oi_debug {

#if defined(__GNUC__) || defined(__clang__)
#define OI_DEBUG_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define OI_DEBUG_FORCE_INLINE inline
#endif

template <typename T>
concept Character = std::is_same_v<T, char>;

template <typename T>
concept Float = std::is_floating_point_v<T>;

template <typename T>
concept Boolean = std::is_same_v<bool, std::decay_t<T>>;

template <typename T>
concept String = !std::is_null_pointer_v<std::decay_t<T>> &&
                 std::convertible_to<T, std::string_view>;

template <typename T>
concept has_to_string = requires(T &&a) {
    a.to_string();
};

template <typename T>
concept FIFO_Container = !std::ranges::range<T> && requires(T & a) {
    a.pop();
    a.front();
};

template <typename T>
concept LIFO_Container = !FIFO_Container<T> && requires(T & a) {
    a.top();
    a.pop();
};


template <typename T>
concept Map = std::ranges::range<T> && requires(T &a) {
    typename std::decay_t<T>::key_type;
    typename std::decay_t<T>::mapped_type;
};

consteval const std::string_view colon() {
    return ": ";
}
consteval const std::string_view comma() {
    return ", ";
}

consteval const std::string_view curly_braces(bool i) {
    return i ? "}" : "{";
}

consteval const std::string_view square_brackets(bool i) {
    return i ? "]" : "[";
}

consteval const std::string_view brackets(bool i) {
    return i ? ")" : "(";
}

constexpr const std::string_view true_false(bool i) {
    return i ? "true" : "false";
}

consteval const std::string_view quotes() {
    return "'";
}
consteval const std::string_view double_quotes() {
    return "\"";
}

consteval const std::string_view new_line() {
    return "\n";
}

constexpr char digit(size_t i) {
    return "0123456789abcdef"[i & 0xF];
}

} // namespace oi_debug

#endif // OI_DEBUG_CORE_HPP
