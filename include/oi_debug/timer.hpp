#ifndef _BGHELPER_TIMER_H__
#define _BGHELPER_TIMER_H__

#include <bits/chrono.h>
#include <chrono>
#include <cstdio>
#include <cxxabi.h>
#include <functional>
#include <iostream>
#include <source_location>
#include "to_string.hpp"

class Timer {
  private:
    std::source_location _location;
    std::chrono::system_clock::time_point _start_time;

  public:
    Timer(const std::source_location location = std::source_location::current())
        : _location(location), _start_time(std::chrono::system_clock::now()) {}
    ~Timer() {
        auto end_time = std::chrono::system_clock::now();
        auto spend_time = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - _start_time);
        printf("It took %ldus to call %s function in %s\n", spend_time,
               _location.function_name(), _location.file_name());
    }
};

template <class dur, class callable,  class... Args>
void _get_time(const char *postfix, const char *func_name, callable &&f, Args &&...args) {
    using return_type = typename std::result_of<callable(Args...)>::type;
    std::function<return_type(Args...)> func = f;

    auto start_time = std::chrono::system_clock::now();
    if constexpr (!std::is_same_v<return_type, void>) {
        auto return_val = func(std::forward<Args&&>(args)...);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto spend_time = std::chrono::duration_cast<dur>(
            end_time - start_time);
        int status;
        char *func_type =
            abi::__cxa_demangle(typeid(callable).name(), NULL, NULL, &status);
        if (status == 0) {
            printf("call %s<%s> spent %ld%s, return value: %s\n", func_name, func_type, spend_time, postfix, oi_debug::to_string(return_val).c_str());
        } else {
            fprintf(stderr, "Eh...something error");
        }
    } else {
        func(std::forward<Args&&>(args)...);
        auto end_time = std::chrono::system_clock::now();
        auto spend_time = std::chrono::duration_cast<dur>(
            end_time - start_time);
        int status;
        char *func_type =
            abi::__cxa_demangle(typeid(callable).name(), NULL, NULL, &status);
        if (status == 0) {
            printf("call %s<%s> spent %ld%s\n", func_name, func_type, spend_time, postfix);
            free(func_type);
        } else {
            fprintf(stderr, "Eh...something error");
        }
    }
}
#define get_time_us(fn, ...) _get_time<std::chrono::microseconds>("us", #fn, (fn), ##__VA_ARGS__)
#define get_time_ms(fn, ...) _get_time<std::chrono::milliseconds>("ms", #fn, (fn), ##__VA_ARGS__)
#define get_time_s(fn, ...) _get_time<std::chrono::seconds>("s", #fn, (fn), ##__VA_ARGS__)

#endif //_BGHELPER_TIMER_H__
