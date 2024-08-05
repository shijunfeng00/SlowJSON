//
// Created by hy-20 on 2024/7/24.
//

#ifndef SLOWJSON_LOAD_HPP
#define SLOWJSON_LOAD_HPP

#include "load_from_dict.hpp"
#include "dynamic_dict.hpp"

namespace slow_json {
    /**
     * 从字符串中加载JSON，并用其来反序列化对象
     * @tparam T 被反序列化的对象类型
     * @param value 被反序列化的对象
     * @param json JSON字符串
     */
    template<typename T>
    static void loads(T &value, const std::string &json) {
        slow_json::dynamic_dict dict(json);
        LoadFromDict<T>::load(value, dict);
    }
}
#endif //SLOWJSON_LOAD_HPP
