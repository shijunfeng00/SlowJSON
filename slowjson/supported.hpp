//
// Created by hy-20 on 2024/7/29.
//

#ifndef SLOWJSON_SUPPORTED_HPP
#define SLOWJSON_SUPPORTED_HPP

#include "dump_to_string.hpp"
#include "load_from_dict.hpp"

namespace slow_json::concepts {

    /**
     * 是否支持反序列化
     * @tparam T
     */
    template<typename T>
    concept load_supported=!requires(T a){ LoadFromDict<T>::not_supported_flag; };

    /**
     * 是否支持序列化
     * @tparam T
     */
    template<typename T>
    concept dump_supported=!requires(T a){ DumpToString<T>::not_supported_flag; };

    /**
     * 是否同时支持序列化与反序列化
     * @tparam T
     */
    template<typename T>
    concept supported=load_supported<T> && dump_supported<T>;
}
#endif //SLOWJSON_SUPPORTED_HPP
