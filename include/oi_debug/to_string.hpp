#ifndef OI_DEBUG_TO_STRING_HPP
#define OI_DEBUG_TO_STRING_HPP

#include "core.hpp"
#include "string_concat.hpp"
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

namespace oi_debug {
namespace detail {

template <typename T>
void append_to(std::string &buffer, const T &value);

template <typename T>
void append_element(std::string &buffer, const T &value);

template <std::integral T>
requires(!Boolean<T>)
void append_integer(std::string &buffer, T value, int base = 10) {
  char digits[std::numeric_limits<T>::digits10 + 3];
  const auto result = std::to_chars(digits, digits + sizeof(digits), value, base);
  buffer.append(digits, result.ptr);
}

template <std::size_t N>
void append_value(std::string &buffer, const char (&value)[N]) {
  buffer.append(string_view_of(value));
}

template <String T>
requires(!std::is_array_v<T>)
void append_value(std::string &buffer, const T &value) {
  buffer.append(string_view_of(value));
}

template <has_to_string T>
requires(!String<T>)
void append_value(std::string &buffer, const T &value) {
  buffer.append(value.to_string());
}

template <std::integral T>
requires(!Boolean<T> && !Character<T>)
void append_value(std::string &buffer, T value) {
  append_integer(buffer, value);
}

inline void append_value(std::string &buffer, char value) {
  buffer.push_back(value);
}

template <typename T>
concept to_chars_supported = requires(char *first, char *last, T value) {
  std::to_chars(first, last, value);
};

template <std::floating_point T>
void append_value(std::string &buffer, T value) {
  char digits[std::numeric_limits<T>::max_digits10 + 32];
  if constexpr (to_chars_supported<T>) {
    const auto result = std::to_chars(digits, digits + sizeof(digits), value);
    if (result.ec == std::errc{}) {
      buffer.append(digits, result.ptr);
      return;
    }
  }
  const auto written = std::snprintf(
      digits, sizeof(digits), "%.*Lg", std::numeric_limits<T>::max_digits10,
      static_cast<long double>(value));
  if (written > 0) {
    const auto size = static_cast<std::size_t>(written);
    if (size < sizeof(digits)) {
      buffer.append(digits, size);
    } else {
      const auto offset = buffer.size();
      buffer.resize(offset + size);
      std::snprintf(buffer.data() + offset, size + 1, "%.*Lg",
                    std::numeric_limits<T>::max_digits10,
                    static_cast<long double>(value));
    }
  }
}

inline void append_value(std::string &buffer, std::nullptr_t) {
  buffer.append("nullptr");
}

inline void append_value(std::string &buffer, std::nullopt_t) {
  buffer.append("std::nullopt");
}

inline void append_value(std::string &buffer, std::monostate) {
  buffer.append("monostate");
}

inline void append_value(std::string &buffer, const volatile void *pointer) {
  if (pointer == nullptr) {
    buffer.append("nullptr");
  } else {
    buffer.append("0x");
    append_integer(buffer, reinterpret_cast<std::uintptr_t>(pointer), 16);
  }
}

template <typename T, typename Deleter>
void append_value(std::string &buffer,
                  const std::unique_ptr<T, Deleter> &pointer) {
  if (!pointer) {
    buffer.append("nullptr");
  } else if constexpr (std::is_array_v<T> || std::is_void_v<T>) {
    append_value(buffer, static_cast<const volatile void *>(pointer.get()));
  } else {
    append_element(buffer, *pointer);
  }
}

template <typename T>
void append_value(std::string &buffer, const std::shared_ptr<T> &pointer) {
  if (!pointer) {
    buffer.append("nullptr");
  } else if constexpr (std::is_array_v<T> || std::is_void_v<T>) {
    append_value(buffer, static_cast<const volatile void *>(pointer.get()));
  } else {
    append_element(buffer, *pointer);
  }
}

template <typename T>
void append_value(std::string &buffer, const std::weak_ptr<T> &pointer) {
  const auto locked = pointer.lock();
  if (locked) {
    append_value(buffer, locked);
  } else {
    buffer.append("expired");
  }
}

template <typename T, typename U>
void append_value(std::string &buffer, const std::pair<T, U> &value) {
  buffer.append(brackets(0));
  append_element(buffer, value.first);
  buffer.append(comma());
  append_element(buffer, value.second);
  buffer.append(brackets(1));
}

template <typename T>
void append_value(std::string &buffer, const std::optional<T> &value) {
  if (value) {
    buffer.append("optional(");
    append_element(buffer, *value);
    buffer.push_back(')');
  } else {
    buffer.append("std::nullopt");
  }
}

template <typename... Ts>
void append_value(std::string &buffer, const std::variant<Ts...> &value) {
  if (value.valueless_by_exception()) {
    buffer.append("valueless variant");
  } else {
    buffer.append("variant(");
    std::visit([&](const auto &alternative) {
      append_element(buffer, alternative);
    }, value);
    buffer.push_back(')');
  }
}

template <typename T>
void append_value(std::string &buffer, const std::complex<T> &value) {
  buffer.append(brackets(0));
  append_to(buffer, value.real());
  buffer.append(comma());
  append_to(buffer, value.imag());
  buffer.append(brackets(1));
}

template <Boolean T>
void append_value(std::string &buffer, T value) {
  buffer.append(true_false(value));
}

template <typename T>
requires(std::is_enum_v<T>)
void append_value(std::string &buffer, T value) {
  append_to(buffer, +static_cast<std::underlying_type_t<T>>(value));
}

template <Map T>
requires(!String<T> && !has_to_string<T>)
void append_value(std::string &buffer, const T &value) {
  buffer.append(curly_braces(0));
  bool first = true;
  for (auto &&entry : value) {
    if (!first) {
      buffer.append(comma());
    }
    first = false;
    append_element(buffer, entry.first);
    buffer.append(colon());
    append_element(buffer, entry.second);
  }
  buffer.append(curly_braces(1));
}

template <FIFO_Container T>
requires(!String<T> && !has_to_string<T>)
void append_value(std::string &buffer, T value) {
  buffer.append(square_brackets(0));
  bool first = true;
  while (!value.empty()) {
    if (!first) {
      buffer.append(comma());
    }
    first = false;
    append_element(buffer, value.front());
    value.pop();
  }
  buffer.append(square_brackets(1));
}

template <typename T>
requires(std::ranges::input_range<const T> && !String<T> &&
         !Map<T> && !has_to_string<T>)
void append_value(std::string &buffer, const T &value) {
  buffer.append(square_brackets(0));
  bool first = true;
  for (auto &&element : value) {
    if (!first) {
      buffer.append(comma());
    }
    first = false;
    append_element(buffer, element);
  }
  buffer.append(square_brackets(1));
}

template <LIFO_Container T>
requires(!String<T> && !has_to_string<T>)
void append_value(std::string &buffer, T value) {
  buffer.append(square_brackets(0));
  bool first = true;
  while (!value.empty()) {
    if (!first) {
      buffer.append(comma());
    }
    first = false;
    append_element(buffer, value.top());
    value.pop();
  }
  buffer.append(square_brackets(1));
}

template <typename... Ts>
void append_value(std::string &buffer, const std::tuple<Ts...> &value) {
  buffer.append(brackets(0));
  if constexpr (sizeof...(Ts) > 0) {
    std::apply([&](const auto &...elements) {
      bool first = true;
      const auto append = [&](const auto &element) {
        if (!first) {
          buffer.append(comma());
        }
        first = false;
        append_element(buffer, element);
      };
      (append(elements), ...);
    }, value);
  }
  buffer.append(brackets(1));
}

} // namespace detail

template <typename T>
concept convertible = requires(std::string &buffer, const T &value) {
  { detail::append_value(buffer, value) } -> std::same_as<void>;
};

namespace detail {

template <typename T>
void append_to(std::string &buffer, const T &value) {
  if constexpr (convertible<T>) {
    append_value(buffer, value);
  } else {
    static_assert(convertible<T>, "some types could not convert");
  }
}

template <typename T>
void append_element(std::string &buffer, const T &value) {
  if constexpr (String<T>) {
    if constexpr (std::is_pointer_v<T>) {
      if (value == nullptr) {
        buffer.append("nullptr");
        return;
      }
    }
    append_quoted(buffer, string_view_of(value));
  } else if constexpr (Character<T>) {
    append_quoted(buffer, std::string_view(&value, 1), quotes());
  } else {
    append_to(buffer, value);
  }
}

} // namespace detail

template <std::integral T>
requires(!Boolean<T>)
[[nodiscard]] inline std::string hex(T value) {
  std::string buffer;
  detail::append_integer(buffer,
                         static_cast<std::make_unsigned_t<T>>(value), 16);
  return buffer;
}

template <typename T>
[[nodiscard]] inline std::string to_string(const T &value) {
  std::string buffer;
  detail::append_to(buffer, value);
  return buffer;
}

template <typename T>
[[nodiscard]] inline std::string quoted_if_string(const T &value) {
  std::string buffer;
  detail::append_element(buffer, value);
  return buffer;
}

template <typename... Args>
[[nodiscard]] inline std::string concat(const Args &...values) {
  std::string buffer;
  (detail::append_to(buffer, values), ...);
  return buffer;
}

} // namespace oi_debug

#endif // OI_DEBUG_TO_STRING_HPP
