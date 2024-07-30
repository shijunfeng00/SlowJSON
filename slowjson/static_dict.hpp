//
// Created by hy-20 on 2024/7/23.
//

#ifndef SLOWJSON_STATIC_DICT_HPP
#define SLOWJSON_STATIC_DICT_HPP

#include<tuple>
#include "dump_to_string_interface.hpp"

namespace slow_json {
    template<typename...Args>
    struct static_dict : public std::tuple<Args...> {
        using super_type = std::tuple<Args...>;

        constexpr explicit static_dict(Args &&...args) : std::tuple<Args...>(std::forward<Args>(args)...) {}

        constexpr explicit static_dict(const Args &...args) : std::tuple<Args...>(args...) {}

        constexpr static std::size_t size_v = std::tuple_size_v<std::tuple<Args...>>;

        template<std::size_t index = 0, char...chs>
        constexpr auto &at(StaticString<chs...> &&) const noexcept {
            if constexpr (std::is_same_v<decltype(std::get<index>(*this).first), StaticString<chs...>>) {
                return std::get<index>(*this).second;
            } else if constexpr (index + 1 < size_v) {
                return this->at<index + 1>(slow_json::StaticString<chs...>());
            } else {
                static_assert(index + 1 < size_v, "找不到对应的元素");
                return std::get<0>(*this).second;
            }
        }

        template<char...chs>
        constexpr auto &operator[](StaticString<chs...> &&) const noexcept {
            return this->at(StaticString<chs...>());
        }
    };


}
#endif //SLOWJSON_STATIC_DICT_HPP
