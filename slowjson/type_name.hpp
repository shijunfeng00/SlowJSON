//
// Created by hy-20 on 2024/7/12.
//

#ifndef SLOWJSON_TYPE_NAME_HPP
#define SLOWJSON_TYPE_NAME_HPP

#include "static_string.hpp"

namespace slow_json {
    /**
     * @brief 在编译时获取对象的类型名称。
     *
     * @tparam Object 对象的类型。
     * @param object 要获取类型名称的对象。
     * @return StaticString对象，作为编译期类型名称的表示。
     */
    template<typename Object>
    constexpr auto type_name(const Object &object) {
        // 获取当前函数的名称，包括模板参数
        constexpr std::string_view pretty_name = __PRETTY_FUNCTION__;

        // 提取类型名称部分，去掉前缀
        constexpr std::string_view pretty_name2 = pretty_name.substr(pretty_name.find("with") + 14);

        // 去掉末尾的字符，得到纯类型名称
        constexpr std::string_view name = pretty_name2.substr(0, pretty_name2.size() - 1);

        // 将类型名称转换为StaticString对象
        constexpr auto static_name = [&]<std::size_t...index>(std::index_sequence<index...> &&) {
            return StaticString64<name[index]..., '\0'>();
        }(std::make_index_sequence<name.size()>());

        // 返回StaticString对象
        return static_name;
    }

    /**
     * @brief 编译时的类型名称变量。
     *
     * @tparam Object 对象的类型。
     */
    template<typename Object>
    constexpr auto type_name_v = decltype(type_name(std::declval<Object>()))();
};
#endif //SLOWJSON_TYPE_NAME_HPP
