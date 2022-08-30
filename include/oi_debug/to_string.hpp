#ifndef _OIDEBUG_TO_STRING_H__
#define _OIDEBUG_TO_STRING_H__

#include "core.hpp"
#include "milo/dtoa_milo.h"
#include "string_concat.hpp"
#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace oi_debug {

template <Character char_type = oi_debug::char_type, std::integral T>
[[nodiscard]] inline std::basic_string<char_type> hex(T value) noexcept {
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
    return std::basic_string<char_type>(buffer, std::end(buffer));
  } else {
    auto result{digit(value)};
    return std::basic_string<char_type>(result, 1);
  }
}

template <Character char_type_t, typename T>
[[nodiscard]] constexpr inline std::basic_string<char_type_t> quoted_if_string(const T &value);

template <Character char_type_t, typename T>
[[nodiscard]] constexpr inline std::basic_string<char_type_t>
to_string(const T &value);

template <Character char_type_t = oi_debug::char_type,
          std::convertible_to<std::basic_string_view<char_type_t>> T>
[[nodiscard]] constexpr inline std::basic_string<char_type_t>
_to_string(const T &value) noexcept {
  return std::basic_string<char_type_t>(value);
}

template <Character char_type_t = oi_debug::char_type, has_to_string T>
[[nodiscard]] constexpr inline std::basic_string<char_type_t>
_to_string(T &t) noexcept {
  return t.to_string();
}

template <Character char_type_t = oi_debug::char_type, std::integral T,
          typename = std::enable_if_t<!Boolean<T> && !Character<T>,
                                      std::basic_string<char_type_t>>>
[[nodiscard]] constexpr inline std::basic_string<char_type_t>
_to_string(const T &value) noexcept {
  if constexpr (std::is_same_v<wchar_t, char_type_t>) {
    return std::to_wstring(value);
  } else {
    return std::to_string(value);
  }
}
template <Character char_type_t = oi_debug::char_type>
[[nodiscard]] constexpr inline std::basic_string<char_type_t>
_to_string(const char_type_t &value) noexcept {
  char_type_t c[2]{value, 0};
  return std::basic_string<char_type_t>(c);
}

template <Character char_type_t = oi_debug::char_type>
[[nodiscard]] constexpr inline std::basic_string<char_type_t>
_to_string(const float &value) noexcept {
  if constexpr (std::is_same_v<wchar_t, char_type_t>) {
    return std::to_wstring(value);
  } else {
    return std::to_string(value);
  }
}
// for pointer
template <Character char_type_t = oi_debug::char_type>
[[nodiscard]] constexpr inline std::basic_string<char_type_t>
_to_string(const void *pointer) noexcept {
  return connect<char_type_t>("0x", hex<char_type_t>(reinterpret_cast<std::uintptr_t>(pointer)));
}

// for c style array
template <Character char_type_t = oi_debug::char_type, typename T, size_t N>
[[nodiscard]] std::enable_if_t<String<T>, std::basic_string<char_type_t>> _to_string(const T (&arr)[N]) {
  std::basic_string<char_type_t> buff;
  buff.append(square_brackets<char_type_t>(0));
  for (size_t i = 0; i < N - 1; ++i) {
    buff.append(quoted_if_string<char_type_t>(arr[i]));
    buff.append(comma<char_type_t>());
  }
  buff.append(quoted_if_string<char_type_t>(arr[N - 1]));
  buff.append(square_brackets<char_type_t>(1));
  return buff;
}

// for pair

template <Character char_type_t = oi_debug::char_type, typename T, typename E>
[[nodiscard]] constexpr std::basic_string<char_type_t>
_to_string(const std::pair<T, E> &value,
           const std::basic_string_view<char_type_t> &split_c =
               comma<char_type_t>()) noexcept {
  return oi_debug::connect<char_type_t>(
      brackets(0), quoted_if_string<char_type_t>(value.first), split_c,
      quoted_if_string<char_type_t>(value.second), brackets(1));
}

// for float double

template <Character char_type_t = oi_debug::char_type>
[[nodiscard]] constexpr std::basic_string<char_type_t>
_to_string(const double &value) noexcept {
  if constexpr (std::is_same_v<wchar_t, char_type_t>) {
    return std::to_wstring(value);
  } else {
    char buff[50];
    dtoa_milo(value, buff);
    return std::string(buff);
  }
}

// for bool

template <Character char_type_t = oi_debug::char_type, Boolean T>
[[nodiscard]] constexpr std::basic_string<char_type_t>
_to_string(const T value) noexcept {
  return true_false<char_type_t>(value);
}

// for map

template <Character char_type_t = oi_debug::char_type, Map T>
[[nodiscard]] std::basic_string<char_type_t>
_to_string(const T &value) noexcept {
  std::basic_string<char_type_t> buffer;
  auto size = value.size();
  if (size > 10) {
    buffer.reserve(size * 3);
  }
  buffer.append(curly_braces<char_type_t>(0));
  for (auto &&e : value) {
    buffer.append(quoted_if_string<char_type_t>(e.first));
    buffer.append(colon<char_type_t>());
    buffer.append(quoted_if_string<char_type_t>(e.second));
    buffer.append(comma<char_type_t>());
  }
  if (size > 0)
    buffer.erase(buffer.size() - comma<char_type_t>().size());
  buffer.append(curly_braces<char_type_t>(1));
  return buffer;
}

// for FIFO Container (like std::queue)

template <Character char_type_t = oi_debug::char_type, FIFO_Container T>
[[nodiscard]] std::basic_string<char_type_t> _to_string(T value) noexcept {
  std::basic_string<char_type_t> buffer;
  buffer.append(square_brackets<char_type_t>(0));
  while (!value.empty()) {
    buffer.append(quoted_if_string<char_type_t>(value.front()));
    value.pop();
    if (!value.empty())
      buffer.append(comma<char_type_t>());
  }
  buffer.append(square_brackets<char_type_t>(1));
  return buffer;
}

// for container (need begin() and end())

template <Character char_type_t = oi_debug::char_type, Iterable T>
[[nodiscard]] std::enable_if_t<!String<T> && !Map<T>,
                               std::basic_string<char_type_t>>
_to_string(const T &value) noexcept {
  std::basic_string<char_type_t> buffer;
  auto size = value.size();
  if (size > 10) {
    buffer.reserve(size * 3);
  }
  buffer.append(square_brackets<char_type_t>(0));
  for (auto &&e : value) {
    buffer.append(quoted_if_string<char_type_t>(e));
    buffer.append(comma<char_type_t>());
  }
  if (size > 0)
    buffer.erase(buffer.size() - comma<char_type_t>().size());
  buffer.append(square_brackets<char_type_t>(1));
  return buffer;
}

// for LIFO Container (like stack)
template <Character char_type_t = oi_debug::char_type, LIFO_Container T>
[[nodiscard]] std::basic_string<char_type_t> _to_string(T value) noexcept {
  std::vector<typename T::value_type> v;
  while (!value.empty()) {
    v.push_back(value.top());
    value.pop();
  }
  return to_string<char_type_t>(v);
}

// for tuple

template <Character char_type_t, typename T, size_t N> struct tuple_to_string {
  static void to_string(const T &tup, std::basic_string<char_type_t> &buff);
};

template <Character char_type_t, typename T>
struct tuple_to_string<char_type_t, T, 1> {
  static void to_string(const T &tup, std::basic_string<char_type_t> &buff);
};

template <Character char_type_t = oi_debug::char_type, typename... Args>
[[nodiscard]] __FORCE_INLINE__ inline std::basic_string<char_type_t>
_to_string(const std::tuple<Args...> &value) {
  constexpr size_t size = sizeof...(Args);
  static_assert(size < 10, "too large tuple!!!");
  std::basic_string<char_type_t> buff;
  buff.append(brackets<char_type_t>(0));
  tuple_to_string<char_type_t, decltype(value), size>::to_string(value, buff);
  buff.append(brackets<char_type_t>(1));
  return buff;
}

template <Character char_type_t, typename T, size_t N>
void tuple_to_string<char_type_t, T, N>::to_string(
    const T &tup, std::basic_string<char_type_t> &buff) {
  tuple_to_string<char_type_t, T, N - 1>::to_string(tup, buff);
  buff.append(comma<char_type_t>());
  buff.append(oi_debug::quoted_if_string<char_type_t>(std::get<N - 1>(tup)));
}

template <Character char_type_t, typename T>
void tuple_to_string<char_type_t, T, 1>::to_string(
    const T &tup, std::basic_string<char_type_t> &buff) {
  buff.append(oi_debug::quoted_if_string<char_type_t>(std::get<0>(tup)));
}

template <typename char_type_t, typename T>
concept convertible = requires(T &a) {
  oi_debug::_to_string<char_type_t>(a);
};

template <Character char_type_t = oi_debug::char_type>
[[nodiscard]] constexpr std::basic_string<char_type> &&
_to_string(std::basic_string<char_type_t> &&value) noexcept {
  return std::forward<std::basic_string<char_type_t> &&>(value);
}

template <Character char_type_t = oi_debug::char_type, typename T>
[[nodiscard]] constexpr inline std::basic_string<char_type_t>
to_string(const T &value) {
  constexpr bool ok = convertible<char_type_t, T>;
  if constexpr (ok) {
    return _to_string<char_type_t>(value);
  } else {
    // error
    static_assert(ok, "some types could not convert");
    return nullptr;
  }
}

template <Character char_type_t = oi_debug::char_type, typename T>
[[nodiscard]] std::basic_string<char_type_t>  constexpr inline quoted_if_string(
    const T &value) {
  if constexpr (String<T>) {
    return oi_debug::quoted<char_type_t>(
        oi_debug::to_string<char_type_t>(value));
  }
  if constexpr (std::is_same_v<char_type_t, T>) {
    char_type_t arr[2]{value, 0};
    return oi_debug::quoted<char_type_t>(std::basic_string<char_type_t>(arr),
                                         quotes<char_type_t>());
  }
  return oi_debug::to_string<char_type_t>(value);
}

template <Character char_type_t = oi_debug::char_type, typename... Args>
[[nodiscard]] constexpr std::basic_string<char_type_t>
concat(const Args &...v) {
  return connect<char_type_t>(to_string<char_type_t>(v)...);
}

} // namespace oi_debug

#endif //_OIDEBUG_TO_STRING_H__
