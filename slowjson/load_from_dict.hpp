//
// Created by hy-20 on 2024/7/24.
//

#ifndef SLOWJSON_LOAD_FROM_DICT_HPP
#define SLOWJSON_LOAD_FROM_DICT_HPP
#include "load_from_dict_interface.hpp"
#include "concetps.hpp"
#include "dict.hpp"
#include "enum.hpp"
#include "serializable.hpp"
namespace slow_json {
    template<concepts::fundamental T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const dynamic_dict &dict) {
            const rapidjson::Value *v = dict._value;
            // 布尔类型
            if constexpr (std::is_same_v<T, bool>) {
                assert_with_message(v->IsBool(), "期望 JSON 为布尔类型");
                value = v->GetBool();
            }
                // 有符号整数类型（包括 char、short、int、long、long long）
            else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
                assert_with_message(v->IsInt() || v->IsInt64(), "期望 JSON 为有符号整数类型");
                int64_t tmp = v->IsInt() ? v->GetInt() : v->GetInt64();
                assert_with_message(tmp >= static_cast<int64_t>(std::numeric_limits<T>::lowest()) &&
                                    tmp <= static_cast<int64_t>(std::numeric_limits<T>::max()),
                                    "有符号整数溢出，超出类型范围");
                value = static_cast<T>(tmp);
            }
                // 无符号整数类型（包括 unsigned char、unsigned short、unsigned int、unsigned long、unsigned long long）
            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
                assert_with_message(v->IsUint() || v->IsUint64(), "期望 JSON 为无符号整数类型");
                uint64_t tmp = v->IsUint() ? v->GetUint() : v->GetUint64();
                assert_with_message(tmp <= static_cast<uint64_t>(std::numeric_limits<T>::max()),
                                    "无符号整数溢出，超出类型范围");
                value = static_cast<T>(tmp);
            }
                // 浮点数类型（包括 float、double、long double）
            else if constexpr (std::is_floating_point_v<T>) {
                assert_with_message(v->IsDouble() || v->IsNumber(), "期望 JSON 为浮点数类型");
                value = static_cast<T>(v->GetDouble());
            }
                // std::string
            else if constexpr (std::is_same_v<T, std::string>) {
                assert_with_message(v->IsString(), "期望 JSON 为字符串类型以解析为 std::string");
                value.assign(v->GetString(), v->GetStringLength());
            }
                // std::string_view
            else if constexpr (std::is_same_v<T, std::string_view>) {
                assert_with_message(v->IsString(), "期望 JSON 为字符串类型以解析为 std::string_view");
                value = std::string_view(v->GetString(), v->GetStringLength());
            }
                // 其他类型（如对象或数组）需自定义特化或加载器
            else {
                static_assert(!sizeof(T), "未为该类型提供 LoadFromDict 特化或实现");
            }
        }
    };

    template<concepts::string T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            assert_with_message(!dict._value->IsNull(), "试图将空对象解析为%s", type_name_v<T>.str);
            if constexpr (std::is_same_v<T, const char *>) {
                const char *data = dict._value->GetString();
                std::size_t size = strlen(data);
                char *data_cp = new char[size + 1];
                memcpy(data_cp, data, size + 1);
                value = data_cp;
            } else {
                value = dict._value->GetString();
            }
        }
    };

    template<concepts::pointer T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            //std::cout<<"caonima1"<<std::endl;
            if(dict._value->IsNull()){
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
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
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
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            assert_with_message(dict.is_object(), "数据不能转化为dict");
            value.clear();
            for (const auto &[k, v]: dict.as_dict()) {
                using key_type = typename T::key_type;
                using value_type = typename T::mapped_type;
                key_type key=k.data(); //对于字典，key总是可以变为字符串的才对
                value_type val;
                LoadFromDict<value_type>::load(val, v);
                value.emplace(std::move(key), std::move(val));
            }
        }
    };

    template<concepts::serializable T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
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
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            assert_with_message(dict.is_array(), "数据不能转化为二维点，数据非列表类型");
            assert_with_message(dict.size() == 2, "数据不能转化为二维点，数据长度不为2（长度为%d）", dict.size());
            LoadFromDict<decltype(value.x)>::load(value.x, dict[0]);
            LoadFromDict<decltype(value.y)>::load(value.y, dict[1]);
        }
    };

    template<concepts::tuple T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
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
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) {
            assert_with_message(dict.is_array() && dict.size() == 2, "数据不能转化为std::pair");
            LoadFromDict<decltype(value.first)>::load(value.first, dict[0]);
            LoadFromDict<decltype(value.second)>::load(value.second, dict[1]);
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
            const char *enum_str = dict._value->GetString();
            value = details::string2enum<T>(enum_str);
        }
    };

    /**
     * 用户有自己的想法，根据get_config传入一个slow_json::dynamic_dict对象，用户自己来完成解析
     * @tparam T
     */
    template<concepts::serializable_oop T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) noexcept {
            value.from_config(dict);
        }
    };

    template<concepts::optional T>
    struct LoadFromDict<T> : public ILoadFromDict<LoadFromDict<T>> {
        static void load_impl(T &value, const slow_json::dynamic_dict &dict) noexcept {
            typename T::value_type object;
            if (!dict.empty()) {
                LoadFromDict<typename T::value_type>::load(object, dict);
                value.emplace(std::move(object));
            }
        }
    };

}
#endif //SLOWJSON_LOAD_FROM_DICT_HPP
