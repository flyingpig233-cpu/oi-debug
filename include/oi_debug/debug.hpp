#ifndef OI_DEBUG_DEBUG_HPP
#define OI_DEBUG_DEBUG_HPP

#include "get_type.hpp"
#include "to_string.hpp"
#include <cstddef>
#include <iostream>
#include <ranges>
#include <string>

namespace oi_debug {

namespace detail {

inline void write_debug_line(std::string &content) {
    content.push_back('\n');
    std::cout.write(content.data(),
                    static_cast<std::streamsize>(content.size()));
}

template <typename T>
concept SliceRange = std::ranges::random_access_range<T> &&
                     std::ranges::sized_range<T>;

struct SliceInterval {
    std::ptrdiff_t first;
    std::ptrdiff_t last;

    void append_label(std::string &label) const {
        label.append("[").append(std::to_string(first)).append("..")
             .append(std::to_string(last)).append("]");
    }

    bool fits(std::size_t size) const {
        return first >= 0 && static_cast<std::size_t>(first) <= size &&
            last >= first - 1 &&
            (last < 0 || static_cast<std::size_t>(last) < size);
    }

    template <SliceRange T>
    auto subrange(T &&value) const {
        using difference_type = std::ranges::range_difference_t<T>;
        const auto begin = std::ranges::begin(value);
        const auto from = begin + static_cast<difference_type>(first);
        const auto to = first > last
            ? from
            : begin + static_cast<difference_type>(last) + 1;
        return std::ranges::subrange{from, to};
    }
};

} // namespace detail

template <typename T>
inline void debug_with_type_impl(const T &t, const char *var_name) {
    std::string content(TypeParseTraits<T>::get_name());
    content.push_back(' ');
    content.append(var_name).append(" = ");
    detail::append_to(content, t);
    detail::write_debug_line(content);
}

template <typename T>
inline void debug_impl(const T &t, const char *var_name) {
    std::string content(var_name);
    content.append(" = ");
    detail::append_to(content, t);
    detail::write_debug_line(content);
}

template <typename T>
requires detail::SliceRange<const T>
inline void debug_slice_impl(const T &value, std::ptrdiff_t left,
                             std::ptrdiff_t right, const char *var_name) {
    const detail::SliceInterval interval{left, right};
    std::string label(var_name);
    interval.append_label(label);

    const auto size = static_cast<std::size_t>(std::ranges::size(value));
    if (!interval.fits(size)) {
        debug_impl("slice error: invalid interval for size " +
                       std::to_string(size),
                   label.c_str());
        return;
    }
    debug_impl(interval.subrange(value), label.c_str());
}

template <typename T>
requires(detail::SliceRange<const T> &&
         detail::SliceRange<std::ranges::range_reference_t<const T>>)
inline void debug_slice_impl(const T &value, std::ptrdiff_t row_first,
                             std::ptrdiff_t row_last, const char *var_name,
                             std::ptrdiff_t col_first, std::ptrdiff_t col_last) {
    const detail::SliceInterval rows{row_first, row_last};
    const detail::SliceInterval cols{col_first, col_last};
    std::string label(var_name);
    rows.append_label(label);
    cols.append_label(label);

    const auto height = static_cast<std::size_t>(std::ranges::size(value));
    if (!rows.fits(height)) {
        debug_impl("slice error: invalid row interval for size " +
                       std::to_string(height),
                   label.c_str());
        return;
    }

    std::string content(label);
    content.append(" = ").append(square_brackets(0));
    const auto first_row = static_cast<std::size_t>(row_first);
    auto row_index = first_row;
    for (auto &&row : rows.subrange(value)) {
        const auto width = static_cast<std::size_t>(std::ranges::size(row));
        if (!cols.fits(width)) {
            debug_impl("slice error: invalid column interval for row " +
                           std::to_string(row_index) + " of size " +
                           std::to_string(width),
                       label.c_str());
            return;
        }
        if (row_index != first_row) {
            content.append(comma());
        }
        detail::append_to(content, cols.subrange(row));
        ++row_index;
    }
    content.append(square_brackets(1));
    detail::write_debug_line(content);
}

} // namespace oi_debug

#define debug_with_type(...)                                                   \
    oi_debug::debug_with_type_impl((__VA_ARGS__), #__VA_ARGS__)
#define debug(...) oi_debug::debug_impl((__VA_ARGS__), #__VA_ARGS__)
#define debug_slice(array, left, right, ...)                                    \
    ::oi_debug::debug_slice_impl((array), (left), (right),                      \
                                #array __VA_OPT__(,) __VA_ARGS__)

#endif // OI_DEBUG_DEBUG_HPP
