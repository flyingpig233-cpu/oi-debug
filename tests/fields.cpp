#include "oi_debug/debug.hpp"
#include "registered.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

struct Empty {};
OI_DEBUG_FIELDS(Empty);

struct Edge {
    int to;
    long long weight;
};
OI_DEBUG_FIELDS(Edge, to, weight);

struct State {
    std::string label;
    int data[2];
    std::vector<Edge> edges;
};
OI_DEBUG_FIELDS(State, label, data, edges);

struct Custom {
    int value;
    std::string to_string() const { return "custom"; }
};
OI_DEBUG_FIELDS(Custom, value);

struct Range {
    int values[2];
    const int *begin() const { return values; }
    const int *end() const { return values + 2; }
};
OI_DEBUG_FIELDS(Range, values);

struct NoCopy {
    int value;
    explicit NoCopy(int value) : value(value) {}
    NoCopy(const NoCopy &) = delete;
};
OI_DEBUG_FIELDS(NoCopy, value);

template <typename T>
struct Box { T value; };

template <typename T>
struct oi_debug::field_traits<Box<T>> {
    inline static constexpr std::string_view name = "Box";
    static constexpr auto fields() {
        return std::make_tuple(oi_debug::make_field("value", &Box<T>::value));
    }
};

struct Sixteen {
    int a=1, b=2, c=3, d=4, e=5, f=6, g=7, h=8;
    int i=9, j=10, k=11, l=12, m=13, n=14, o=15, p=16;
};
OI_DEBUG_FIELDS(Sixteen, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p);

constexpr bool constexpr_visit() {
    int sum = 0;
    oi_debug::visit_fields(Edge{2, 3}, [&](auto, const auto &v) { sum += v; });
    return sum == 5;
}
static_assert(constexpr_visit());
static_assert(oi_debug::registered_fields<Edge>);
static_assert(!oi_debug::registered_fields<int>);
static_assert(oi_debug::convertible<Edge>);

int main() {
    assert(oi_debug::to_string(Empty{}) == "Empty{}");
    assert(oi_debug::to_string(Edge{3, 10}) == "Edge{to: 3, weight: 10}");
    assert(oi_debug::to_string(State{"a\nb", {1, 2}, {{3, 4}}}) ==
           "State{label: \"a\\nb\", data: [1, 2], edges: [Edge{to: 3, weight: 4}]}");
    assert(oi_debug::to_string(Custom{1}) == "custom");
    assert(oi_debug::to_string(Range{{1, 2}}) == "Range{values: [1, 2]}");
    const NoCopy value(42);
    oi_debug::visit_fields(value, [&](auto name, const auto &field) {
        assert(name == "value" && &field == &value.value);
    });
    assert(oi_debug::to_string(value) == "NoCopy{value: 42}");
    assert(oi_debug::to_string(Box<Edge>{{2, 3}}) == "Box{value: Edge{to: 2, weight: 3}}");
    assert(oi_debug::to_string(fixture::Record{1, {2, 3}}) ==
           "fixture::Record{value: 1, samples: [2, 3]}");
    assert(std::tuple_size_v<decltype(oi_debug::field_traits<Sixteen>::fields())> == 16);
    assert(oi_debug::to_string(Sixteen{}).ends_with("o: 15, p: 16}"));
    std::ostringstream output;
    auto *previous = std::cout.rdbuf(output.rdbuf());
    const Edge edge{3, 10};
    debug(edge);
    debug_with_type(edge);
    std::cout.rdbuf(previous);
    assert(output.str() == "edge = Edge{to: 3, weight: 10}\nEdge edge = Edge{to: 3, weight: 10}\n");
    std::cout << "fields checks passed\n";
}
