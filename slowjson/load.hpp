//
// Created by hy-20 on 2024/7/24.
//

#ifndef SLOWJSON_LOAD_HPP
#define SLOWJSON_LOAD_HPP

#include "load_from_dict.hpp"
#include "dict_handler.hpp"

namespace slow_json {
    /**
     * 从字符串中加载JSON，并用其来反序列化对象
     * @tparam T 被反序列化的对象类型
     * @param value 被反序列化的对象
     * @param json JSON字符串
     */
    template<typename T>
    static void loads(T &value, const std::string &json) {
        if constexpr(std::is_same_v<T,slow_json::dict>){
            value=slow_json::dict::from_string(json);
        }else{
            slow_json::dict dict=details::DictHandler::parse_json_to_dict(json);
            LoadFromDict<T>::load(value, dict);
        }
    }

    template<typename T>
    static T loads(const std::string&json){
        if constexpr(std::is_same_v<T,slow_json::dict>){
            return details::DictHandler::parse_json_to_dict(json);
        }else{
            if constexpr(std::is_default_constructible_v<T>){
                T value;
                slow_json::dict dict=details::DictHandler::parse_json_to_dict(json);
                LoadFromDict<T>::load(value, dict);
                return value;
            }else{
                assert_with_message(
                        std::is_default_constructible_v<T>,
                        "类型%s没有无参构造函数",
                        slow_json::type_name_v<T>.with_end().str);
            }
        }
    }
}
#endif //SLOWJSON_LOAD_HPP
