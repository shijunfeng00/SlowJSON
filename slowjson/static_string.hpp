//
// Created by hy-20 on 2024/7/12.
//

#ifndef SLOWJSON_STATIC_STRING_HPP
#define SLOWJSON_STATIC_STRING_HPP

#include<type_traits>
#include<utility>
#include<string_view>

namespace slow_json {
    /**
     * @brief 编译时的静态 字符串表示。
     *
     * @tparam chs 字符串的字符。
     */
    template<char...chs>
    struct StaticString {
        /// 静态字符串数组。
        constexpr static const char str[] = {chs...};

        /**
         * @brief 将 StaticString 转换为 C 风格字符串。
         *
         * @return const char* C 风格字符串。
         */
        constexpr operator const char *() const noexcept {
            return str;
        }

        /**
         * 拼接两个字符串
         * @tparam chs2
         * @return
         */
        template<char...chs2>
        StaticString<chs..., chs2...> operator+(StaticString<chs2...> &&) {
            return StaticString<chs..., chs2...>{};
        }

        constexpr static StaticString<chs..., '\0'> with_end() noexcept {
            return StaticString<chs..., '\0'>{};
        }
    };
    namespace static_string_literals {
        /**
         * @brief 用于创建 StaticString 的用户定义字面量。
         *
         * @tparam T 字符类型。
         * @tparam chs 字符。
         * @return constexpr StaticString<chs...> 创建的 StaticString。
         */
        template<typename T, T...chs>
        constexpr StaticString<chs...> operator ""_ss() {
            return StaticString<chs...>();
        }
    }
};
#endif //SLOWJSON_STATIC_STRING_HPP
