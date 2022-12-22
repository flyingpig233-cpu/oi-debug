#ifndef _OIDEBUG_STRING_CONCAT_H__
#define _OIDEBUG_STRING_CONCAT_H__

#include "core.hpp"
#include <cassert>
#include <string>
#include <string_view>

namespace oi_debug {

template <std::string_view const&... Strs>
struct join {
    // Join all strings into a single std::array of chars
    static consteval auto impl() noexcept
    {
        constexpr std::size_t len = (Strs.size() + ... + 0);
        std::array<char, len + 1> arr {};
        auto append = [i = 0, &arr](auto const& s) mutable {
            for (auto c : s)
                arr[i++] = c;
        };
        (append(Strs), ...);
        arr[len] = 0;
        return arr;
    }
    // Give the joined string static storage
    static constexpr auto arr = impl();
    // View as a std::string_view
    static constexpr std::string_view value { arr.data(), arr.size() - 1 };
};
// Helper to get the value out
template <std::string_view const&... Strs>
static constexpr auto join_v = join<Strs...>::value;

/**
 * Summary: connect strings
 * Parameters:
 *     v: strings to concatenate
 * Example:
 *     connect("A", "B", "C") --> "ABC"
 */
template <Character char_type_t = oi_debug::char_type, typename... Args>
[[nodiscard]] constexpr std::basic_string<char_type_t> connect(const Args &...v) {
	std::basic_string<char_type_t> buffer;
	std::size_t size{};
	((size += std::basic_string_view<char_type_t>(v).size()), ...);
	buffer.reserve(size);
	((buffer += std::basic_string_view<char_type_t>(v)), ...);
	return buffer;
}

/**
 *	Summary: Concatenate arrays with string elements
 *  Parameters:
 *     container: the container to concatenate
 *  Example:
 *     connect(std::vector<std::string>({"Hello", "World"})) --> "HelloWorld"
 */
template <Character char_type_t = oi_debug::char_type,
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

/**
 *	Summary: Bracketing elements
 *  Parameters:
 *    v: target element
 *    c: Inserted characters
 *  Example:
 *     quoted("string", "'") --> 'string'
 */
template <Character char_type_t = oi_debug::char_type>
[[nodiscard]] inline std::basic_string<char_type_t>
quoted(const std::basic_string_view<char_type_t> &v,
	   const std::basic_string_view<char_type_t> &c =
		   double_quotes<char_type_t>()) {
	return connect<char_type_t>(c, v, c);
}

} // namespace oi_debug

#endif //_OIDEBUG_STRING_CONCAT_H__
