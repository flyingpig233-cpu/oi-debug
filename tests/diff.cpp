#include "oi_debug/diff.hpp"
#include "registered.hpp"
#include <array>
#include <cassert>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <vector>

struct Counted {
    inline static int conversions = 0;
    inline static int comparisons = 0;
    int value;
    explicit Counted(int value) : value(value) {}
    Counted(const Counted &) = delete;
    std::string to_string() const {
        ++conversions;
        return std::to_string(value);
    }
    bool operator==(const Counted &other) const {
        ++comparisons;
        return value == other.value;
    }
};

struct State {
    int distance;
    std::vector<fixture::Record> records;
    std::string to_string() const { return "same rendering"; }
};
OI_DEBUG_FIELDS(State, distance, records);

struct Empty {};
OI_DEBUG_FIELDS(Empty);

struct Tree {
    int value;
    std::vector<Tree> children;
};
OI_DEBUG_FIELDS(Tree, value, children);

enum class Flag { off, on };

int main() {
    auto scalar = oi_debug::diff(1, 2);
    assert(scalar.complete() && scalar.visited_nodes == 1);
    assert(scalar.changes.size() == 1 && scalar.changes[0].path == "$");
    assert(scalar.changes[0].kind == oi_debug::diff_kind::modified);
    assert(oi_debug::diff_to_string(scalar) == "  $: 1 -> 2");
    assert(oi_debug::diff_to_string(oi_debug::diff(1,1)) == "  no changes");
    assert(oi_debug::diff(Flag::off, Flag::on).changes.size() == 1);
    const std::vector<std::vector<int>> before{{1,2}, {3,4}};
    const std::vector<std::vector<int>> after{{1,5}, {3}, {6,7}};
    const auto nested = oi_debug::diff(before, after);
    assert(nested.complete() && nested.changes.size() == 4);
    assert(oi_debug::diff_to_string(nested) ==
           "  [0][1]: 2 -> 5\n  [1][1]: 4 -> <missing>\n"
           "  [2][0]: <missing> -> 6\n  [2][1]: <missing> -> 7");
    assert(nested.changes[1].kind == oi_debug::diff_kind::removed);
    assert(!nested.changes[1].after && nested.changes[1].before == "4");
    assert(nested.changes[2].kind == oi_debug::diff_kind::added);
    assert(!nested.changes[2].before && nested.changes[2].after == "6");
    assert(before == std::vector<std::vector<int>>({{1,2},{3,4}}));
    const int old_array[2][2] = {{1,2},{3,4}};
    const int new_array[2][2] = {{1,2},{3,5}};
    assert(oi_debug::diff(old_array,new_array).changes[0].path == "[1][1]");
    const std::array<int,2> a{1,2}, b{1,3};
    assert(oi_debug::diff(a,b).changes[0].path == "[1]");
    assert(oi_debug::diff(std::span<const int>(a), std::span<const int>(b)).changes.size() == 1);
    assert(oi_debug::diff(std::span<const int>(a).first(1), std::span<const int>(b)).changes[0].kind ==
           oi_debug::diff_kind::added);
    assert(oi_debug::diff(std::vector<bool>{true, false}, std::vector<bool>{false, false, true}).changes.size() == 2);
    const State old_state{12, {{1, {2,3}}}};
    const State new_state{9, {{1, {2,4}}}};
    assert(oi_debug::diff_to_string(oi_debug::diff(old_state, new_state)) ==
           "  .distance: 12 -> 9\n  .records[0].samples[1]: 3 -> 4");
    assert(oi_debug::diff(Empty{},Empty{}).changes.empty());
    assert(oi_debug::diff(std::vector<Empty>{},std::vector<Empty>{{}}).changes[0].after == "Empty{}");
    assert(oi_debug::diff(std::vector<std::vector<int>>{},
                          std::vector<std::vector<int>>{{}}).changes[0].after == "[]");
    const std::map<std::string, int> old_map{{"a",1}, {"b",2}};
    const std::map<std::string, int> new_map{{"a",3}, {"c\"\n",4}};
    assert(oi_debug::diff_to_string(oi_debug::diff(old_map,new_map)) ==
           "  [\"a\"]: 1 -> 3\n  [\"b\"]: 2 -> <missing>\n  [\"c\\\"\\n\"]: <missing> -> 4");
    const std::map<int,std::map<int,int>> no_maps, empty_map{{1,{}}};
    assert(oi_debug::diff(no_maps,empty_map).changes[0].after == "{}");
    const std::map<int, std::vector<fixture::Record>> no_records, records{{1,{{2,{3,4}}}}};
    assert(oi_debug::diff(no_records,records).changes.size() == 3);
    const char old_text[3] = {'a','b','c'}, new_text[3] = {'a','b','d'};
    assert(oi_debug::diff(old_text,new_text).changes[0].before == "\"abc\"");
    assert(oi_debug::diff(std::string("a\nb"),std::string("a\tc")).changes[0].after == "\"a\\tc\"");
    const char *null_text = nullptr, *literal = "nullptr";
    assert(oi_debug::diff(null_text,literal).changes.size() == 1);
    assert(oi_debug::diff(null_text,null_text).changes.empty());
    const char same_buffer1[] = "abc", same_buffer2[] = "abc";
    const char *same1 = same_buffer1, *same2 = same_buffer2;
    assert(oi_debug::diff(same1,same2).changes.empty());
    int x=1, y=1;
    int *px=&x, *py=&y;
    assert(oi_debug::diff(px,py).changes.size() == 1);
    assert(oi_debug::diff(px,px).changes.empty());
    assert(oi_debug::diff(nullptr,nullptr).changes.empty());
    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto inf = std::numeric_limits<double>::infinity();
    assert(oi_debug::diff(nan,nan).changes.empty());
    assert(oi_debug::diff(inf,inf).changes.empty());
    assert(oi_debug::diff(nan,inf).changes.size() == 1);
    assert(oi_debug::diff(0.0,-0.0).changes.empty());
    const Counted equal1(1), equal2(1), different(2);
    assert(oi_debug::diff(equal1,equal2).changes.empty());
    assert(Counted::conversions == 0 && Counted::comparisons == 1);
    assert(oi_debug::diff(equal1,different).changes.size() == 1);
    assert(Counted::conversions == 2 && Counted::comparisons == 2);
    const auto capped = oi_debug::diff(before,after,{.max_changes=1});
    assert(capped.changes.size() == 1 && !capped.complete());
    assert(capped.stop_reason == oi_debug::diff_stop_reason::max_changes);
    assert(capped.visited_nodes == 4);
    assert(oi_debug::diff_to_string(capped).find("remaining values not compared") != std::string::npos);
    const auto zero = oi_debug::diff(1,2,{.max_changes=0});
    assert(zero.changes.empty() && !zero.complete());
    assert(oi_debug::diff(1,1,{.max_changes=0}).complete());
    const auto no_nodes = oi_debug::diff(before,after,{.max_nodes=0});
    assert(no_nodes.visited_nodes == 0 && no_nodes.stop_reason == oi_debug::diff_stop_reason::max_nodes);
    const auto nodes = oi_debug::diff(before,after,{.max_nodes=3});
    assert(nodes.visited_nodes == 3 && nodes.stop_reason == oi_debug::diff_stop_reason::max_nodes);
    const auto depth = oi_debug::diff(before,after,{.max_depth=1});
    assert(depth.stop_reason == oi_debug::diff_stop_reason::max_depth);
    assert(oi_debug::diff(1,2,{.max_depth=0}).complete());
    const auto additions = oi_debug::diff(std::vector<std::vector<int>>{},after,{.max_nodes=2});
    assert(additions.visited_nodes == 2 && !additions.complete() && additions.changes.empty());
    const Tree tree1{1, {{2,{}}}}, tree2{1, {{3,{}}}};
    assert(oi_debug::diff(tree1,tree2).changes[0].path == ".children[0].value");
    std::ostringstream output;
    auto *previous = std::cout.rdbuf(output.rdbuf());
    int calls = 0;
    const auto get_before = [&]() -> const auto & { ++calls; return old_state; };
    const auto get_after = [&]() -> const auto & { ++calls; return new_state; };
    debug_diff(get_before(),get_after(), {.max_changes=1});
    std::cout.rdbuf(previous);
    assert(calls == 2 && output.str().starts_with("get_before() -> get_after():\n  .distance: 12 -> 9\n"));
    std::cout << "diff checks passed\n";
}
