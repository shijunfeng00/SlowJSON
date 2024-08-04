//
// Created by hy-20 on 2024/8/5.
//

#ifndef SLOWJSON_SERIALIZABLE_HPP
#define SLOWJSON_SERIALIZABLE_HPP

#include "polymorphic_dict.hpp"
#include "dynamic_dict.hpp"
#include "dump_to_string_interface.hpp"
#include "load_from_dict_interface.hpp"

namespace slow_json {
    /**
     * @brief 规定一个类型是可以与JSON相互转换的接口类
     * @details 该接口定义了两个纯虚函数，get_config和from_config，分别用于将对象转换为JSON，和将JSON转换为对象
     *          因此实现了这两个接口的类，都是可序列化的类
     */
    struct ISerializable {
        /**
         * 将对象序列化为JSON
         * @return
         */
        [[nodiscard]] virtual slow_json::polymorphic_dict get_config() const noexcept = 0;

        /**
         * 从JSON中反序列化对象
         * @param json
         */
        virtual void from_config(const slow_json::dynamic_dict &json) = 0;

        virtual ~ISerializable() = default;
    };

}

#endif //SLOWJSON_SERIALIZABLE_HPP
