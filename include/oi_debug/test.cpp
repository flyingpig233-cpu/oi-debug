#include "debug.hpp"
#include "timer.hpp"
#include <bits/stdc++.h>

int main() {
  auto func = []() {
    std::ios::sync_with_stdio(false);
    for (int i = 0; i < 100000; ++i) {
      std::cout << "HELLO WORLD!\n";
    }
  };
  auto func2 = []() {
    for (int i = 0; i < 100000; ++i) {
      oi_debug::print("HELLO WORLD!\n");
    }
  };
  auto func3 = []() {
    for (int i = 0; i < 100000; ++i) {
      printf("HELLO WORLD!\n");
    }
  };
  get_time_us(func);
}
