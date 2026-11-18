#ifndef OI_DEBUG_FIELDS_HPP
#define OI_DEBUG_FIELDS_HPP

#include <concepts>
#include <functional>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace oi_debug {

template <typename T>
struct field_traits {};

template <typename Member>
struct field_descriptor {
    std::string_view name;
    Member pointer;
};

template <typename Member>
requires std::is_member_object_pointer_v<Member>
constexpr auto make_field(std::string_view name, Member pointer) {
    return field_descriptor<Member>{name, pointer};
}

template <typename T>
concept registered_fields = requires {
    { field_traits<std::remove_cvref_t<T>>::name } ->
        std::convertible_to<std::string_view>;
    field_traits<std::remove_cvref_t<T>>::fields();
};

template <registered_fields T, typename Visitor>
constexpr void visit_fields(const T &value, Visitor &&visitor) {
    std::apply([&](const auto &...field) {
        (std::invoke(visitor, field.name, value.*(field.pointer)), ...);
    }, field_traits<std::remove_cvref_t<T>>::fields());
}

} // namespace oi_debug

// The convenience macro supports up to sixteen fields. Specialize field_traits
// directly for longer lists or template types containing preprocessor commas.
#define OI_DEBUG_DETAIL_FIELD(T, F) ::oi_debug::make_field(#F, &T::F)
#define OI_DEBUG_DETAIL_FIELDS_1(T, F) OI_DEBUG_DETAIL_FIELD(T, F)
#define OI_DEBUG_DETAIL_FIELDS_2(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_1(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_3(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_2(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_4(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_3(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_5(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_4(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_6(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_5(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_7(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_6(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_8(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_7(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_9(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_8(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_10(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_9(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_11(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_10(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_12(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_11(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_13(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_12(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_14(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_13(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_15(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_14(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_16(T, F, ...) OI_DEBUG_DETAIL_FIELD(T, F), OI_DEBUG_DETAIL_FIELDS_15(T, __VA_ARGS__)
#define OI_DEBUG_DETAIL_FIELDS_SELECT(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, N, ...) N
#define OI_DEBUG_DETAIL_FIELDS(T, ...) \
    OI_DEBUG_DETAIL_FIELDS_SELECT(__VA_ARGS__, OI_DEBUG_DETAIL_FIELDS_16, \
        OI_DEBUG_DETAIL_FIELDS_15, OI_DEBUG_DETAIL_FIELDS_14, OI_DEBUG_DETAIL_FIELDS_13, \
        OI_DEBUG_DETAIL_FIELDS_12, OI_DEBUG_DETAIL_FIELDS_11, OI_DEBUG_DETAIL_FIELDS_10, \
        OI_DEBUG_DETAIL_FIELDS_9, OI_DEBUG_DETAIL_FIELDS_8, OI_DEBUG_DETAIL_FIELDS_7, \
        OI_DEBUG_DETAIL_FIELDS_6, OI_DEBUG_DETAIL_FIELDS_5, OI_DEBUG_DETAIL_FIELDS_4, \
        OI_DEBUG_DETAIL_FIELDS_3, OI_DEBUG_DETAIL_FIELDS_2, OI_DEBUG_DETAIL_FIELDS_1)(T, __VA_ARGS__)

#define OI_DEBUG_FIELDS(T, ...) \
    template <> struct oi_debug::field_traits<T> { \
        inline static constexpr std::string_view name = #T; \
        static constexpr auto fields() { \
            return std::make_tuple(__VA_OPT__(OI_DEBUG_DETAIL_FIELDS(T, __VA_ARGS__))); \
        } \
    }

#endif // OI_DEBUG_FIELDS_HPP
