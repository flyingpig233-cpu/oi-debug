#ifndef OI_DEBUG_TO_STRING_HPP
#define OI_DEBUG_TO_STRING_HPP

#include "core.hpp"
#include "string_concat.hpp"
#include <algorithm>
#include <charconv>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <system_error>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace oi_debug {

template <std::integral T> [[nodiscard]] std::string hex(T value) noexcept {
  using Unsigned = std::make_unsigned_t<T>;
  Unsigned v = static_cast<Unsigned>(value);
  char buffer[sizeof(Unsigned) * 2]{};
  std::size_t len = 0;
  do {
    buffer[len++] = digit(static_cast<std::size_t>(v & 0xF));
    v >>= 4;
  } while (v != 0);
  std::reverse(buffer, buffer + len);
  return std::string(buffer, len);
}

template <typename T>
[[nodiscard]] constexpr inline std::string quoted_if_string(const T &value);

template <typename T>
[[nodiscard]] constexpr inline std::string to_string(const T &value);

template <std::size_t N>
[[nodiscard]] inline std::string _to_string(const char (&value)[N]) noexcept {
  std::size_t len = 0;
  while (len < N && value[len] != '\0') {
    ++len;
  }
  return std::string(value, len);
}

template <std::convertible_to<std::string_view> T>
requires(!std::is_null_pointer_v<std::decay_t<T>> && !std::is_array_v<T>)
[[nodiscard]] inline std::string _to_string(const T &value) noexcept {
  if constexpr (std::is_pointer_v<T>) {
    if (value == nullptr) {
      return "nullptr";
    }
  }
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

template <typename T>
requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
[[nodiscard]] inline std::string _to_string(const T value) noexcept {
  char buffer[64]{};
  const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
  if (result.ec != std::errc{}) {
    return std::to_string(value);
  }
  return std::string(buffer, result.ptr);
}

template <typename T>
concept to_chars_supported = requires(char *first, char *last, T value) {
  std::to_chars(first, last, value);
};

template <typename T>
requires(std::is_same_v<T, long double>)
[[nodiscard]] inline std::string _to_string(const T value) noexcept {
  char buffer[128]{};
  if constexpr (to_chars_supported<T>) {
    const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    if (result.ec == std::errc{}) {
      return std::string(buffer, result.ptr);
    }
  }
  const auto written = std::snprintf(
      buffer, sizeof(buffer), "%.*Lg",
      std::numeric_limits<long double>::max_digits10, value);
  if (written > 0) {
    return std::string(buffer, static_cast<std::size_t>(written));
  }
  return {};
}

[[nodiscard]] inline std::string _to_string(std::nullptr_t) noexcept {
  return "nullptr";
}

[[nodiscard]] inline std::string _to_string(std::nullopt_t) noexcept {
  return "std::nullopt";
}

[[nodiscard]] inline std::string _to_string(std::monostate) noexcept {
  return "monostate";
}

// for pointer
[[nodiscard]] inline std::string
_to_string(const void *pointer) noexcept {
  return connect("0x", hex(reinterpret_cast<std::uintptr_t>(pointer)));
}

// for smart pointers
template <typename T, typename Deleter>
requires(!std::is_array_v<T> && !std::is_void_v<T>)
[[nodiscard]] inline std::string
_to_string(const std::unique_ptr<T, Deleter> &pointer) {
  return pointer ? quoted_if_string(*pointer) : std::string("nullptr");
}

template <typename T, typename Deleter>
requires(std::is_array_v<T> || std::is_void_v<T>)
[[nodiscard]] inline std::string
_to_string(const std::unique_ptr<T, Deleter> &pointer) {
  return pointer
             ? connect("0x",
                       hex(reinterpret_cast<std::uintptr_t>(pointer.get())))
             : std::string("nullptr");
}

template <typename T>
requires(!std::is_array_v<T> && !std::is_void_v<T>)
[[nodiscard]] inline std::string _to_string(const std::shared_ptr<T> &pointer) {
  return pointer ? quoted_if_string(*pointer) : std::string("nullptr");
}

template <typename T>
requires(std::is_array_v<T> || std::is_void_v<T>)
[[nodiscard]] inline std::string _to_string(const std::shared_ptr<T> &pointer) {
  return pointer
             ? connect("0x",
                       hex(reinterpret_cast<std::uintptr_t>(pointer.get())))
             : std::string("nullptr");
}

template <typename T>
[[nodiscard]] inline std::string _to_string(const std::weak_ptr<T> &pointer) {
  const auto locked = pointer.lock();
  return locked ? to_string(locked) : std::string("expired");
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

// for optional

template <typename T>
[[nodiscard]] inline std::string
_to_string(const std::optional<T> &value) {
  return value ? connect("optional(", quoted_if_string(*value), ")")
               : std::string("std::nullopt");
}

// for variant

template <typename... Ts>
[[nodiscard]] inline std::string
_to_string(const std::variant<Ts...> &value) {
  if (value.valueless_by_exception()) {
    return "valueless variant";
  }
  return std::visit(
      [](const auto &alternative) {
        return connect("variant(", quoted_if_string(alternative), ")");
      },
      value);
}

// for complex

template <typename T>
[[nodiscard]] inline std::string
_to_string(const std::complex<T> &value) {
  return connect(brackets(0), to_string(value.real()), comma(),
                 to_string(value.imag()), brackets(1));
}

// for bool

template <Boolean T>
[[nodiscard]] constexpr std::string _to_string(const T value) noexcept {
  return std::string(true_false(value));
}

template <typename T>
requires(std::is_enum_v<T>)
[[nodiscard]] inline std::string _to_string(const T value) noexcept {
  return std::to_string(static_cast<std::underlying_type_t<T>>(value));
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
[[nodiscard]] OI_DEBUG_FORCE_INLINE std::string
_to_string(const std::tuple<Args...> &value) {
  constexpr size_t size = sizeof...(Args);
  static_assert(size < 20, "too large tuple!!!");
  std::string buff;
  buff.append(brackets(0));
  if constexpr (size > 0) {
    tuple_to_string<decltype(value), size>::to_string(value, buff);
  }
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

template <typename T>
[[nodiscard]] constexpr inline std::string to_string(const T &value) {
  constexpr bool ok = convertible<T>;
  if constexpr (ok) {
    return _to_string(value);
  } else {
    static_assert(ok, "some types could not convert");
    return {};
  }
}

template <typename T>
[[nodiscard]] std::string constexpr inline quoted_if_string(const T &value) {
  if constexpr (String<T>) {
    if constexpr (std::is_pointer_v<T>) {
      if (value == nullptr) {
        return "nullptr";
      }
    }
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

#endif // OI_DEBUG_TO_STRING_HPP
