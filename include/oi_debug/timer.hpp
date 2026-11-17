#ifndef OI_DEBUG_TIMER_HPP
#define OI_DEBUG_TIMER_HPP

#include "to_string.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cxxabi.h>
#include <functional>
#include <memory>
#include <source_location>
#include <type_traits>
#include <typeinfo>
#include <utility>

class Timer {
  private:
    std::source_location _location;
    std::chrono::steady_clock::time_point _start_time;

  public:
    Timer(const std::source_location location = std::source_location::current())
        : _location(location), _start_time(std::chrono::steady_clock::now()) {}
    ~Timer() {
        const auto spend_time =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - _start_time)
                .count();
        printf("It took %lldus to call %s function in %s\n",
               static_cast<long long>(spend_time), _location.function_name(),
               _location.file_name());
    }
};

namespace oi_debug {

template <class dur, class callable, class... Args>
void get_time_impl(const char *postfix, const char *func_name, callable &&f,
                   Args &&...args) {
    using return_type = std::invoke_result_t<callable, Args...>;
    int status = 0;
    const char *mangled_name = typeid(std::decay_t<callable>).name();
    const std::unique_ptr<char, decltype(&std::free)> func_type(
        abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status), &std::free);
    const char *type_name = status == 0 && func_type
                                ? func_type.get() : mangled_name;
    const auto start_time = std::chrono::steady_clock::now();

    if constexpr (std::is_void_v<return_type>) {
        std::invoke(std::forward<callable>(f), std::forward<Args>(args)...);
        const auto spend_time = std::chrono::duration_cast<dur>(
                                    std::chrono::steady_clock::now() -
                                    start_time)
                                    .count();
        printf("call %s<%s> spent %lld%s\n", func_name, type_name,
               static_cast<long long>(spend_time), postfix);
    } else {
        decltype(auto) return_val = std::invoke(
            std::forward<callable>(f), std::forward<Args>(args)...);
        const auto spend_time = std::chrono::duration_cast<dur>(
                                    std::chrono::steady_clock::now() -
                                    start_time)
                                    .count();
        printf("call %s<%s> spent %lld%s, return value: %s\n", func_name,
               type_name, static_cast<long long>(spend_time), postfix,
               oi_debug::to_string(return_val).c_str());
    }
}

} // namespace oi_debug

#define get_time_us(fn, ...)                                                   \
    oi_debug::get_time_impl<std::chrono::microseconds>(                        \
        "us", #fn, (fn) __VA_OPT__(, ) __VA_ARGS__)
#define get_time_ms(fn, ...)                                                   \
    oi_debug::get_time_impl<std::chrono::milliseconds>(                        \
        "ms", #fn, (fn) __VA_OPT__(, ) __VA_ARGS__)
#define get_time_s(fn, ...)                                                    \
    oi_debug::get_time_impl<std::chrono::seconds>(                             \
        "s", #fn, (fn) __VA_OPT__(, ) __VA_ARGS__)

#endif // OI_DEBUG_TIMER_HPP
