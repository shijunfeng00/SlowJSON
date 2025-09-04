//
// Created by hy-20 on 2024/7/24.
//

#ifndef SLOWJSON_LOAD_FROM_DICT_HPP
#define SLOWJSON_LOAD_FROM_DICT_HPP
#include "load_from_dict_interface.hpp"
#include "concepts.hpp"
#include "dict.hpp"
#include "enum.hpp"
#include "serializable.hpp"
#include "visit.hpp"

namespace slow_json {
    template<concepts::fundamental T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const dict &dict) {
            assert_with_message(!dict.is_null(), "试图将空对象解析为%s", type_name_v<T>.str);
            //dict.cast本身已经提供了fundamental_type+string/string_view/const char*的支持，因此这里可以直接调用
            value = dict.cast<T>();
        }
    };

    template<concepts::string T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dict &dict) {
            assert_with_message(!dict.is_null(), "试图将空对象解析为%s", type_name_v<T>.str);
            if constexpr(concepts::array<T> && std::is_same_v<std::decay_t<T>,char *>){ //char[N]类型
                //char value[N]类型，已经分配好空间了，直接往value拷贝就行
                const auto&data=dict.cast_cref<std::string>();
                memcpy(value, data.c_str(), data.size() + 1);
            } else if constexpr (std::is_same_v<T, const char *>) {
                const auto&data=dict.cast_cref<std::string>();
                // 反序列化中，假设对象本身指针是无效/空的，因此要在这里进行分配工作
                char *data_cp = new char[data.size() + 1];
                memcpy(data_cp, data.c_str(), data.size() + 1);
                value = data_cp;
            } else {
                // std::string,std::string_view
                value = dict.cast<T>();
            }
        }
    };

    template<concepts::pointer T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dict &dict) {
            if(dict.is_null()){
                value=nullptr;
                return;
            }
            if constexpr(std::is_pointer_v<T>){;
                using base_type=std::remove_pointer_t<T>;
                if(value==nullptr){
                    value=new base_type();
                }
                LoadFromDict<base_type>::load(*value, dict);
            }else{
                //智能指针,shared_ptr,unique_ptr
                using element_type=T::element_type;
                if(value==nullptr) {
                    value = T(new element_type());
                }
                LoadFromDict<element_type>::load(*value, dict);
            }
        }
    };

    template<concepts::iterable T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dict &dict) {
            assert_with_message(dict.is_array(), "数据不能转化为list");
            using element_type = std::remove_const_t<std::remove_reference_t<decltype(*std::begin(value))>>;
            if constexpr (concepts::array<T>) {
                auto array_size = std::size(value);
                assert_with_message(dict.size() <= array_size, "数组越界，数组大小为%zu，实际希望大小为%zu", array_size,
                                    dict.size());
                for (int i = 0; i < dict.size(); i++) {
                    LoadFromDict<element_type>::load(value[i], dict[i]);
                }
            } else if constexpr (slow_json::concepts::set<T>) {
                value.clear();
                for (int i = 0; i < dict.size(); i++) {
                    element_type element;
                    LoadFromDict<element_type>::load(element, dict[i]);
                    value.emplace(std::move(element));
                }
            } else {
                value.clear();
                for (int i = 0; i < dict.size(); i++) {
                    element_type element;
                    LoadFromDict<element_type>::load(element, dict[i]);
                    value.emplace_back(std::move(element));
                }
            }
        }
    };

    template<concepts::dict T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dict &dict) {
            assert_with_message(dict.is_dict(), "数据不能转化为dict");
            value.clear();
            for (const auto &[k, v]: dict.as_dict()) {
                using key_type = typename T::key_type;
                using value_type = typename T::mapped_type;
                key_type key=k; //对于字典，key总是可以变为字符串的才对
                value_type val;
                LoadFromDict<value_type>::load(val, v);
                value.emplace(std::move(key), std::move(val));
            }
        }
    };

    template<concepts::serializable T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dict &dict) {
            if constexpr(std::is_same_v<decltype(T::get_config()),slow_json::dict>){
                slow_json::dict config=T::get_config();
                for(const auto&it:config._data){
                    auto value_ptr=static_cast<const details::field_wrapper*>(it._value.value());
                    if(value_ptr!=nullptr){
                        // 目前只考虑在get_config中用于自定义class对象的反序列化
                        auto&field_value=*((details::field_wrapper*)value_ptr);
                        field_value._object_ptr=&value;
                        field_value.load_fn(dict[it._key]);
                    }else{
                        assert_with_message(value_ptr!=nullptr,"目前只考虑在get_config中用于自定义class对象的反序列化，尚未实现任意slow_json::dict的反序列化");
                    }
                };
            }
            else {
                const auto config = T::get_config();
                constexpr auto size_v = config.size_v;
                auto handle_pair = [&value, &dict](const auto &pair) {
                    auto &[field_name, field_ptr] = pair;
                    auto &field_value = value.*field_ptr;
                    using field_t = decltype(concepts::details::match_field_type(field_ptr));
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
        static void load_impl(T &value, const slow_json::dict &dict) {
            assert_with_message(dict.is_array(), "数据不能转化为二维点，数据非列表类型");
            assert_with_message(dict.size() == 2, "数据不能转化为二维点，数据长度不为2（长度为%d）", dict.size());
            LoadFromDict<decltype(value.x)>::load(value.x, dict[0]);
            LoadFromDict<decltype(value.y)>::load(value.y, dict[1]);
        }
    };

    template<concepts::tuple T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dict &dict) {
            assert_with_message(dict.is_array(), "数据不能转化为list");
            assert_with_message(dict.size() == std::tuple_size_v<T>,
                                "list中元素数量和std::tuple<Args...>参数数量不对等");
            auto fn = [&]<typename U>(U &value, std::size_t index) {
                LoadFromDict<U>::load(value, dict[index]);
            };
            auto apply = [&]<std::size_t...index>(std::index_sequence<index...> &&) {
                (fn(std::get<index>(value), index), ...);
            };
            apply(std::make_index_sequence<std::tuple_size_v<T>>{});
        }
    };

    template<concepts::pair T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dict &dict) {
            assert_with_message(dict.is_array() && dict.size() == 2, "数据不能转化为std::pair");
            LoadFromDict<decltype(value.first)>::load(value.first, dict[0]);
            LoadFromDict<decltype(value.second)>::load(value.second, dict[1]);
        }
    };

    template<concepts::eigen_point T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dict &dict) {
            assert_with_message(dict.is_array() && dict.size() == 2, "数据不能转化为二维点");
            LoadFromDict<decltype(value.x())>::load(value.x(), dict[0]);
            LoadFromDict<decltype(value.y())>::load(value.x(), dict[1]);
        }
    };

    template<concepts::enumerate T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dict &dict) {
            const char *enum_str = dict.cast<const char*>();
            value = details::string2enum<T>(enum_str);
        }
    };

    /**
     * 根据get_config传入一个slow_json::dict对象，提供成员属性指针和名称，据此来完成解析
     * @tparam T
     */
    template<concepts::serializable_oop T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dict &dict) SLOW_JSON_NOEXCEPT {
            value.from_config(dict);
        }
    };

    template<concepts::optional T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dict &dict) SLOW_JSON_NOEXCEPT {
            typename T::value_type object;
            if (!dict.is_null()) {
                LoadFromDict<typename T::value_type>::load(object, dict);
                value.emplace(std::move(object));
            }
        }
    };
}
#endif //SLOWJSON_LOAD_FROM_DICT_HPP
