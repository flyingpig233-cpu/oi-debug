
#ifndef _BGHELPER_STRING_CONCAT_H__
#define _BGHELPER_STRING_CONCAT_H__

#include "core.hpp"
#include <cassert>
#include <string>
#include <string_view>

namespace bg_helper {

/**
 * Summary: connect strings
 * Paramters:
 *     v: strings to concatenate
 * Example:
 *     connect("A", "B", "C") --> "ABC"
 */
template <Character char_type_t = bg_helper::char_type, typename... Args>
[[nodiscard]] std::basic_string<char_type_t> connect(const Args &...v) {
	std::basic_string<char_type_t> buffer;
	std::size_t size{};
	((size += std::basic_string_view<char_type_t>(v).size()), ...);
	buffer.reserve(size);
	((buffer += std::basic_string_view<char_type_t>(v)), ...);
	return buffer;
}

template <Character char_type_t = bg_helper::char_type,
		  std::ranges::range Container>
requires String<typename Container::value_type>
[[nodiscard]] std::basic_string<char_type_t>
connect(const Container &container) {
	size_t size = 0;
	for (auto &&e : container) {
		size += e.size();
	}

	std::basic_string<char_type_t> buffer;
	buffer.reserve(size);

	for (auto &&e : container) {
		buffer += e;
	}
	return buffer;
}

template <Character char_type_t = bg_helper::char_type>
[[nodiscard]] inline std::basic_string<char_type_t>
quoted(const std::basic_string_view<char_type_t> &v,
	   const std::basic_string_view<char_type_t> &c =
		   double_quotes<char_type_t>()) {
	return connect<char_type_t>(c, v, c);
}

/**
 * Summary: connect elements to string
 * Paramters:
 *     v: elements to concatenate
 * Example:
 *     concat("PI = ", 3.1415) --> "PI = 3.1415"
 */

} // namespace bg_helper

#endif //_BGHELPER_STRING_CONCAT_H__
