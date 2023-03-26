#ifndef _OIDEBUG_TO_STRING_H__
#define _OIDEBUG_TO_STRING_H__

#include "core.hpp"
#include "milo/dtoa_milo.h"
#include "string_concat.hpp"
#include <string>
#include <tuple>
#include <type_traits>

namespace oi_debug {

template <std::integral T> [[nodiscard]] std::string hex(T value) noexcept {
  const size_t h_len = sizeof(T) << 1;
  if (value > 0xF) {
    T temp{};
    char buffer[h_len]{};
    std::uint8_t i{h_len - 1};

    while (value >= 16) {
      temp = value & 0xF;
      value >>= 4;
      buffer[i--] = digit(temp);
    }

    buffer[i] = digit(value);
    return std::string(buffer, std::end(buffer));
  } else {
    auto result{digit(value)};
    return std::string(result, 1);
  }
}

template <typename T>
[[nodiscard]] constexpr inline std::string quoted_if_string(const T &value);

template <typename T>
[[nodiscard]] constexpr inline std::string to_string(const T &value);

template <std::convertible_to<std::string_view> T>
[[nodiscard]] constexpr inline std::string _to_string(const T &value) noexcept {
  return std::string(value);
}

template <has_to_string T>
[[nodiscard]] constexpr inline std::string _to_string(T &t) noexcept {
  return t.to_string();
}

template <std::integral T, typename = std::enable_if_t<
                               !Boolean<T> && !Character<T>, std::string>>
[[nodiscard]] constexpr inline std::string _to_string(const T &value) noexcept {
  return std::to_string(value);
}
[[nodiscard]] inline std::string
_to_string(const char value) noexcept {
  char c[2] {value, 0};
  return std::string(c);
}

[[nodiscard]] inline std::string
_to_string(const float value) noexcept {
  return std::to_string(value);
}
// for pointer
[[nodiscard]] inline std::string
_to_string(const void *pointer) noexcept {
  return connect("0x", hex(reinterpret_cast<std::uintptr_t>(pointer)));
}

// for c style array
template <typename T, size_t N>
[[nodiscard]] std::enable_if_t<String<T>, std::string>
_to_string(const T (&arr)[N]) {
  std::string buff;
  buff.append(square_brackets(0));
  for (size_t i = 0; i < N - 1; ++i) {
    buff.append(quoted_if_string(arr[i]));
    buff.append(comma());
  }
  buff.append(quoted_if_string(arr[N - 1]));
  buff.append(square_brackets(1));
  return buff;
}

// for pair

template <typename T, typename E>
[[nodiscard]] constexpr std::string
_to_string(const std::pair<T, E> &value,
           const std::string_view &split_c = comma()) noexcept {
  return oi_debug::connect(brackets(0), quoted_if_string(value.first), split_c,
                           quoted_if_string(value.second), brackets(1));
}

// for float double

[[nodiscard]] inline std::string _to_string(const double &value) noexcept {
  char buff[50];
  dtoa_milo(value, buff);
  return std::string(buff);
}

// for bool

template <Boolean T>
[[nodiscard]] constexpr std::string _to_string(const T value) noexcept {
  return std::string(true_false(value));
}

// for map

template <Map T> [[nodiscard]] std::string _to_string(const T &value) noexcept {
  std::string buffer;
  auto size = value.size();
  if (size > 10) {
    buffer.reserve(size * 3);
  }
  buffer.append(curly_braces(0));
  for (auto &&e : value) {
    buffer.append(quoted_if_string(e.first));
    buffer.append(colon());
    buffer.append(quoted_if_string(e.second));
    buffer.append(comma());
  }
  if (size > 0)
    buffer.erase(buffer.size() - comma().size());
  buffer.append(curly_braces(1));
  return buffer;
}

// for FIFO Container (like std::queue)

template <FIFO_Container T>
[[nodiscard]] std::string _to_string(T value) noexcept {
  std::string buffer;
  buffer.append(square_brackets(0));
  while (!value.empty()) {
    buffer.append(quoted_if_string(value.front()));
    value.pop();
    if (!value.empty())
      buffer.append(comma());
  }
  buffer.append(square_brackets(1));
  return buffer;
}

// for container (requires begin() and end())

template <std::ranges::range T>
[[nodiscard]] std::enable_if_t<!String<T> && !Map<T>, std::string>
_to_string(const T &value) noexcept {
  std::string buffer;
  buffer.append(square_brackets(0));
  for (auto &&e : value) {
    buffer.append(quoted_if_string(e));
    buffer.append(comma());
  }
  if (!std::ranges::empty(value))
    buffer.erase(buffer.size() - comma().size());
  buffer.append(square_brackets(1));
  return buffer;
}

// for LIFO Container (like stack)
template <LIFO_Container T>
[[nodiscard]] std::string _to_string(T value) noexcept {
  std::vector<typename T::value_type> v;
  while (!value.empty()) {
    v.push_back(value.top());
    value.pop();
  }
  return to_string(v);
}

// for tuple

template <typename T, size_t N> struct tuple_to_string {
  static void to_string(const T &tup, std::string &buff);
};

template <typename T> struct tuple_to_string<T, 1> {
  static void to_string(const T &tup, std::string &buff);
};

template <typename... Args>
[[nodiscard]] __FORCE_INLINE__ inline std::string
_to_string(const std::tuple<Args...> &value) {
  constexpr size_t size = sizeof...(Args);
  static_assert(size < 20, "too large tuple!!!");
  std::string buff;
  buff.append(brackets(0));
  tuple_to_string<decltype(value), size>::to_string(value, buff);
  buff.append(brackets(1));
  return buff;
}

template <typename T, size_t N>
void tuple_to_string<T, N>::to_string(const T &tup, std::string &buff) {
  tuple_to_string<T, N - 1>::to_string(tup, buff);
  buff.append(comma());
  buff.append(oi_debug::quoted_if_string(std::get<N - 1>(tup)));
}

template <typename T>
void tuple_to_string<T, 1>::to_string(const T &tup, std::string &buff) {
  buff.append(oi_debug::quoted_if_string(std::get<0>(tup)));
}

template <typename T>
concept convertible = requires(T &&a) { oi_debug::_to_string(a); };

[[nodiscard]] constexpr std::string &&_to_string(std::string &&value) noexcept {
  return std::forward<std::string &&>(value);
}

template <typename T>
[[nodiscard]] constexpr inline std::string to_string(const T &value) {
  constexpr bool ok = convertible<T>;
  if constexpr (ok) {
    return _to_string(value);
  } else {
    // error
    static_assert(ok, "some types could not convert");
    return nullptr;
  }
}

template <typename T>
[[nodiscard]] std::string constexpr inline quoted_if_string(const T &value) {
  if constexpr (String<T>) {
    return oi_debug::quoted(oi_debug::to_string(value));
  }
  if constexpr (std::is_same_v<char, T>) {
    char arr[2]{value, 0};
    return oi_debug::quoted(std::string(arr), quotes());
  }
  return oi_debug::to_string(value);
}

template <typename... Args>
[[nodiscard]] constexpr std::string concat(const Args &...v) {
  return connect(to_string(v)...);
}

} // namespace oi_debug

#endif //_OIDEBUG_TO_STRING_H__
