//
// Created by hy-20 on 2024/7/19.
//

#ifndef SLOWJSON_DUMP_TO_STRING_HPP
#define SLOWJSON_DUMP_TO_STRING_HPP

#include "dump_to_string_interface.hpp"
#include "concetps.hpp"
#include "polymorphic_dict.hpp"
#include "enum.hpp"

namespace slow_json {

    /**
     * @brief 整数类型转字符串
     * @details 后面再来优化，现在先用STL的工具
     * @tparam T
     */
    template<concepts::integer T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
            if constexpr (std::is_same_v<T, char>) {
                buffer += value;
            } else {
                buffer += std::to_string(value);
            }
        }
    };

    /**
     * @brief 布尔类型转字符串
     * @details JSON中的布尔类型用true或false表示
     */
    template<>
    struct DumpToString<bool> : public IDumpToString<DumpToString<bool>> {
        static void dump_impl(Buffer &buffer, const bool &value) noexcept {
            buffer += value ? "true" : "false";
        }
    };


    template<concepts::reference_wrapper T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
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
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
            if constexpr (std::is_same_v<float, T>) {
                buffer.try_reserve(buffer.size() + 8);
                sprintf(buffer.end(), "%.7f", value);
                char *pos = buffer.end() + 7;
                while (--pos != buffer.end() && (*pos == '0' || *pos == '.'));
                buffer.resize(buffer.size() + pos - buffer.end() + 1);
            } else if constexpr (std::is_same_v<double, T>) {
                buffer.try_reserve(buffer.size() + 16);
                sprintf(buffer.end(), "%.15f", value);
                char *pos = buffer.end() + 15;
                while (--pos != buffer.end() && (*pos == '0' || *pos == '.'));
                buffer.resize(buffer.size() + pos - buffer.end() + 1);
            } else if constexpr (std::is_same_v<long double, T>) {
                buffer.try_reserve(buffer.size() + 19);
                sprintf(buffer.end(), "%.18f", value);
                char *pos = buffer.end() + 18;
                while (--pos != buffer.end() && (*pos == '0' || *pos == '.'));
                buffer.resize(buffer.size() + pos - buffer.end() + 1);
            } else {
                buffer += std::to_string(value);
            }
        }
    };

    /**
     * @brief 字符串的处理，需要加上双引号
     * @details 这里没有处理转义字符的问题，需自行处理
     * @tparam T
     */
    template<slow_json::concepts::string T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
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
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
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

    template<slow_json::concepts::tuple T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
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

    template<slow_json::concepts::slow_json_static_dict T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
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
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
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
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
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

    template<slow_json::concepts::pair T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
            buffer.push_back('[');
            DumpToString<typename T::first_type>::dump(buffer, value.first);
            buffer.push_back(',');
            DumpToString<typename T::second_type>::dump(buffer, value.second);
            buffer.push_back(']');
        }
    };

    template<slow_json::concepts::pointer T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                buffer.append("null");
            } else {
                const auto &data = *value;
                using type = std::remove_const_t<std::remove_reference_t<decltype(*value)>>;
                DumpToString<type>::dump(buffer, data);
            }
        }
    };

    template<slow_json::concepts::serializable T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
            if constexpr (std::is_same_v<decltype(T::get_config()), slow_json::polymorphic_dict>) {
                auto config = T::get_config();
                config.set_object(value);
                DumpToString<decltype(config)>::dump(buffer, config);
            } else {
                auto get_value = []<typename Tp>(const Tp &tp) {
                    if constexpr (concepts::slow_json_static_dict<Tp>) {
                        return static_cast<typename Tp::super_type>(tp);
                    } else {
                        return tp;
                    }
                };
                auto config = get_value(T::get_config());
                auto print = [&value, &buffer](const auto &field) {
                    const auto &[field_name, field_ptr] = field;
                    using field_type = std::remove_const_t<std::remove_reference_t<decltype(value.*field_ptr)>>;
                    DumpToString<std::remove_const_t<decltype(field_name.with_end())>>::dump(buffer,
                                                                                             field_name.with_end());
                    buffer.push_back(':');
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
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
            buffer.push_back('[');
            DumpToString<decltype(value.x)>::dump(buffer, value.x);
            buffer.push_back(',');
            DumpToString<decltype(value.y)>::dump(buffer, value.y);
            buffer.push_back(']');
        }
    };

    template<slow_json::concepts::eigen_point T>
    struct DumpToString<T> : public IDumpToString<DumpToString<T>> {
        static void dump_impl(Buffer &buffer, const T &value) noexcept {
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
            buffer += slow_json::enum2string(value);
        }
    };
};

#endif //SLOWJSON_DUMP_TO_STRING_HPP
