//
// Created by hy-20 on 2024/7/24.
//

#ifndef SLOWJSON_LOAD_FROM_DICT_HPP
#define SLOWJSON_LOAD_FROM_DICT_HPP

#include "load_from_dict_interface.hpp"
#include "concetps.hpp"
#include "polymorphic_dict.hpp"
#include "enum.hpp"
namespace slow_json {
    template<concepts::fundamental T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            if constexpr (std::is_same_v<T, uint8_t>) {
                auto data = dict.value()->Get<int>();
                assert_with_message(0 <= data && data <= 255, "发现整型溢出行为");
                value = data;
            } else if constexpr (std::is_same_v<T, int8_t>) {
                auto data = dict.value()->Get<int>();
                assert_with_message(-128 <= data && data <= 127, "发现整型溢出行为");
                value = data;
            } else {
                value = dict.value()->Get<T>();
            }
        }
    };

    template<concepts::string T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            if constexpr (std::is_same_v<T, const char *>) {
                const char *data = dict.value()->GetString();
                std::size_t size = strlen(data);
                char *data_cp = new char[size + 1];
                memcpy(data_cp, data, size + 1);
                value = data_cp;
            } else {
                value = dict.value()->GetString();
            }
        }
    };

    template<concepts::iterable T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            using element_type = std::remove_const_t<std::remove_reference_t<decltype(*std::begin(value))>>;
            if constexpr (concepts::array<T>) {
                auto array_size = std::size(value);
                assert_with_message(dict.size() <= array_size, "数组越界，数组大小为%zu，实际希望大小为%zu", array_size,
                                    dict.size());
                for (int i = 0; i < dict.size(); i++) {
                    LoadFromDict<element_type>::load(value[i], dict[i]);
                }
            } else if constexpr (slow_json::concepts::set<T>) {
                for (int i = 0; i < dict.size(); i++) {
                    element_type element;
                    LoadFromDict<element_type>::load(element, dict[i]);
                    value.emplace(element);
                }
            } else {
                for (int i = 0; i < dict.size(); i++) {
                    element_type element;
                    LoadFromDict<element_type>::load(element, dict[i]);
                    value.emplace_back(element);
                }
            }
        }
    };

    template<concepts::dict T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            for (const auto &[k, v]: dict.value()->GetObject()) {
                using key_type = typename T::key_type;
                using value_type = typename T::mapped_type;
                key_type key;
                LoadFromDict<key_type>::load(key, dynamic_dict::wrap(k));
                value_type val;
                LoadFromDict<value_type>::load(val, dynamic_dict::wrap(v));
                value.emplace(std::move(key), std::move(val));
            }
        }
    };

    template<concepts::serializable T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            if constexpr (std::is_same_v<decltype(T::get_config()), polymorphic_dict>) {
                polymorphic_dict config = T::get_config();
                config.set_object(value);
                LoadFromDict<polymorphic_dict>::load(config, dict);
            } else {
                constexpr auto config = T::get_config();
                constexpr auto size_v = config.size_v;
                auto handle_pair = [&value, &dict](const auto &pair) {
                    auto &[field_name, field_ptr] = pair;
                    auto &field_value = value.*field_ptr;
                    using field_t = decltype(concepts::helper::match_field_type(field_ptr));
                    LoadFromDict<field_t>::load(field_value, dict[field_name.with_end().str]);
                };
                [&config, &handle_pair]<std::size_t...index>(std::index_sequence<index...> &&) {
                    (handle_pair(std::get<index>(config)), ...);
                }(std::make_index_sequence<size_v>());
            }
        }
    };

    template<concepts::cv_point T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            assert_with_message(dict.is_array() && dict.size() == 2, "数据不能转化为二维点");
            LoadFromDict<decltype(value.x)>::load(value.x, dict[0]);
            LoadFromDict<decltype(value.y)>::load(value.x, dict[1]);
        }
    };

    template<concepts::eigen_point T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            assert_with_message(dict.is_array() && dict.size() == 2, "数据不能转化为二维点");
            LoadFromDict<decltype(value.x())>::load(value.x(), dict[0]);
            LoadFromDict<decltype(value.y())>::load(value.x(), dict[1]);
        }
    };

    template<concepts::enumerate T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            const char *enum_str = dict.value()->GetString();
            value = slow_json::string2enum<T>(enum_str);
        }
    };

}
#endif //SLOWJSON_LOAD_FROM_DICT_HPP
