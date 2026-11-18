#include "oi_debug/matrix.hpp"
#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <span>
#include <sstream>
#include <string>
#include <vector>

struct Cell {
    inline static int conversions = 0;
    int value;
    Cell(int value) : value(value) {}
    Cell(const Cell &) = delete;
    std::string to_string() const {
        ++conversions;
        return std::to_string(value);
    }
};

struct Control {
    std::string to_string() const { return std::string("a\nb\r\t\x1b\0", 7); }
};

static_assert(oi_debug::Matrix<int[2][3]>);
static_assert(!oi_debug::Matrix<std::vector<int>>);

int main() {
    const int matrix[2][2] = {{1, -20}, {300, 4}};
    assert(oi_debug::matrix_to_string(matrix) ==
           "     0   1\n0    1 -20\n1  300   4");
    assert(oi_debug::matrix_to_string(matrix, {.show_indices = false}) ==
           "  1 -20\n300   4");
    assert(oi_debug::matrix_to_string(matrix, {.highlight = std::pair{0u, 1u}}) ==
           "     0     1\n0    1 [-20]\n1  300     4");
    assert(oi_debug::matrix_to_string(matrix, {.row_begin = 1, .col_begin = 1}) ==
           "   1\n1  4\n... (rows [1, 2) of 2, columns [1, 2) of 2)");
    assert(oi_debug::matrix_to_string(matrix, {.max_rows = 0}) ==
           "[]\n... (rows [0, 0) of 2, columns [0, 2) of 2)");
    assert(oi_debug::matrix_to_string(matrix, {.max_cols = 1}) ==
           "     0\n0    1\n1  300\n... (rows [0, 2) of 2, columns [0, 1) of 2)");
    assert(oi_debug::matrix_to_string(matrix, {.row_begin = 3}) ==
           "matrix error: window begins outside the matrix");
    assert(oi_debug::matrix_to_string(matrix, {.row_begin = 2}).starts_with("[]\n"));
    const std::vector<std::vector<int>> empty;
    assert(oi_debug::matrix_to_string(empty) == "[]");
    assert(oi_debug::matrix_to_string(std::vector<std::vector<int>>(2)) ==
           "[]\n... (rows [0, 2) of 2, columns [0, 0) of 0)");
    assert(oi_debug::matrix_to_string(std::vector<std::vector<int>>{{1,2}, {3}}) ==
           "matrix error: row 1 has 1 columns; expected 2");
    // Validation includes rows outside the displayed window.
    assert(oi_debug::matrix_to_string(std::vector<std::vector<int>>{{1,2}, {3}},
           {.max_rows = 1}).starts_with("matrix error:"));
    std::array<std::array<int, 2>, 2> array{{{{1, -20}}, {{300, 4}}}};
    assert(oi_debug::matrix_to_string(std::span(array)) == oi_debug::matrix_to_string(matrix));
    const int one[] = {1, -20};
    const int two[] = {300, 4};
    const std::array<std::span<const int>, 2> rows{one, two};
    assert(oi_debug::matrix_to_string(rows) == oi_debug::matrix_to_string(matrix));
    const char chars[1][2] = {{'\n', 'x'}};
    assert(oi_debug::matrix_to_string(chars, {.show_indices = false}) == "'\\n' 'x'");
    const std::string strings[1][2] = {{"a\nb", "\""}};
    assert(oi_debug::matrix_to_string(strings, {.show_indices = false}) ==
           "\"a\\nb\" \"\\\"\"");
    const Control controls[1][1];
    assert(oi_debug::matrix_to_string(controls, {.show_indices = false}) ==
           "a\\nb\\r\\t\\033\\000");
    const double floats[1][2] = {{std::numeric_limits<double>::infinity(),
                                std::numeric_limits<double>::quiet_NaN()}};
    assert(oi_debug::matrix_to_string(floats).find("inf") != std::string::npos);
    assert(oi_debug::matrix_to_string(floats).find("nan") != std::string::npos);
    const Cell counted[2][2] = {{1,2}, {3,4}};
    const auto clipped = oi_debug::matrix_to_string(counted, {.max_rows = 1, .max_cols = 1});
    assert(Cell::conversions == 1 && clipped.find('1') != std::string::npos);
    std::ostringstream output;
    auto *previous = std::cout.rdbuf(output.rdbuf());
    int calls = 0;
    const auto get_matrix = [&]() -> const auto & { ++calls; return matrix; };
    debug_matrix(get_matrix(), {.show_indices = false});
    std::cout.rdbuf(previous);
    assert(calls == 1);
    assert(output.str() == "get_matrix() =\n  1 -20\n300   4\n");
    std::cout << "matrix checks passed\n";
}
