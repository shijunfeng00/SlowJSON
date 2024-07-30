//
// Created by hy-20 on 2024/7/24.
//

#ifndef SLOWJSON_LOAD_FROM_DICT_INTERFACE_HPP
#define SLOWJSON_LOAD_FROM_DICT_INTERFACE_HPP
#define SLOW_JSON_SUPPORTED false

#include "dynamic_dict.hpp"

namespace slow_json {
    template<typename Self>
    struct ILoadFromDict {
        template<typename T>
        static void load(T &value, const slow_json::dynamic_dict &dict) {
            return Self::load_impl(value, dict);
        }
    };

    template<typename T>
    struct LoadFromDict : public ILoadFromDict<LoadFromDict<T>> {
        static constexpr bool not_supported_flag = true; //如果某个类型匹配到了这个偏特化，则说明该类型是不被支持的
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            assert_with_message(SLOW_JSON_SUPPORTED,
                                "无法将JSON字段转化为类型为'%s'的对象正确对象，找不到对应的LoadFromDict偏特化类",
                                slow_json::type_name_v<T>.with_end().str);
        }
    };
}
#endif //SLOWJSON_LOAD_FROM_DICT_INTERFACE_HPP
