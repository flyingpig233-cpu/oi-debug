#ifndef OI_DEBUG_STRING_CONCAT_HPP
#define OI_DEBUG_STRING_CONCAT_HPP

#include "core.hpp"
#include <ranges>
#include <string>
#include <string_view>

namespace oi_debug {
namespace detail {

template <std::size_t N>
constexpr std::string_view string_view_of(const char (&value)[N]) {
	std::size_t size = 0;
	while (size < N && value[size] != '\0') {
		++size;
	}
	return {value, size};
}

template <String T>
requires(!std::is_array_v<T>)
constexpr std::string_view string_view_of(const T &value) {
	if constexpr (std::is_pointer_v<T>) {
		if (value == nullptr) {
			return "nullptr";
		}
	}
	return std::string_view(value);
}

inline void append_quoted(std::string &buffer, std::string_view value,
						  std::string_view delimiter = double_quotes()) {
	buffer.append(delimiter);
	for (std::size_t i = 0; i < value.size(); ++i) {
		const auto c = static_cast<unsigned char>(value[i]);
		switch (c) {
		case '\0':
			buffer.append(i + 1 < value.size() && value[i + 1] >= '0' &&
						  value[i + 1] <= '7' ? "\\000" : "\\0");
			break;
		case '\a': buffer.append("\\a"); break;
		case '\b': buffer.append("\\b"); break;
		case '\t': buffer.append("\\t"); break;
		case '\n': buffer.append("\\n"); break;
		case '\v': buffer.append("\\v"); break;
		case '\f': buffer.append("\\f"); break;
		case '\r': buffer.append("\\r"); break;
		case '\\': buffer.append("\\\\"); break;
		default:
			if (c < 0x20 || c == 0x7f) {
				buffer.push_back('\\');
				buffer.push_back(static_cast<char>('0' + (c >> 6)));
				buffer.push_back(static_cast<char>('0' + ((c >> 3) & 7)));
				buffer.push_back(static_cast<char>('0' + (c & 7)));
			} else {
				if (delimiter.size() == 1 && value[i] == delimiter[0]) {
					buffer.push_back('\\');
				}
				buffer.push_back(value[i]);
			}
		}
	}
	buffer.append(delimiter);
}

} // namespace detail


/**
 * Summary: connect strings
 * Parameters:
 *     v: strings to concatenate
 * Example:
 *     connect("A", "B", "C") --> "ABC"
 */
template <typename... Args>
requires(String<Args> && ...)
[[nodiscard]] constexpr std::string connect(const Args &...v) {
	std::string buffer;
	std::size_t size{};
	((size += detail::string_view_of(v).size()), ...);
	buffer.reserve(size);
	((buffer += detail::string_view_of(v)), ...);
	return buffer;
}

/**
 *	Summary: Concatenate arrays with string elements
 *  Parameters:
 *     container: the container to concatenate
 *  Example:
 *     connect(std::vector<std::string>({"Hello", "World"})) --> "HelloWorld"
 */
template <typename Container>
requires(!String<Container> && std::ranges::input_range<const Container> &&
		 String<std::ranges::range_reference_t<const Container>>)
[[nodiscard]] std::string connect(const Container &container) {
	std::string buffer;
	if constexpr (std::ranges::forward_range<const Container>) {
		std::size_t size = 0;
		for (auto &&e : container) {
			size += detail::string_view_of(e).size();
		}
		buffer.reserve(size);
	}
	for (auto &&e : container) {
		buffer.append(detail::string_view_of(e));
	}
	return buffer;
}

/**
 *	Summary: Bracketing elements
 *  Parameters:
 *    v: target element
 *    c: Inserted characters
 *  Example:
 *     quoted("string", "'") --> 'string'
 */
[[nodiscard]] inline std::string
quoted(const std::string_view &v,
	   const std::string_view &c =
		   double_quotes()) {
	std::string buffer;
	buffer.reserve(v.size() + c.size() * 2);
	detail::append_quoted(buffer, v, c);
	return buffer;
}

} // namespace oi_debug

#endif // OI_DEBUG_STRING_CONCAT_HPP
