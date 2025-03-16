//
// Creat天哦——：——ted by hy-20 on 2024/7/22.
//

#ifndef SLOWJSON_DUMP_HPP
#define SLOWJSON_DUMP_HPP

#include "dump_to_string.hpp"
#include "indent.hpp"
#include "dict.hpp"

namespace slow_json {
    /**
     * 对DumpToString特化类做一个封装，根据不同类型去调用不同的偏特化函数
     * @tparam T 被转换为JSON的对象类型
     * @param buffer 存储转换之后的JSON的缓冲区对象
     * @param value 被转化为JSON的对象
     * @param indent 首行缩进长度，如果不需要可设置为空
     */
    template<typename T>
    static void dumps(Buffer &buffer, const T &value, std::optional<std::size_t> indent = std::nullopt) {
        DumpToString<T>::dump(buffer, value);
        if (indent) {
            slow_json::indent(buffer, indent.value());
        }
    }
}
#endif //SLOWJSON_DUMP_HPP
