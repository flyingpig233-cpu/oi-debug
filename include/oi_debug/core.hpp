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
concept Character = std::is_same_v<std::remove_cvref_t<T>, char>;

template <typename T>
concept Float = std::is_floating_point_v<T>;

template <typename T>
concept Boolean = std::is_same_v<bool, std::decay_t<T>>;

template <typename T>
concept String = !std::is_null_pointer_v<std::decay_t<T>> &&
                 std::convertible_to<const std::remove_reference_t<T> &,
                                     std::string_view>;

template <typename T>
concept has_to_string = requires(const std::remove_reference_t<T> &a) {
    { a.to_string() } -> std::convertible_to<std::string>;
};

template <typename T>
concept FIFO_Container = !std::ranges::range<T> &&
    std::copy_constructible<T> &&
    requires(T &a, const T &b) {
        typename T::value_type;
        requires std::copy_constructible<typename T::value_type>;
        a.pop();
        b.front();
        { b.empty() } -> std::convertible_to<bool>;
    };

template <typename T>
concept LIFO_Container = !std::ranges::range<T> && !FIFO_Container<T> &&
    std::copy_constructible<T> &&
    requires(T &a, const T &b) {
        typename T::value_type;
        requires std::copy_constructible<typename T::value_type>;
        a.pop();
        b.top();
        { b.empty() } -> std::convertible_to<bool>;
    };


template <typename T>
concept Map = std::ranges::input_range<const T> && requires(const T &a) {
    typename std::decay_t<T>::key_type;
    typename std::decay_t<T>::mapped_type;
    (*std::ranges::begin(a)).first;
    (*std::ranges::begin(a)).second;
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
