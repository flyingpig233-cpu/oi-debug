#ifndef _OIDEBUG_GET_TYPE_HPP__
#define _OIDEBUG_GET_TYPE_HPP__

#include "string_concat.hpp"
#include <array>
#include <cxxabi.h>
#include <forward_list>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <typename T>
consteval auto describe() noexcept {
  std::string_view name, prefix, suffix;
#ifdef __clang__
  name = __PRETTY_FUNCTION__;
  prefix = "auto describe() [T = ";
  suffix = "]";
#elif defined(__GNUC__)
  name = __PRETTY_FUNCTION__;
  prefix = "constexpr auto describe() [with T = ";
  suffix = "]";
#elif defined(_MSC_VER)
  name = __FUNCSIG__;
  prefix = "auto __cdecl describe<";
  suffix = ">(void)";
#endif
  name.remove_prefix(prefix.size());
  name.remove_suffix(suffix.size());
  return name;
}

template <typename T>
struct TypeParseTraits {
    consteval static std::string_view get_name()
    {
        return describe<T>();
    }
};
#define REGISTER_PARSE_MAP_TYPE(X)                                             \
    template <typename Key, typename T>                                        \
    struct TypeParseTraits<X<Key, T>> {                                        \
        constexpr static std::string get_name()                                \
        {                                                                      \
            return oi_debug::connect(#X "<", TypeParseTraits<Key>::get_name(), \
                oi_debug::comma(),                                       \
                TypeParseTraits<T>::get_name(), ">");                          \
        }                                                                      \
    }

#define REGISTER_PARSE_TYPE(X)                  \
    template <>                                 \
    struct TypeParseTraits<X> {                 \
        consteval static const std::string_view get_name() \
        {                                       \
            return #X;                          \
        }                                       \
    }

#define REGISTER_PARSE_CONTAINER_TYPE(X)                                           \
    template <typename T>                                                          \
    struct TypeParseTraits<X<T>> {                                                 \
        constexpr static std::string get_name()                                    \
        {                                                                          \
            return oi_debug::connect(#X "<", TypeParseTraits<T>::get_name(), ">"); \
        }                                                                          \
    }

REGISTER_PARSE_TYPE(bool);
REGISTER_PARSE_TYPE(char);
REGISTER_PARSE_TYPE(wchar_t);
REGISTER_PARSE_TYPE(int);
REGISTER_PARSE_TYPE(unsigned int);
REGISTER_PARSE_TYPE(long);
REGISTER_PARSE_TYPE(long long);
REGISTER_PARSE_TYPE(unsigned long long);
REGISTER_PARSE_TYPE(float);
REGISTER_PARSE_TYPE(double);
REGISTER_PARSE_CONTAINER_TYPE(std::vector);
REGISTER_PARSE_CONTAINER_TYPE(std::list);
REGISTER_PARSE_CONTAINER_TYPE(std::stack);
REGISTER_PARSE_CONTAINER_TYPE(std::queue);
REGISTER_PARSE_CONTAINER_TYPE(std::deque);
REGISTER_PARSE_CONTAINER_TYPE(std::unique_ptr);
REGISTER_PARSE_CONTAINER_TYPE(std::shared_ptr);
REGISTER_PARSE_MAP_TYPE(std::map);
REGISTER_PARSE_MAP_TYPE(std::pair);
REGISTER_PARSE_MAP_TYPE(std::multimap);
REGISTER_PARSE_MAP_TYPE(std::unordered_multimap);
REGISTER_PARSE_MAP_TYPE(std::set);
REGISTER_PARSE_MAP_TYPE(std::multiset);
REGISTER_PARSE_MAP_TYPE(std::unordered_multiset);

template <class Tuple, size_t... I>
constexpr std::string get_tuple_name(std::index_sequence<I...>)
{
    std::string buffer;
    buffer.append("std::tuple<");
    (..., buffer.append(I == 0 ? "" : oi_debug::comma()).append(TypeParseTraits<typename std::tuple_element_t<I, Tuple>>::get_name()));
    buffer.append(">");
    return buffer;
}

template <typename... Args>
struct TypeParseTraits<std::tuple<Args...>> {
    constexpr static std::string get_name()
    {
        return get_tuple_name<std::tuple<Args...>>(
            std::make_index_sequence<sizeof...(Args)>());
    }
};


#endif // _OIDEBUG_GET_TYPE_HPP__
