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

#define ENUM(Name, ...)                                                                                      \
    enum class Name { __VA_ARGS__ };                                                                         \
    inline constexpr auto Name##_names = enum_detail::split<enum_detail::count(#__VA_ARGS__)>(#__VA_ARGS__); \
    constexpr std::string_view to_string(Name e) { return Name##_names[static_cast<std::size_t>(e)]; }

#endif //__ENUM_H__

