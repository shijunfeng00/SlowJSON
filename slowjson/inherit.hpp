//
// Created by hy-20 on 2024/7/30.
//

#ifndef SLOWJSON_MERGER_HPP
#define SLOWJSON_MERGER_HPP

#include "concetps.hpp"

namespace slow_json {
    template<slow_json::concepts::slow_json_static_dict Ta, slow_json::concepts::slow_json_static_dict Tb>
    constexpr auto merge(const Ta &a, const Tb &b) {
        constexpr auto size1 = std::tuple_size_v<typename Ta::super_type>;
        constexpr auto size2 = std::tuple_size_v<typename Tb::super_type>;
        return [a, b]<std::size_t...index1, std::size_t...index2>(std::index_sequence<index1...> &&,
                                                                  std::index_sequence<index2...> &&) {
            return slow_json::static_dict{std::get<index1>(a)..., std::get<index2>(b)...};
        }(std::make_index_sequence<size1>{}, std::make_index_sequence<size2>{});
    }

    template<typename SuperClass, concepts::slow_json_static_dict T>
    constexpr auto inherit(const T &subclass_info) {
        return slow_json::merge(SuperClass::get_config(), subclass_info);
    }
}
#endif //SLOWJSON_MERGER_HPP
