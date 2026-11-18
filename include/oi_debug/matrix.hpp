#ifndef OI_DEBUG_MATRIX_HPP
#define OI_DEBUG_MATRIX_HPP

#include "to_string.hpp"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace oi_debug {

struct matrix_options {
    std::size_t row_begin = 0;
    std::size_t col_begin = 0;
    std::size_t max_rows = 20;
    std::size_t max_cols = 12;
    bool show_indices = true;
    std::optional<std::pair<std::size_t, std::size_t>> highlight = std::nullopt;
};

template <typename T>
concept Matrix = !String<T> && std::ranges::random_access_range<const T> &&
    std::ranges::sized_range<const T> &&
    std::ranges::random_access_range<std::ranges::range_reference_t<const T>> &&
    std::ranges::sized_range<std::ranges::range_reference_t<const T>>;

namespace detail {

inline void append_matrix_padded(std::string &buffer, std::string_view text,
                                 std::size_t width) {
    buffer.append(width - text.size(), ' ');
    buffer.append(text);
}

// User-provided serializers may return literal control characters. Keep every
// cell on one terminal line without escaping already formatted backslashes.
inline std::string matrix_cell_text(std::string_view text) {
    std::string result;
    for (unsigned char c : text) {
        if (c == '\n') {
            result.append("\\n");
        } else if (c == '\r') {
            result.append("\\r");
        } else if (c == '\t') {
            result.append("\\t");
        } else if (c < 0x20 || c == 0x7f) {
            result.push_back('\\');
            result.push_back(static_cast<char>('0' + (c >> 6)));
            result.push_back(static_cast<char>('0' + ((c >> 3) & 7)));
            result.push_back(static_cast<char>('0' + (c & 7)));
        } else {
            result.push_back(static_cast<char>(c));
        }
    }
    return result;
}

} // namespace detail

template <Matrix T>
[[nodiscard]] std::string matrix_to_string(const T &matrix,
                                           matrix_options options = {}) {
    const auto rows = static_cast<std::size_t>(std::ranges::size(matrix));
    const auto begin = std::ranges::begin(matrix);
    using row_difference = std::ranges::range_difference_t<const T>;
    const auto row_at = [&](std::size_t r) -> decltype(auto) {
        return begin[static_cast<row_difference>(r)];
    };
    const auto cols = rows == 0 ? std::size_t{0}
        : static_cast<std::size_t>(std::ranges::size(row_at(0)));
    for (std::size_t r = 1; r < rows; ++r) {
        const auto size = static_cast<std::size_t>(std::ranges::size(row_at(r)));
        if (size != cols) {
            return "matrix error: row " + std::to_string(r) + " has " +
                std::to_string(size) + " columns; expected " + std::to_string(cols);
        }
    }
    if (options.row_begin > rows || options.col_begin > cols) {
        return "matrix error: window begins outside the matrix";
    }
    const auto height = std::min(options.max_rows, rows - options.row_begin);
    const auto width = std::min(options.max_cols, cols - options.col_begin);
    std::string result;
    if (height == 0 || width == 0) {
        result = "[]";
    } else {
        std::vector<std::size_t> widths(width, 0);
        std::vector<std::string> labels;
        if (options.show_indices) {
            for (std::size_t c = 0; c < width; ++c) {
                labels.push_back(std::to_string(options.col_begin + c));
                widths[c] = labels.back().size();
            }
        }
        std::vector<std::vector<std::string>> cells;
        for (std::size_t r = 0; r < height; ++r) {
            auto &&row = row_at(options.row_begin + r);
            const auto cell_begin = std::ranges::begin(row);
            using column_difference = std::ranges::range_difference_t<decltype(row)>;
            std::vector<std::string> line;
            for (std::size_t c = 0; c < width; ++c) {
                auto text = detail::matrix_cell_text(quoted_if_string(
                    cell_begin[static_cast<column_difference>(options.col_begin + c)]));
                if (options.highlight == std::pair{options.row_begin + r,
                                                   options.col_begin + c}) {
                    text = "[" + text + "]";
                }
                widths[c] = std::max(widths[c], text.size());
                line.push_back(std::move(text));
            }
            cells.push_back(std::move(line));
        }
        const auto index_width = std::to_string(options.row_begin + height - 1).size();
        if (options.show_indices) {
            result.append(index_width + 1, ' ');
            for (std::size_t c = 0; c < width; ++c) {
                result.push_back(' ');
                detail::append_matrix_padded(result, labels[c], widths[c]);
            }
            result.push_back('\n');
        }
        for (std::size_t r = 0; r < height; ++r) {
            if (options.show_indices) {
                detail::append_matrix_padded(result,
                    std::to_string(options.row_begin + r), index_width);
                result.push_back(' ');
            }
            for (std::size_t c = 0; c < width; ++c) {
                if (options.show_indices || c != 0) {
                    result.push_back(' ');
                }
                detail::append_matrix_padded(result, cells[r][c], widths[c]);
            }
            if (r + 1 != height) {
                result.push_back('\n');
            }
        }
    }
    if (options.row_begin != 0 || options.col_begin != 0 ||
        height != rows || width != cols || (rows != 0 && cols == 0)) {
        result.append("\n... (rows [").append(std::to_string(options.row_begin));
        result.append(", ").append(std::to_string(options.row_begin + height));
        result.append(") of ").append(std::to_string(rows));
        result.append(", columns [").append(std::to_string(options.col_begin));
        result.append(", ").append(std::to_string(options.col_begin + width));
        result.append(") of ").append(std::to_string(cols)).append(")");
    }
    return result;
}

template <Matrix T>
void debug_matrix_impl(const T &matrix, std::string_view name,
                       matrix_options options = {}) {
    std::string content(name);
    content.append(" =\n").append(matrix_to_string(matrix, options));
    content.push_back('\n');
    std::cout.write(content.data(), static_cast<std::streamsize>(content.size()));
}

} // namespace oi_debug

#define debug_matrix(matrix, ...) \
    ::oi_debug::debug_matrix_impl((matrix), #matrix __VA_OPT__(,) __VA_ARGS__)

#endif // OI_DEBUG_MATRIX_HPP
