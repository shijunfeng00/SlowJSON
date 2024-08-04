//
// Created by hy-20 on 2024/7/18.
//

#ifndef SLOWJSON_DUMP_TO_STRING_INTERFACE_HPP
#define SLOWJSON_DUMP_TO_STRING_INTERFACE_HPP
#define SLOW_JSON_SUPPORTED false

#include "buffer.hpp"
#include "type_name.hpp"
#include "concetps.hpp"

namespace slow_json {
    template<typename Self>
    struct IDumpToString {
        template<typename T>
        static void dump(Buffer &buffer, const T &value) {
            return Self::dump_impl(buffer, value);
        }
    };

    /**
     * 处理匹配失败的情况，如果没有偏特化的模板类匹配成功，则最终会调用这个类的静态方法
     * @tparam T
     */
    template<typename T>
    struct DumpToString : public IDumpToString<DumpToString<T>> {
        static constexpr bool not_supported_flag = true; //如果某个类型匹配到了这个偏特化，则说明该类型是不被支持的
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
            assert_with_message(SLOW_JSON_SUPPORTED,
                                "无法将类型为'%s'的对象正确转换为字符串，找不到对应的DumpToString偏特化类",
                                type_name_v<T>.str);
        }
    };
}
#endif //SLOWJSON_DUMP_TO_STRING_INTERFACE_HPP
