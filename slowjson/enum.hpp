//
// Created by hy-20 on 2024/8/2.
//

#ifndef SLOWJSON_ENUM_HPP
#define SLOWJSON_ENUM_HPP

#include<string_view>
#include<array>
#include "concetps.hpp"

namespace slow_json {

    template<auto value>
    constexpr auto enum_name() {
        std::string_view name;
#if __GNUC__ || __clang__
        name = __PRETTY_FUNCTION__;
        std::size_t start = name.find('=') + 2;
        std::size_t end = name.size() - 1;
        name = std::string_view{name.data() + start, end - start};
        start = name.rfind("::");
#elif _MSC_VER
        name = __FUNCSIG__;
        std::size_t start = name.find('<') + 1;
        std::size_t end = name.rfind(">(");
        name = std::string_view{ name.data() + start, end - start };
        start = name.rfind("::");
#endif
        return start == std::string_view::npos ? name : std::string_view{
                name.data() + start + 2, name.size() - start - 2
        };
    }

    template<concepts::enumerate T, std::size_t N = 0>
    constexpr auto enum_max() {
        constexpr T value = static_cast<T>(N);
        if constexpr (enum_name<value>().find(")") == std::string_view::npos)
            return enum_max<T, N + 1>();
        else
            return N;
    }

    template<concepts::enumerate T>
    constexpr auto enum_name_list() {
        constexpr auto num = enum_max<T>();
        constexpr auto names = []<std::size_t... Is>(std::index_sequence<Is...>) {
            return std::array<std::string_view, num>{
                    enum_name<static_cast<T>(Is)>()...
            };
        }(std::make_index_sequence<num>{});
        return names;
    }

    template<concepts::enumerate T>
    constexpr auto enum2string(T value) {
        auto names = enum_name_list<T>();
        return names[static_cast<std::size_t>(value)];
    }

    template<concepts::enumerate T>
    T string2enum(const char *enum_str) {
        static std::unordered_map<std::string_view, T> mp = []() {
            std::unordered_map<std::string_view, T> mp;
            constexpr auto enum_list = slow_json::enum_name_list<T>();
            for (int i = 0; i < std::size(enum_list); i++) {
                mp[enum_list[i]] = static_cast<T>(i);
            }
            return mp;
        }();
        assert_with_message(mp.contains(enum_str), "字符串转换enum失败，没有这个值");
        return mp[enum_str];
    }

}
#endif //SLOWJSON_ENUM_HPP
