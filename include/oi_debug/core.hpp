#ifndef _BGHELPER_CORE_H__
#define _BGHELPER_CORE_H__

#include <array>
#include <concepts>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

namespace bg_helper {

#ifdef UNICODE
using char_type = wchar_t;
#else
using char_type = char;
#endif

template <typename T>
concept Character = std::is_same_v<T, char> || std::is_same_v<T, wchar_t>;

template <typename T>
concept Float = std::is_floating_point_v<T>;

template <typename T>
concept Boolean = std::is_same_v<bool, std::decay_t<T>>;

template <typename T>
concept String = std::convertible_to<T, std::string_view> ||
	std::convertible_to<T, std::wstring_view>;

template <typename T>
concept has_to_string = requires(T& a) {
	a.to_string();
};

template <typename T>
concept Map = std::ranges::range<T> && requires(T a) {
	typename std::decay_t<T>::key_type;
	typename std::decay_t<T>::mapped_type;
};

template <Character char_type_t = bg_helper::char_type>
constexpr const std::basic_string<char_type_t> colon() {
	if constexpr (std::is_same_v<char_type_t, wchar_t>)
		return L": ";
	else
		return ": ";
}
template <Character char_type_t = bg_helper::char_type>
constexpr const std::basic_string<char_type_t> comma() {
	if constexpr (std::is_same_v<char_type_t, wchar_t>)
		return L", ";
	else
		return ", ";
}

template <Character char_type_t = bg_helper::char_type>
constexpr const std::basic_string<char_type_t> curly_braces(bool i) {
	if constexpr (std::is_same_v<char_type_t, wchar_t>)
		return i ? L"}" : L"{";
	else
		return i ? "}" : "{";
}

template <Character char_type_t = bg_helper::char_type>
constexpr const std::basic_string<char_type_t> square_brakets(bool i) {
	if constexpr (std::is_same_v<char_type_t, wchar_t>)
		return i ? L"]" : L"[";
	else
		return i ? "]" : "[";
}

template <Character char_type_t = bg_helper::char_type>
constexpr const std::basic_string<char_type_t> brakets(bool i) {
	if constexpr (std::is_same_v<char_type_t, wchar_t>)
		return i ? L")" : L"(";
	else
		return i ? ")" : "(";
}

template <Character char_type_t = bg_helper::char_type>
constexpr const std::basic_string<char_type_t> true_false(bool i) {
	if constexpr (std::is_same_v<char_type_t, wchar_t>)
		return i ? L"true" : L"false";
	else
		return i ? "true" : "false";
}

template <Character char_type_t = bg_helper::char_type>
constexpr const std::basic_string<char_type_t> quotes() {
	if constexpr (std::is_same_v<char_type_t, wchar_t>)
		return L"'";
	else
		return "'";
}
template <Character char_type_t = bg_helper::char_type>
constexpr const std::basic_string<char_type_t> double_quotes() {
	if constexpr (std::is_same_v<char_type_t, wchar_t>)
		return L"\"";
	else
		return "\"";
}
} // namespace bg_helper

#endif //_BGHELPER_CORE_H__
