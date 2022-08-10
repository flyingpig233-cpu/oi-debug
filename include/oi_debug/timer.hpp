#ifndef _BGHELPER_TIMER_H__
#define _BGHELPER_TIMER_H__

#include <bits/chrono.h>
#include <chrono>
#include <cstdio>
#include <functional>
#include <iostream>
#include <source_location>

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

template <class callable, class... Args>
void _get_time_with_args(callable f, const char *func_name,
						 const char *args_name, Args &&...args) {
	std::function<typename std::result_of<callable(Args...)>::type()> task(
		std::bind(std::forward<callable>(f), std::forward<Args>(args)...));

	auto start_time = std::chrono::system_clock::now();
	task(std::forward<Args>(args)...);
	auto end_time = std::chrono::system_clock::now();
	auto spend_time = std::chrono::duration_cast<std::chrono::microseconds>(
		end_time - start_time);

	printf("call %s(%s) spent %ldus\n", func_name, args_name, spend_time);
}

template <class callable> void _get_time(callable f, const char *func_name) {
	auto start_time = std::chrono::system_clock::now();
	f();
	auto end_time = std::chrono::system_clock::now();
	auto spend_time = std::chrono::duration_cast<std::chrono::microseconds>(
		end_time - start_time);

	printf("call %s() spent %ldus\n", func_name, spend_time);
}

#define get_time_with_args(f, args...) _get_time_with_args(f, #f, #args, args)
#define get_time(f) _get_time(f, #f)

#endif //_BGHELPER_TIMER_H__
