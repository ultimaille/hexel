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

    template <std::size_t N>
        constexpr std::array<std::string_view, N> split(std::string_view s) {
            std::array<std::string_view, N> out{};
            for (auto &token : out) {
                auto comma = s.find(',');
                token = s.substr(0, comma);
                while (!token.empty() && token.front() == ' ') token.remove_prefix(1);
                while (!token.empty() && token.back()  == ' ') token.remove_suffix(1);
                s = comma == s.npos ? std::string_view{} : s.substr(comma + 1);
            }
            return out;
        }
}

#define ENUM(Name, ...)                                                                                          \
    struct Name {                                                                                                \
        enum class Value { __VA_ARGS__ };                                                                        \
        using enum Value;                                                                                        \
        static constexpr auto names = enum_detail::split<enum_detail::count(#__VA_ARGS__)>(#__VA_ARGS__);        \
        Value value{};                                                                                           \
        constexpr Name() = default;                                                                              \
        constexpr Name(Value v) : value(v) {}                                                                    \
        constexpr operator Value() const { return value; }                                                       \
        friend constexpr std::string_view to_string(Name e) { return names[static_cast<std::size_t>(e.value)]; } \
    };

#endif //__ENUM_H__

