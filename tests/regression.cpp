#include "oi_debug/debug.hpp"
#include "oi_debug/output.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <forward_list>
#include <iterator>
#include <limits>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct ConstValue {
    std::string to_string() const { return "custom"; }
};

struct MutableValue {
    std::string to_string() { return "mutable"; }
};

struct InvalidValue {
    int to_string() const { return 1; }
};

struct ThrowingValue {
    std::string to_string() const { throw std::runtime_error("serialize"); }
};

struct CountedValue {
    inline static int copies = 0;
    int value;
    explicit CountedValue(int value) : value(value) {}
    CountedValue(const CountedValue &other) : value(other.value) { ++copies; }
    CountedValue(CountedValue &&) = default;
    CountedValue &operator=(const CountedValue &) = default;
    CountedValue &operator=(CountedValue &&) = default;
    std::string to_string() const { return std::to_string(value); }
};

template <typename T>
struct InputRange {
    std::istream &input;
    auto begin() const { return std::istream_iterator<T>(input); }
    auto end() const { return std::istream_iterator<T>(); }
};

struct UnsizedMap {
    using key_type = int;
    using mapped_type = std::string;
    std::array<std::pair<int, std::string>, 2> entries{{{1, "a"}, {2, "b"}}};
    auto begin() const { return entries.begin(); }
    auto end() const { return entries.end(); }
};

class Capture {
    struct Buffer : std::stringbuf {
        int flushes = 0;
        int sync() override { ++flushes; return std::stringbuf::sync(); }
    } buffer;
    std::streambuf *previous = std::cout.rdbuf(&buffer);
  public:
    ~Capture() { std::cout.rdbuf(previous); }
    std::string str() const { return buffer.str(); }
    int flushes() const { return buffer.flushes; }
};

template <typename T>
concept HasHex = requires(T value) { oi_debug::hex(value); };

static_assert(oi_debug::convertible<ConstValue>);
static_assert(!oi_debug::convertible<MutableValue>);
static_assert(!oi_debug::convertible<InvalidValue>);
static_assert(!oi_debug::convertible<std::queue<std::unique_ptr<int>>>);
static_assert(!oi_debug::convertible<std::stack<std::unique_ptr<int>>>);
static_assert(HasHex<int> && !HasHex<bool>);
static_assert(std::ranges::input_range<const InputRange<int>>);
static_assert(!std::ranges::forward_range<const InputRange<int>>);

void check_text() {
    assert(oi_debug::to_string('\0') == std::string(1, '\0'));
    assert(oi_debug::quoted_if_string('\0') == "'\\0'");
    assert(oi_debug::quoted_if_string('\'') == "'\\\''");
    assert(oi_debug::quoted_if_string('\\') == "'\\\\'");
    assert(oi_debug::quoted_if_string('\n') == "'\\n'");
    const std::string special = "\a\b\t\n\v\f\r\\\"";
    assert(oi_debug::quoted_if_string(special) ==
           "\"\\a\\b\\t\\n\\v\\f\\r\\\\\\\"\"");
    assert(oi_debug::to_string(special) == special);
    assert(oi_debug::quoted_if_string(std::string("\0" "7", 2)) == "\"\\0007\"");
    assert(oi_debug::quoted_if_string(std::string("\x01" "f\x7f")) == "\"\\001f\\177\"");
    assert(oi_debug::quoted_if_string(std::string("中文")) == "\"中文\"");
    const char bounded[3] = {'a', 'b', 'c'};
    assert(oi_debug::to_string(bounded) == "abc");
    assert(oi_debug::quoted_if_string(bounded) == "\"abc\"");
    const char grid[2][3] = {{'a', 'b', 'c'}, {'d', 'e', 'f'}};
    assert(oi_debug::to_string(grid) == "[\"abc\", \"def\"]");
    assert(oi_debug::quoted("a\nb") == "\"a\\nb\"");
    assert(oi_debug::quoted("x", "<>") == "<>x<>");
}

void check_values() {
    const int array[2][2] = {{1, 2}, {3, 4}};
    assert(oi_debug::to_string(array) == "[[1, 2], [3, 4]]");
    assert(oi_debug::to_string(std::vector<std::vector<int>>{{}, {1, 2}}) == "[[], [1, 2]]");
    assert(oi_debug::to_string(std::forward_list<int>{1, 2}) == "[1, 2]");
    assert(oi_debug::to_string(std::vector<bool>{true, false}) == "[true, false]");
    assert(oi_debug::to_string(std::vector<int>{}) == "[]");
    assert(oi_debug::to_string(std::map<int, int>{}) == "{}");
    assert(oi_debug::to_string(UnsizedMap{}) == "{1: \"a\", 2: \"b\"}");
    assert(oi_debug::to_string(std::set<int>{2, 1}) == "[1, 2]");
    assert(oi_debug::to_string(std::multimap<int, int>{{1, 2}, {1, 3}}) == "{1: 2, 1: 3}");
    const auto unordered = oi_debug::to_string(std::unordered_set<int>{1, 2});
    assert(unordered == "[1, 2]" || unordered == "[2, 1]");
    const auto unordered_map = oi_debug::to_string(std::unordered_map<int, int>{{1, 2}, {3, 4}});
    assert(unordered_map == "{1: 2, 3: 4}" || unordered_map == "{3: 4, 1: 2}");
    assert(oi_debug::to_string(std::pair<char, std::string>{'x', "a\nb"}) == "('x', \"a\\nb\")");
    assert(oi_debug::to_string(std::tuple<>{}) == "()");
    const auto large = std::make_tuple(0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                                      10, 11, 12, 13, 14, 15, 16, 17, 18, 19);
    assert(oi_debug::to_string(large) == "(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19)");
    assert(oi_debug::to_string(std::optional<std::string>{"a\nb"}) == "optional(\"a\\nb\")");
    assert(oi_debug::to_string(std::optional<int>{}) == "std::nullopt");
    assert(oi_debug::to_string(std::variant<int, std::string>{"x"}) == "variant(\"x\")");
    assert(oi_debug::to_string(std::complex<double>{1, 2}) == "(1, 2)");
    assert(oi_debug::to_string(ConstValue{}) == "custom");
    enum class Number : unsigned char { value = 255 };
    assert(oi_debug::to_string(Number::value) == "255");
    enum class Flag : bool { yes = true };
    assert(oi_debug::to_string(Flag::yes) == "1");
    assert(oi_debug::to_string(std::numeric_limits<long long>::min()) == std::to_string(std::numeric_limits<long long>::min()));
    assert(oi_debug::to_string(std::numeric_limits<unsigned long long>::max()) == std::to_string(std::numeric_limits<unsigned long long>::max()));
    assert(oi_debug::hex(255) == "ff");
    assert(oi_debug::hex(static_cast<signed char>(-1)) == "ff");
    assert(oi_debug::to_string(3.1415926535897) == "3.1415926535897");
    assert(oi_debug::to_string(std::numeric_limits<double>::infinity()) == "inf");
    assert(oi_debug::to_string(std::numeric_limits<double>::quiet_NaN()) == "nan");
    for (const auto value : {1.0L / 3, std::numeric_limits<long double>::min(),
                             std::numeric_limits<long double>::max()}) {
        const auto text = oi_debug::to_string(value);
        assert(std::strtold(text.c_str(), nullptr) == value);
    }
}

void check_ranges_and_adapters() {
    std::istringstream numbers("1 2 3");
    assert(oi_debug::to_string(InputRange<int>{numbers}) == "[1, 2, 3]");
    std::istringstream empty;
    assert(oi_debug::to_string(InputRange<int>{empty}) == "[]");
    std::vector<int> values{1, 2};
    const auto filtered = values | std::views::filter([](int x) { return x > 1; });
    static_assert(!oi_debug::convertible<decltype(filtered)>);
    std::queue<CountedValue> queue;
    queue.emplace(1);
    queue.emplace(2);
    CountedValue::copies = 0;
    assert(oi_debug::to_string(queue) == "[1, 2]");
    assert(CountedValue::copies == 2);
    assert(queue.size() == 2 && queue.front().value == 1);
    std::stack<CountedValue> stack;
    stack.emplace(1);
    stack.emplace(2);
    CountedValue::copies = 0;
    assert(oi_debug::to_string(stack) == "[2, 1]");
    assert(CountedValue::copies == 2);
    assert(stack.size() == 2 && stack.top().value == 2);
    std::priority_queue<int> priority;
    priority.push(1);
    priority.push(3);
    priority.push(2);
    assert(oi_debug::to_string(priority) == "[3, 2, 1]");
    assert(priority.size() == 3 && priority.top() == 3);
    assert(oi_debug::to_string(std::queue<int>{}) == "[]");
    assert(oi_debug::to_string(std::stack<int>{}) == "[]");
}

void check_pointers() {
    assert(oi_debug::to_string(nullptr) == "nullptr");
    assert(oi_debug::to_string(static_cast<int *>(nullptr)) == "nullptr");
    assert(oi_debug::to_string(static_cast<const char *>(nullptr)) == "nullptr");
    assert(oi_debug::to_string(static_cast<void *>(nullptr)) == "nullptr");
    assert(oi_debug::quoted_if_string(static_cast<const char *>(nullptr)) == "nullptr");
    auto owned = std::make_unique<std::string>("a\nb");
    assert(oi_debug::to_string(owned) == "\"a\\nb\"");
    auto shared = std::make_shared<int>(4);
    std::weak_ptr<int> weak = shared;
    assert(oi_debug::to_string(shared) == "4");
    assert(oi_debug::to_string(weak) == "4");
    shared.reset();
    assert(oi_debug::to_string(weak) == "expired");
    assert(oi_debug::to_string(shared) == "nullptr");
    auto array = std::make_unique<int[]>(2);
    assert(oi_debug::to_string(array) == oi_debug::to_string(array.get()));
    auto shared_array = std::shared_ptr<int[]>(new int[2]);
    assert(oi_debug::to_string(shared_array) == oi_debug::to_string(shared_array.get()));
    std::shared_ptr<void> erased = shared_array;
    assert(oi_debug::to_string(erased) == oi_debug::to_string(erased.get()));
    auto chars = std::make_unique<char[]>(2);
    chars[0] = 'x';
    assert(oi_debug::to_string(chars) ==
           oi_debug::to_string(static_cast<void *>(chars.get())));
    array.reset();
    assert(oi_debug::to_string(array) == "nullptr");
}

void check_connect() {
    const std::vector<const char *> pointers{"a", "b"};
    assert(oi_debug::connect(pointers) == "ab");
    const std::string strings[] = {"a", "b"};
    assert(oi_debug::connect(strings) == "ab");
    const char arrays[2][2] = {{'a', 'b'}, {'c', 'd'}};
    assert(oi_debug::connect(arrays) == "abcd");
    assert(oi_debug::connect(arrays[0], arrays[1]) == "abcd");
    assert(oi_debug::connect(std::vector<std::string_view>{"a", "b"}) == "ab");
    std::istringstream words("a bb c");
    assert(oi_debug::connect(InputRange<std::string>{words}) == "abbc");
    assert(oi_debug::connect() == "");
    assert(oi_debug::concat("n=", 1, std::vector<int>{2, 3}) == "n=1[2, 3]");
}

void check_exceptions_and_output() {
    const auto expect_throw = [](const auto &value) {
        bool caught = false;
        try { (void)oi_debug::to_string(value); }
        catch (const std::runtime_error &error) {
            caught = std::string(error.what()) == "serialize";
        }
        assert(caught);
    };
    expect_throw(std::pair<ThrowingValue, int>{});
    expect_throw(std::vector<ThrowingValue>(1));
    expect_throw(std::map<int, ThrowingValue>{{1, {}}});
    expect_throw(std::optional<ThrowingValue>{std::in_place});
    expect_throw(std::variant<ThrowingValue, int>{});
    Capture capture;
    const char nul = '\0';
    debug(nul);
    debug_with_type(std::vector<int>{1, 2});
    oi_debug::print("a", 1);
    oi_debug::println("b", 2);
    oi_debug::println();
    std::string expected("nul = ");
    expected.push_back('\0');
    expected.append("\nstd::vector<int> std::vector<int>{1, 2} = [1, 2]\na1b2\n\n");
    assert(capture.str() == expected);
    assert(capture.flushes() == 0);
    const auto before = capture.str();
    try { debug(ThrowingValue{}); }
    catch (const std::runtime_error &) {}
    assert(capture.str() == before);
}

int main() {
    check_text();
    check_values();
    check_ranges_and_adapters();
    check_pointers();
    check_connect();
    check_exceptions_and_output();
    std::cout << "serialization checks passed\n";
}
