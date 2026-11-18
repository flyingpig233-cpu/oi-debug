#include <array>
#ifndef ONLINE_JUDGE
#include "oi_debug/diff.hpp"
#include "oi_debug/matrix.hpp"
#endif

int main() {
    constexpr int weight = 2;
    constexpr int value = 3;
    constexpr int capacity_limit = 4;
    using Table = std::array<std::array<int, capacity_limit + 1>, 2>;

    Table dp{};
    for (int capacity = weight; capacity <= capacity_limit; ++capacity) {
        // Bug: the current row can already include this item.
        dp[1][capacity] = dp[1][capacity - weight] + value;
    }

#ifndef ONLINE_JUDGE
    const Table expected{{{}, {0, 0, 3, 3, 3}}};
    debug_diff(expected, dp);
    debug_matrix(dp, {.highlight = {{1, 4}}});
#endif
}
