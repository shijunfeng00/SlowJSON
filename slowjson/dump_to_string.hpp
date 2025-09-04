//
// Created by hy-20 on 2024/7/19.
//

#ifndef SLOWJSON_DUMP_TO_STRING_HPP
#define SLOWJSON_DUMP_TO_STRING_HPP

#include "dump_to_string_interface.hpp"
#include "concepts.hpp"
#include "dict.hpp"
#include "enum.hpp"
#include "serializable.hpp"
#include "internal/dtoa.h"
#include <cmath>

namespace slow_json {
    /**
     * @brief 整数类型转字符串
     * @details 后面再来优化，现在先用STL的工具
     * @tparam T
     */
    template<concepts::integer T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            if constexpr (std::is_same_v<T, char>) {
                buffer += value;
            } else if constexpr (sizeof(T) <= 2) {
                // 16位及以下整数提升为32位处理
                if constexpr (std::is_signed_v<T>) {
                    int32_t v = static_cast<int32_t>(value);
                    buffer.try_reserve(buffer.size() + 12);
                    const char *end = rapidjson::internal::i32toa(v, buffer.end());
                    buffer.resize(buffer.size() + (end - buffer.end()));
                } else {
                    uint32_t v = static_cast<uint32_t>(value);
                    buffer.try_reserve(buffer.size() + 11);
                    const char *end = rapidjson::internal::u32toa(v, buffer.end());
                    buffer.resize(buffer.size() + (end - buffer.end()));
                }
            } else if constexpr (sizeof(T) == 4) {
                if constexpr (std::is_signed_v<T>) {
                    buffer.try_reserve(buffer.size() + 12);
                    const char *end = rapidjson::internal::i32toa(value, buffer.end());
                    buffer.resize(buffer.size() + (end - buffer.end()));
                } else {
                    // 使用无符号版本
                    buffer.try_reserve(buffer.size() + 11);
                    const char *end = rapidjson::internal::u32toa(value, buffer.end());
                    buffer.resize(buffer.size() + (end - buffer.end()));
                }
            } else if constexpr (sizeof(T) == 8) {
                if constexpr (std::is_signed_v<T>) {
                    buffer.try_reserve(buffer.size() + 21);
                    const char *end = rapidjson::internal::i64toa(value, buffer.end());
                    buffer.resize(buffer.size() + (end - buffer.end()));
                } else {
                    // 使用无符号版本
                    buffer.try_reserve(buffer.size() + 21);
                    const char *end = rapidjson::internal::u64toa(value, buffer.end());
                    buffer.resize(buffer.size() + (end - buffer.end()));
                }
            } else {
                // 非常规整数类型，使用通用方法
                auto s = std::to_string(value);
                buffer += s;
            }
        }
    };

    /**
     * @brief 布尔类型转字符串
     * @details JSON中的布尔类型用true或false表示
     */
    template<>
    struct DumpToString<bool> : public IDumpToString<DumpToString<bool>> {
        static void dump_impl(Buffer &buffer, const bool &value) SLOW_JSON_NOEXCEPT {
            buffer += value ? "true" : "false";
        }
    };


    template<concepts::reference_wrapper T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            DumpToString<typename T::type>::dump(buffer, value.get());
        }
    };


    /**
     * @brief 浮点数转字符串，不同类型有不同精度
     * @details 这里未来可以换成更加高效的浮点数转字符串的算法
     * @tparam T
     */
    template<concepts::floating_point T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            assert_with_message(value != INFINITY, "slowjson暂不支持处理浮点数的inf");
            buffer.try_reserve(buffer.size() + 20);
            char *end = rapidjson::internal::dtoa(value, buffer.end())-1;
            if (std::is_same_v<float, T> && end - buffer.end() > 8) {
                end = buffer.end() + 8;
            };
            char *begin = *buffer.end() == '-' ? buffer.end() + 1 : buffer.end();

            while(end>begin){
                if((*end=='0' && *(end-1)!='.') || *end=='\0'){
                    --end;
                }else{
                    break;
                }
            }

            buffer.resize(buffer.size() + end - buffer.end() + 1);
        }
    };

    /**
     * @brief 字符串的处理，需要加上双引号
     * @details 这里没有处理转义字符的问题，需自行处理
     * @tparam T
     */
    template<slow_json::concepts::string T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            buffer.push_back('"');
            if constexpr (concepts::static_string<T>) {
                buffer += value.with_end();
            } else {
                buffer += value;
            }
            buffer.push_back('"');
        }
    };

    /**
     * 列表的处理
     * @tparam T
     */
    template<slow_json::concepts::iterable T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            using element_type = std::remove_const_t<std::remove_reference_t<decltype(*std::begin(value))>>;
            buffer.push_back('[');
            for (const auto &it: value) {
                DumpToString<element_type>::dump(buffer, it);
                buffer.push_back(',');
            }
            if (buffer.back() == ',') {
                buffer.back() = ']';
            } else {
                buffer.push_back(']');
            }
        }
    };

    /**
     * 对于std::tuple的处理，将其作为list得到一个列表
     * @tparam T
     */
    template<slow_json::concepts::tuple T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            constexpr std::size_t size = std::tuple_size_v<T>;
            [&value, &buffer]<std::size_t...index>(std::index_sequence<index...> &&) {
                buffer.push_back('[');
                ([&]<typename U>(const U &u) {
                    DumpToString<U>::dump(buffer, u);
                    buffer.push_back(',');
                }(std::get<index>(value)), ...);
            }(std::make_index_sequence<size>());
            if (buffer.back() == ',') {
                buffer.back() = ']';
            } else {
                buffer.push_back(']');
            }
        }
    };

    /**
     * slow_json::static_dict的处理，将其作为object，得到一个大括号表示的字典
     * @tparam T
     */
    template<slow_json::concepts::slow_json_static_dict T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            constexpr std::size_t size = T::size_v;
            [&value, &buffer]<std::size_t...index>(std::index_sequence<index...> &&) {
                buffer.push_back('{');
                ([&]<typename U>(const U &u) {
                    if constexpr (slow_json::concepts::pair<U>) {
                        DumpToString<decltype(u.first)>::dump(buffer, u.first);
                        buffer.push_back(':');
                        DumpToString<decltype(u.second)>::dump(buffer, u.second);
                        buffer.push_back(',');
                    } else {
                        assert_with_message(slow_json::concepts::pair<U>, "非键值对类型:%s",
                                            type_name_v<U>.with_end().str);
                    }
                }(std::get<index>(value)), ...);
            }(std::make_index_sequence<size>());
            if (buffer.back() == ',') {
                buffer.back() = '}';
            } else {
                buffer.push_back('}');
            }
        }
    };


    template<slow_json::concepts::dict T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            buffer.push_back('{');
            for (const auto &[k, v]: value) {
                using key_type = std::remove_const_t<std::remove_reference_t<decltype(k)>>;
                using value_type = std::remove_const_t<std::remove_reference_t<decltype(v)>>;
                DumpToString<key_type>::dump(buffer, k);
                buffer.push_back(':');
                DumpToString<value_type>::dump(buffer, v);
                buffer.push_back(',');
            }
            if (buffer.back() == ',') {
                buffer.back() = '}';
            } else {
                buffer.push_back('}');
            }
        }
    };

    template<slow_json::concepts::optional T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            if constexpr (std::is_same_v<T, std::nullopt_t>) {
                buffer.append("null");
            } else {
                if (value) {
                    DumpToString<typename T::value_type>::dump(buffer, value.value());
                } else {
                    buffer.append("null");
                }
            }
        }
    };

    template<concepts::variant T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            const void*value_ptr=nullptr;
            auto do_func=[&]<std::size_t...index>(std::index_sequence<index...>){
                using variant=decltype(concepts::details::variant_traits{value});
                ([&]() {
                    // 遍历所有可能的类型
                    using maybe_type=typename variant::template maybe_types<index>;
                    if(value_ptr!=nullptr){
                        return;
                    }
                    value_ptr=std::get_if<maybe_type>(&value);
                    if(value_ptr!=nullptr){
                        DumpToString<maybe_type>::dump(buffer,*(maybe_type*)value_ptr);//只会有一个类型会成功
                    }
                }(),...);
            };
            if(value.valueless_by_exception()){
                buffer.append("null"); //空值
            }else {
                do_func(std::make_index_sequence<decltype(concepts::details::variant_traits{value})::size_v>());
            }
        }
    };

    template<slow_json::concepts::pair T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            buffer.push_back('[');
            DumpToString<typename T::first_type>::dump(buffer, value.first);
            buffer.push_back(',');
            DumpToString<typename T::second_type>::dump(buffer, value.second);
            buffer.push_back(']');
        }
    };

    template<slow_json::concepts::pointer T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                buffer.append("null");
            } else {
                if(value==nullptr){
                    buffer.append("null");
                }else {
                    const auto &data = *value;
                    using type = std::remove_const_t<std::remove_reference_t<decltype(*value)>>;
                    DumpToString<type>::dump(buffer, data);
                }
            }
        }
    };

    template<slow_json::concepts::callable T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            DumpToString<decltype(value())>::dump(buffer, value());
        };
    };



    template<>
    struct DumpToString<details::serializable_wrapper>:public IDumpToString<DumpToString<details::serializable_wrapper>>{
        static void dump_impl(Buffer&buffer,const details::serializable_wrapper&object)SLOW_JSON_NOEXCEPT{
            assert_with_message(object._dump_fn,"_fn为空，可能存在悬空引用问题");
            object.dump_fn(buffer);
        }
    };

    template<>
    struct DumpToString<details::field_wrapper>:public IDumpToString<DumpToString<details::field_wrapper>>{
        static void dump_impl(Buffer&buffer,const details::field_wrapper&object)SLOW_JSON_NOEXCEPT{
            object.dump_fn(buffer);
        }
    };

    template<>
    struct DumpToString<dict>:public IDumpToString<DumpToString<dict>>{
        static void dump_impl(Buffer&buffer,const dict&object)SLOW_JSON_NOEXCEPT{
            if(object.is_dict()) {
                buffer += '{';
                for (const auto &[k, v]: object._data) {
                    DumpToString<const char *>::dump(buffer, k);
                    buffer += ':';
                    DumpToString<details::serializable_wrapper>::dump(buffer, v);
                    buffer += ',';
                }
                if (buffer.back() == ',') {
                    buffer.back() = '}';
                } else {
                    buffer += '}';
                }
            }else if(object.is_array()){
                auto*list=static_cast<std::vector<details::serializable_wrapper>*>(object._data_ptr->value());
                buffer += '[';
                for(auto&v:*list){
                    DumpToString<details::serializable_wrapper>::dump(buffer,v);
                    buffer += ',';
                }
                if(buffer.back()==','){
                    buffer.back()=']';
                }else{
                    buffer += ']';
                }
            }else if(object.is_fundamental()){
                auto&v=*static_cast<details::serializable_wrapper*>(object._data_ptr->value());
                DumpToString<details::serializable_wrapper>::dump(buffer,v);
            }else if(object.is_null()){
                buffer+="null";
            }
        }
    };

    template<>
    struct DumpToString<details::pair>:public IDumpToString<DumpToString<details::pair>>{
        static void dump_impl(Buffer&buffer,const details::pair&object)SLOW_JSON_NOEXCEPT{
            DumpToString<const char*>::dump(buffer,object._key);
            buffer+=':';
            DumpToString<details::serializable_wrapper>::dump(buffer,object._value);
        }
    };


    /**
     * 可序列化的类的处理
     * @tparam T
     */
    template<slow_json::concepts::serializable T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            if constexpr(std::is_same_v<decltype(T::get_config()),slow_json::dict>){
                slow_json::dict config=T::get_config();
                for(const auto&it:config._data){
                    if(auto value_ptr=static_cast<const details::field_wrapper*>(it._value.value());value_ptr!=nullptr){
                        value_ptr->_object_ptr=&value;
                    }
                };
                DumpToString<decltype(config)>::dump(buffer,config);
            }
            else {
                auto get_value = []<typename Tp>(const Tp &tp) {
                    if constexpr (concepts::slow_json_static_dict<Tp>) {
                        return static_cast<typename Tp::super_type>(tp);
                    } else {
                        return tp;
                    }
                };
                // 先得到对象属性信息，包括属性名称和指针
                auto config = get_value(T::get_config());
                auto print = [&value, &buffer](const auto &field) {
                    const auto &[field_name, field_ptr] = field;
                    using field_type = std::remove_const_t<std::remove_reference_t<decltype(value.*field_ptr)>>;
                    DumpToString<std::remove_const_t<decltype(field_name.with_end())>>::dump(buffer,
                                                                                             field_name.with_end());
                    buffer.push_back(':');
                    //根据对象和对象属性指针获得对象属性值，然后将其转换为JSON
                    DumpToString<field_type>::dump(buffer, value.*field_ptr);
                    buffer.push_back(',');
                };
                if constexpr (concepts::pair<std::tuple_element_t<0, decltype(config)>>) {
                    constexpr std::size_t size = std::tuple_size_v<decltype(config)>;
                    buffer.push_back('{');
                    [&config, &print]<std::size_t...index>(std::index_sequence<index...> &&) {
                        (print(std::get<index>(config)), ...);
                    }(std::make_index_sequence<size>());
                    if (buffer.back() == ',') {
                        buffer.back() = '}';
                    } else {
                        buffer.push_back('}');
                    }
                } else {
                    //std::tuple{std::pair{'a','b'}} 会变为 std::tuple{'a','b'}，所以这里做个简单的处理
                    buffer.push_back('{');
                    print(config);
                    if (buffer.back() == ',') {
                        buffer.back() = '}';
                    } else {
                        buffer.push_back('}');
                    }
                }
            }
        }
    };

    template<slow_json::concepts::cv_point T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            buffer.push_back('[');
            DumpToString<decltype(value.x)>::dump(buffer, value.x);
            buffer.push_back(',');
            DumpToString<decltype(value.y)>::dump(buffer, value.y);
            buffer.push_back(']');
        }
    };

    template<slow_json::concepts::eigen_point T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            buffer.push_back('[');
            DumpToString<decltype(value.x())>::dump(buffer, value.x());
            buffer.push_back(',');
            DumpToString<decltype(value.y())>::dump(buffer, value.y());
            buffer.push_back(']');
        }
    };

    template<slow_json::concepts::enumerate T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) {
            // 在转化为JSON的时候，必须以带双引号的字符串来进行表示，否则会导致JSON解析错误
            buffer.push_back('"');
            buffer += slow_json::enum2string(value);
            buffer.push_back('"');
        }
    };

    /**
     * 继承了ISerializable的类，可根据from_config返回一个可以直接转换为JSON的slow_json::dict对象
     * @tparam T
     */
    template<concepts::serializable_oop T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) SLOW_JSON_NOEXCEPT {
            DumpToString<decltype(value.get_config())>::dump(buffer, value.get_config());
        }
    };

};

#endif //SLOWJSON_DUMP_TO_STRING_HPP
