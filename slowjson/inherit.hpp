//
// Created by hy-20 on 2024/7/30.
//

#ifndef SLOWJSON_MERGER_HPP
#define SLOWJSON_MERGER_HPP

#include "concetps.hpp"

namespace slow_json {
    /**
     * 合并两个slow_json::static_string
     * @tparam Ta
     * @tparam Tb
     * @param a
     * @param b
     * @return
     */
    template<slow_json::concepts::slow_json_static_dict Ta, slow_json::concepts::slow_json_static_dict Tb>
    constexpr auto merge(const Ta &a, const Tb &b) {
        constexpr auto size1 = std::tuple_size_v<typename Ta::super_type>;
        constexpr auto size2 = std::tuple_size_v<typename Tb::super_type>;
        return [a, b]<std::size_t...index1, std::size_t...index2>(std::index_sequence<index1...> &&,
                                                                  std::index_sequence<index2...> &&) {
            return slow_json::static_dict{std::get<index1>(a)..., std::get<index2>(b)...};
        }(std::make_index_sequence<size1>{}, std::make_index_sequence<size2>{});
    }

    /**
     * 为派生类的序列化提供支持
     * @tparam SuperClass 父类类型
     * @tparam T 派生类类型
     * @param subclass_info 派生类的属性信息
     * @return 合并父类和派生类属性信息的slow_json::static_dict
     */
    template<typename SuperClass, concepts::slow_json_static_dict T>
    constexpr auto inherit(const T &subclass_info) {
        return slow_json::merge(SuperClass::get_config(), subclass_info);
    }
}
#endif //SLOWJSON_MERGER_HPP
