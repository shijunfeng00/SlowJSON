//
// Created by hy-20 on 2024/7/24.
//

#ifndef SLOWJSON_LOAD_HPP
#define SLOWJSON_LOAD_HPP

#include "load_from_dict.hpp"
#include "dynamic_dict.hpp"

namespace slow_json {
    template<typename T>
    void loads(T &value, const std::string &json) {
        slow_json::dynamic_dict dict(json);
        LoadFromDict<T>::load(value, dict);
    }
}
#endif //SLOWJSON_LOAD_HPP
