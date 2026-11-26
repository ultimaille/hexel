#ifndef __ENUM_H__
#define __ENUM_H__

#include <array>
#include <string_view>

namespace enum_detail {
    constexpr std::size_t count(std::string_view s) {
        std::size_t n = 1;
        for (char c : s)
            n += c == ',';
        return n;
    }

    constexpr bool is_space(char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    // "A, B, C" -> "A\0B\0C\0\0"
    template <std::size_t L>
        constexpr std::array<char, L + 2> zero_separated(std::string_view s) {
            std::array<char, L + 2> out{};
            std::size_t j = 0;
            for (char c : s)
                if (!is_space(c))
                    out[j++] = c == ',' ? '\0' : c;
            return out;
        }

    constexpr const char *nth(const char *p, std::size_t i) {
        while (i--)
            p += std::char_traits<char>::length(p) + 1;
        return p;
    }
}

#define ENUM(Name, ...)                                                                                \
    struct Name {                                                                                      \
        enum class Choice { __VA_ARGS__ };                                                             \
        using enum Choice;                                                                             \
        static constexpr std::size_t size = enum_detail::count(#__VA_ARGS__);                          \
        static constexpr auto names = enum_detail::zero_separated<sizeof(#__VA_ARGS__)>(#__VA_ARGS__); \
        Choice choice{};                                                                               \
        constexpr Name() = default;                                                                    \
        constexpr Name(Choice v) : choice(v) {}                                                        \
        constexpr operator Choice() const { return choice; }                                           \
        friend constexpr const char *to_string(Name e) {                                               \
            return enum_detail::nth(names.data(), static_cast<std::size_t>(e.choice));                 \
        }                                                                                              \
    };

#endif //__ENUM_H__

