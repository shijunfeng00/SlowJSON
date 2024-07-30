//
// Created by hy-20 on 2024/7/23.
//

#ifndef SLOWJSON_POLYMORPHIC_DICT_HPP
#define SLOWJSON_POLYMORPHIC_DICT_HPP

#include "dump_to_string_interface.hpp"
#include "static_dict.hpp"
#include <functional>

namespace slow_json {

    struct polymorphic_dict {
        friend struct IDumpToString<DumpToString<polymorphic_dict>>;
    public:
        template<typename...Args>
        constexpr explicit polymorphic_dict(Args &&...args):
                _object{nullptr},
                _dump_fn{[this, args...](slow_json::Buffer &buffer) {
                    using pair_t = std::tuple_element_t<0, std::tuple<Args...>>;
                    using field_t = typename pair_t::second_type;
                    if constexpr (std::is_member_object_pointer_v<field_t>) {
                        using class_t = decltype(concepts::helper::match_class_type(field_t{}));
                        class_t &object = *static_cast<class_t *>(_object);
                        auto add_object = [&object](auto &pair) {
                            auto &[name, field_pointer] = pair;
                            return std::pair{name, object.*field_pointer};
                        };
                        auto data = slow_json::static_dict{add_object(args)...};
                        slow_json::DumpToString<decltype(data)>::dump(buffer, data);
                    } else {
                        auto data = slow_json::static_dict{args...};
                        slow_json::DumpToString<decltype(data)>::dump(buffer, data);

                    }
                }} {}

        template<typename T>
        void set_object(const T &object) {
            this->_object = const_cast<T *>(&object);
        }

        template<typename T>
        requires requires(T t){
            !std::is_const_v<T>;
        }
        void set_object(T &object) {
            this->_object = &object;
        }

        void *_object;
        std::function<void(slow_json::Buffer &)> _dump_fn;
    };

    template<>
    struct DumpToString<polymorphic_dict> : public IDumpToString<DumpToString<polymorphic_dict>> {
        static void dump_impl(Buffer &buffer, const polymorphic_dict &value) noexcept {
            value._dump_fn(buffer);
        }
    };
}
#endif //SLOWJSON_POLYMORPHIC_DICT_HPP
