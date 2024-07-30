//
// Creat天哦——：——ted by hy-20 on 2024/7/22.
//

#ifndef SLOWJSON_DUMP_HPP
#define SLOWJSON_DUMP_HPP

#include "dump_to_string.hpp"
#include "indent.hpp"
#include "polymorphic_dict.hpp"

namespace slow_json {
    /**
     * 对上述的函偏特化类做一个封装，自动去调用不同的偏特化函数
     * @tparam T
     * @param buffer
     * @param value
     * @param indent 首行缩进，如果不需要可设置为空
     */
    template<typename T>
    void dumps(Buffer &buffer, const T &value, std::optional<std::size_t> indent = std::nullopt) {
        DumpToString<T>::dump(buffer, value);
        if (indent) {
            slow_json::indent(buffer, indent.value());
        }
    }
}
#endif //SLOWJSON_DUMP_HPP
