//
// Created by hy-20 on 2024/7/23.
//

#ifndef SLOWJSON_POLYMORPHIC_DICT_HPP
#define SLOWJSON_POLYMORPHIC_DICT_HPP

#include "dump_to_string_interface.hpp"
#include "load_from_dict_interface.hpp"
#include "static_dict.hpp"
#include "dynamic_dict.hpp"
#include <functional>

namespace slow_json {

    /**
     * @brief 基于多态(std::function)实现的多态字典，避免模板参数
     * @details 不支持数据查寻，只能支持将其转化为JSON，或者配合get_config()将对象与JSON相互转换
     */
    struct polymorphic_dict {
        friend struct IDumpToString<DumpToString<polymorphic_dict>>;
    public:
        polymorphic_dict(const polymorphic_dict &d1, const polymorphic_dict &d2) :
                _object(nullptr),
                _dump_fn([d1, d2](const polymorphic_dict*self,slow_json::Buffer &buffer) {
                    d1._dump_fn(self,buffer);
                    std::size_t position = buffer.size() - 1;
                    buffer.resize(position);
                    d2._dump_fn(self,buffer);
                    buffer[position] = ',';
                }),
                _load_fn([](const slow_json::dynamic_dict &) {
                    assert_with_message(false, "没有实现该方法");
                }) {}

        template<typename...Args> requires ((concepts::pair<Args>) && ...)
        constexpr explicit polymorphic_dict(Args &&...args):
                _object{nullptr},
                _dump_fn{[args...](const polymorphic_dict*self,slow_json::Buffer &buffer) {
                    using pair_t = std::tuple_element_t<
                            std::tuple_size_v<std::tuple<Args...>> - 1, std::tuple<Args...>>;
                    using field_t = typename pair_t::second_type;
                    if constexpr (std::is_member_object_pointer_v<field_t>) {
                        using class_t = decltype(concepts::helper::match_class_type(field_t{}));
                        class_t &object = *static_cast<class_t *>(self->_object);
                        auto add_object = [&object](auto &pair) {
                            auto &[name, field_pointer] = pair;
                            return std::pair{name, std::ref(object.*field_pointer)};
                        };
                        auto data = slow_json::static_dict{add_object(args)...};
                        slow_json::DumpToString<decltype(data)>::dump(buffer, data);
                    } else {
                        auto data = slow_json::static_dict{args...};
                        slow_json::DumpToString<decltype(data)>::dump(buffer, data);

                    }
                }},
                _load_fn{[this, args...](const slow_json::dynamic_dict &dict) {
                    auto add_object = [this, &dict](auto &pair) {
                        using pair_t = std::remove_const_t<std::remove_reference_t<decltype(pair)>>;
                        using field_pointer_t = typename pair_t::second_type;
                        if constexpr (std::is_member_object_pointer_v<field_pointer_t>) {
                            using class_t = decltype(concepts::helper::match_class_type(field_pointer_t{}));
                            class_t &object = *static_cast<class_t *>(_object);
                            using field_t = decltype(concepts::helper::match_field_type(
                                    std::declval<field_pointer_t>()));
                            auto &[name, field_pointer] = pair;
                            field_t &field_value = object.*field_pointer;
                            std::string_view field_name = name.with_end().str;
                            LoadFromDict<field_t>::load(field_value, dict[field_name]);
                        }
                    };
                    (add_object(args), ...);

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

        void * _object;
        std::function<void(const polymorphic_dict*self,slow_json::Buffer &)> _dump_fn;
        std::function<void(const slow_json::dynamic_dict &)> _load_fn;
    };

    template<>
    struct DumpToString<polymorphic_dict> : public IDumpToString<DumpToString<polymorphic_dict>> {
        static void dump_impl(Buffer &buffer, const polymorphic_dict &value) noexcept {
            value._dump_fn(&value,buffer);
        }
    };

    template<>
    struct LoadFromDict<polymorphic_dict> : public ILoadFromDict<LoadFromDict<polymorphic_dict>> {
        static void load_impl(polymorphic_dict &value, const slow_json::dynamic_dict &dict) noexcept {
            value._load_fn(dict);
        }
    };
}
#endif //SLOWJSON_POLYMORPHIC_DICT_HPP
