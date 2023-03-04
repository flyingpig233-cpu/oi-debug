#ifndef _OIDEBUG_STRING_CONCAT_H__
#define _OIDEBUG_STRING_CONCAT_H__

#include "core.hpp"
#include <cassert>
#include <string>
#include <string_view>

namespace oi_debug {


/**
 * Summary: connect strings
 * Parameters:
 *     v: strings to concatenate
 * Example:
 *     connect("A", "B", "C") --> "ABC"
 */
template <typename... Args>
[[nodiscard]] constexpr std::string connect(const Args &...v) {
	std::string buffer;
	std::size_t size{};
	((size += std::string_view(v).size()), ...);
	buffer.reserve(size);
	((buffer += std::string_view(v)), ...);
	return buffer;
}

/**
 *	Summary: Concatenate arrays with string elements
 *  Parameters:
 *     container: the container to concatenate
 *  Example:
 *     connect(std::vector<std::string>({"Hello", "World"})) --> "HelloWorld"
 */
template <
		  std::ranges::range Container>
requires String<typename Container::value_type>
[[nodiscard]] std::string
connect(const Container &container) {
	size_t size = 0;
	for (auto &&e : container) {
		size += std::ranges::size(e);
	}

	std::string buffer;
	buffer.reserve(size);

	for (auto &&e : container) {
		buffer += e;
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
	return connect(c, v, c);
}

} // namespace oi_debug

#endif //_OIDEBUG_STRING_CONCAT_H__
