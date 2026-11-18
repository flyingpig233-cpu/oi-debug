#ifndef OI_DEBUG_DIFF_HPP
#define OI_DEBUG_DIFF_HPP

#include "to_string.hpp"
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace oi_debug {

struct diff_options {
    std::size_t max_changes = 100;
    std::size_t max_nodes = 100000;
    std::size_t max_depth = 64;
};

enum class diff_kind { added, removed, modified };
enum class diff_stop_reason { none, max_changes, max_nodes, max_depth };

struct diff_entry {
    std::string path;
    diff_kind kind;
    std::optional<std::string> before;
    std::optional<std::string> after;
};

struct diff_report {
    std::vector<diff_entry> changes;
    std::size_t visited_nodes = 0;
    diff_stop_reason stop_reason = diff_stop_reason::none;

    [[nodiscard]] bool complete() const noexcept {
        return stop_reason == diff_stop_reason::none;
    }
};

namespace detail {

template <typename T>
inline constexpr bool diff_sequence = false;
template <typename T, std::size_t N>
inline constexpr bool diff_sequence<T[N]> = true;
template <typename T, std::size_t N>
inline constexpr bool diff_sequence<std::array<T, N>> = true;
template <typename T, typename Allocator>
inline constexpr bool diff_sequence<std::vector<T, Allocator>> = true;
template <typename T, std::size_t Extent>
inline constexpr bool diff_sequence<std::span<T, Extent>> = true;

template <typename T>
inline constexpr bool diff_map = false;
template <typename K, typename V, typename Compare, typename Allocator>
inline constexpr bool diff_map<std::map<K, V, Compare, Allocator>> = true;

template <typename T>
concept DiffLeaf = String<T> ||
    ((std::is_arithmetic_v<T> || std::is_enum_v<T> || std::is_pointer_v<T> ||
      std::is_null_pointer_v<T> || has_to_string<T>) && convertible<T> &&
     requires(const T &a, const T &b) {
         { a == b } -> std::convertible_to<bool>;
     });

template <DiffLeaf T>
bool diff_leaf_equal(const T &a, const T &b) {
    if constexpr (String<T>) {
        if constexpr (std::is_pointer_v<T>) {
            if (a == nullptr || b == nullptr) {
                return a == b;
            }
        }
        return string_view_of(a) == string_view_of(b);
    } else if constexpr (std::is_floating_point_v<T>) {
        return a == b || (std::isnan(a) && std::isnan(b));
    } else {
        return a == b;
    }
}

struct diff_context {
    diff_options options;
    diff_report report;

    bool stopped() const noexcept { return !report.complete(); }

    template <typename T>
    void record(const T *before, const T *after, const std::string &path) {
        if (report.changes.size() >= options.max_changes) {
            report.stop_reason = diff_stop_reason::max_changes;
            return;
        }
        diff_entry entry{path.empty() ? "$" : path,
            !before ? diff_kind::added : !after ? diff_kind::removed
                                                 : diff_kind::modified,
            std::nullopt, std::nullopt};
        if (before) {
            entry.before = quoted_if_string(*before);
        }
        if (after) {
            entry.after = quoted_if_string(*after);
        }
        report.changes.push_back(std::move(entry));
        // Stop conservatively at the cap; no extra traversal to count omissions.
        if (report.changes.size() == options.max_changes) {
            report.stop_reason = diff_stop_reason::max_changes;
        }
    }
};

template <typename T>
void diff_walk(const T *before, const T *after, std::string &path,
               std::size_t depth, diff_context &context);

template <typename T>
void diff_walk(const T *before, const T *after, std::string &path,
               std::size_t depth, diff_context &context) {
    if (context.stopped()) {
        return;
    }
    if (depth > context.options.max_depth) {
        context.report.stop_reason = diff_stop_reason::max_depth;
        return;
    }
    if (context.report.visited_nodes >= context.options.max_nodes) {
        context.report.stop_reason = diff_stop_reason::max_nodes;
        return;
    }
    ++context.report.visited_nodes;
    if constexpr (String<T>) {
        if (!before || !after || !diff_leaf_equal(*before, *after)) {
            context.record(before, after, path);
        }
    } else if constexpr (registered_fields<T>) {
        const auto fields = field_traits<std::remove_cvref_t<T>>::fields();
        if constexpr (std::tuple_size_v<decltype(fields)> == 0) {
            if (!before || !after) {
                context.record(before, after, path);
            }
        } else {
            std::apply([&](const auto &...field) {
                const auto visit = [&](const auto &descriptor) {
                    if (context.stopped()) {
                        return;
                    }
                    const auto length = path.size();
                    path.push_back('.');
                    path.append(descriptor.name);
                    diff_walk(before ? std::addressof((*before).*(descriptor.pointer)) : nullptr,
                              after ? std::addressof((*after).*(descriptor.pointer)) : nullptr,
                              path, depth + 1, context);
                    path.resize(length);
                };
                (visit(field), ...);
            }, fields);
        }
    } else if constexpr (diff_sequence<std::remove_cv_t<T>>) {
        // Missing composite values are expanded to leaves so node/change caps
        // also bound traversal of newly added or removed nested structures.
        const T &available = before ? *before : *after;
        auto old_it = std::ranges::begin(before ? *before : available);
        const auto old_end = std::ranges::end(before ? *before : available);
        auto new_it = std::ranges::begin(after ? *after : available);
        const auto new_end = std::ranges::end(after ? *after : available);
        if ((!before || std::ranges::empty(*before)) &&
            (!after || std::ranges::empty(*after)) && (!before || !after)) {
            context.record(before, after, path);
        }
        for (std::size_t i = 0; !context.stopped(); ++i) {
            const bool has_before = before && old_it != old_end;
            const bool has_after = after && new_it != new_end;
            if (!has_before && !has_after) {
                break;
            }
            const auto length = path.size();
            path.append("[").append(std::to_string(i)).append("]");
            if (has_before && has_after) {
                auto &&old_value = *old_it;
                auto &&new_value = *new_it;
                diff_walk(std::addressof(old_value), std::addressof(new_value),
                          path, depth + 1, context);
            } else if (has_before) {
                auto &&value = *old_it;
                diff_walk<std::remove_cvref_t<decltype(value)>>(
                    std::addressof(value), nullptr, path, depth + 1, context);
            } else {
                auto &&value = *new_it;
                diff_walk<std::remove_cvref_t<decltype(value)>>(
                    nullptr, std::addressof(value), path, depth + 1, context);
            }
            path.resize(length);
            if (has_before) {
                ++old_it;
            }
            if (has_after) {
                ++new_it;
            }
        }
    } else if constexpr (diff_map<std::remove_cv_t<T>>) {
        if ((!before || before->empty()) && (!after || after->empty()) &&
            (!before || !after)) {
            context.record(before, after, path);
        }
        const auto visit = [&](const auto &key, const auto *old_value,
                               const auto *new_value) {
            const auto length = path.size();
            path.append("[").append(quoted_if_string(key)).append("]");
            diff_walk(old_value, new_value, path, depth + 1, context);
            path.resize(length);
        };
        if (before) {
            for (const auto &[key, value] : *before) {
                if (context.stopped()) {
                    break;
                }
                const typename T::mapped_type *other = nullptr;
                if (after) {
                    const auto found = after->find(key);
                    if (found != after->end()) {
                        other = std::addressof(found->second);
                    }
                }
                visit(key, std::addressof(value), other);
            }
        }
        if (after) {
            for (const auto &[key, value] : *after) {
                if (context.stopped()) {
                    break;
                }
                if (!before || before->find(key) == before->end()) {
                    const typename T::mapped_type *missing = nullptr;
                    visit(key, missing, std::addressof(value));
                }
            }
        }
    } else {
        static_assert(DiffLeaf<T>,
            "oi_debug::diff: unsupported leaf type; register fields or provide const to_string() and equality");
        if constexpr (DiffLeaf<T>) {
            if (!before || !after || !diff_leaf_equal(*before, *after)) {
                context.record(before, after, path);
            }
        }
    }
}

} // namespace detail

template <typename T>
[[nodiscard]] diff_report diff(const T &before, const T &after,
                                diff_options options = {}) {
    detail::diff_context context{options, {}};
    std::string path;
    detail::diff_walk(std::addressof(before), std::addressof(after), path, 0, context);
    return std::move(context.report);
}

[[nodiscard]] inline std::string diff_to_string(const diff_report &report) {
    std::string text;
    for (const auto &entry : report.changes) {
        if (!text.empty()) {
            text.push_back('\n');
        }
        text.append("  ").append(entry.path).append(": ");
        text.append(entry.before ? *entry.before : "<missing>");
        text.append(" -> ");
        text.append(entry.after ? *entry.after : "<missing>");
    }
    if (!report.complete()) {
        if (!text.empty()) {
            text.push_back('\n');
        }
        text.append("  comparison stopped: ");
        switch (report.stop_reason) {
        case diff_stop_reason::max_changes: text.append("max_changes"); break;
        case diff_stop_reason::max_nodes: text.append("max_nodes"); break;
        case diff_stop_reason::max_depth: text.append("max_depth"); break;
        case diff_stop_reason::none: break;
        }
        text.append(" (remaining values not compared)");
    } else if (text.empty()) {
        text = "  no changes";
    }
    return text;
}

template <typename T>
void debug_diff_impl(const T &before, const T &after,
                     std::string_view before_name, std::string_view after_name,
                     diff_options options = {}) {
    const auto report = diff(before, after, options);
    std::string content(before_name);
    content.append(" -> ").append(after_name).append(":\n");
    content.append(diff_to_string(report)).push_back('\n');
    std::cout.write(content.data(), static_cast<std::streamsize>(content.size()));
}

} // namespace oi_debug

#define debug_diff(before, after, ...) \
    ::oi_debug::debug_diff_impl((before), (after), #before, #after __VA_OPT__(,) __VA_ARGS__)

#endif // OI_DEBUG_DIFF_HPP
