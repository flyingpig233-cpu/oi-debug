#ifndef _OIDEBUG_GET_TYPE_HPP__
#define _OIDEBUG_GET_TYPE_HPP__

#include "string_concat.hpp"
#include <array>
#include <cxxabi.h>
#include <list>
#include <queue>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

template <typename T> struct TypeParseTraits {
    constexpr static std::string get_name() {
        int status;
        std::string tname = typeid(T).name();
        char *demangled_name =
            abi::__cxa_demangle(tname.c_str(), NULL, NULL, &status);
        if (status == 0) {
            tname = demangled_name;
            std::free(demangled_name);
        }
        return tname;
    }
};

#define REGISTER_PARSE_TYPE(X)                                                 \
    template <> struct TypeParseTraits<X> {                                    \
        constexpr static std::string get_name() { return #X; }                 \
    }

#define REGISTER_PARSE_CONTAINER_TYPE(X)                                       \
    template <typename T> struct TypeParseTraits<X<T>> {                       \
        constexpr static std::string get_name() {                              \
            return oi_debug::connect(#X "<", TypeParseTraits<T>::get_name(),   \
                                     ">");                                     \
        }                                                                      \
    }

REGISTER_PARSE_CONTAINER_TYPE(std::vector);
REGISTER_PARSE_CONTAINER_TYPE(std::list);
REGISTER_PARSE_CONTAINER_TYPE(std::stack);
REGISTER_PARSE_CONTAINER_TYPE(std::queue);
REGISTER_PARSE_CONTAINER_TYPE(std::deque);

template <class Tuple, size_t... I>
std::string get_tuple_name(std::index_sequence<I...>) {
    std::string buffer;
    buffer.append("std::tuple<");
    (..., buffer.append(I == 0 ? "" : oi_debug::comma<char>())
              .append(TypeParseTraits<
                      typename std::tuple_element_t<I, Tuple>>::get_name()));
    buffer.append(">");
    return buffer;
}

template <typename... Args> struct TypeParseTraits<std::tuple<Args...>> {
    constexpr static std::string get_name() {
        return get_tuple_name<std::tuple<Args...>>(
            std::make_index_sequence<sizeof...(Args)>());
    }
};

REGISTER_PARSE_TYPE(std::string);
REGISTER_PARSE_TYPE(std::wstring);

#endif // _OIDEBUG_GET_TYPE_HPP__
