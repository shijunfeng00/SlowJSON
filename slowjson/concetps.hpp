//
// Created by hy-20 on 2024/7/13.
//

#ifndef SLOWJSON_CONCETPS_HPP
#define SLOWJSON_CONCETPS_HPP

#include<type_traits>
#include<optional>
#include<memory>
#include "static_dict.hpp"

namespace slow_json::concepts {

    namespace helper {
        template<typename...Args>
        auto match_tuple(std::tuple<Args...> &) {}

        template<typename...Args>
        auto match_static_dict(slow_json::static_dict<Args...> &) {}

        template<typename T>
        auto match_optional(std::optional<T> &) {}

        template<typename T>
        auto match_reference_wrapper(const std::reference_wrapper<T> &) {}

        template<typename T>
        void match_shared_ptr(const std::shared_ptr<T> &) {}

        template<typename T>
        void match_unique_ptr(const std::unique_ptr<T> &) {}

        template<typename T>
        void match_weak_ptr(const std::weak_ptr<T> &) {}

        template<typename FieldType, typename ClassType>
        auto match_field_type(FieldType ClassType::*) -> FieldType;

        template<typename FieldType, typename ClassType, std::size_t N>
        auto match_field_type(FieldType (ClassType::*)[N]) -> FieldType(&)[N];

        template<typename FieldType, typename ClassType>
        auto match_class_type(FieldType ClassType::*) -> ClassType;

        template<char...chs>
        void match_static_string(slow_json::StaticString<chs...> &) {}

        template<typename T, std::size_t N>
        std::size_t get_array_size(const T(&arr)[N]) {
            return N;
        }

        template<typename T, std::size_t N>
        std::size_t get_array_size(const std::array<T, N> &) {
            return N;
        }
    }

    /**
     * 类型T是否是Args中的一个
     * @tparam T
     * @tparam Args
     */
    template<typename T, typename... Args>
    concept contains = std::disjunction_v<std::is_same<T, Args>...>;

    template<typename T, typename...Args>
    constexpr bool is_contains_v = contains<T, Args...>;

    template<typename T>
    concept array=requires(T &&t){
        helper::get_array_size(t);
    };

    /**
     * 整数类型约束
     * @tparam T
     */
    template<typename T>
    concept integer=std::is_integral_v<T> && !std::is_same_v<T, bool>;
    /**
     * 浮点数类型约束
     * @tparam T
     */
    template<typename T>
    concept floating_point=std::is_floating_point_v<T>;

    template<typename T>
    concept fundamental=std::is_fundamental_v<T>;

    template<typename T>
    concept string=(
                           contains<std::decay_t<T>, std::string, std::string_view, char *, const char *> ||
                           std::is_convertible_v<T, const char *>) && !std::is_same_v<T, std::nullptr_t>;

    template<typename T>
    concept static_string=requires(T t){
        helper::match_static_string(t);
    };

    template<typename T>
    concept reference_wrapper=requires(T t){
        helper::match_reference_wrapper(t);
    };

    template<typename T>
    concept dict=requires(T a){
        *std::begin(a);
        *std::end(a);
        requires !string<T>;
        typename T::mapped_type;
    };

    template<typename T>
    concept set=requires(T a){
        *std::begin(a);
        *std::end(a);
        a.insert(std::declval<typename T::key_type>());
    };

    /**
     * 是否是可迭代的类型
     * @tparam T
     */
    template<typename T>
    concept iterable=requires(const T &a){
        *std::begin(a);
        *std::end(a);
        requires !string<T>;
        requires !dict<T>;
    };

    template<typename T>
    concept tuple=requires(T t){
        helper::match_tuple(t);
        std::tuple_size<T>::value;
    };

    template<typename T>
    concept slow_json_static_dict=requires(T t){
        helper::match_static_dict(t);
    };


    template<typename T>
    concept pair=requires(T t){
        t.first;
        t.second;
        std::get<0>(t);
    };

    template<typename T>
    concept optional=requires(T t){
        helper::match_optional(t);
    } || std::is_same_v<T, std::nullopt_t>;

    template<typename T>
    concept pointer=(std::is_pointer_v<T> ||
                     std::is_same_v<T, std::nullptr_t> || requires(T t){ helper::match_shared_ptr(t); } ||
                     requires(T t){ helper::match_unique_ptr(t); } || requires(T t){ helper::match_weak_ptr(t); }) &&
                    !string<T>;

    template<typename T>
    concept serializable=requires(T t){
        T::get_config();
    };

    template<typename T>
    concept cv_point=requires(T t){
        t.x;
        t.y;
    } && !serializable<T>;

    template<typename T>
    concept eigen_point=requires(T t){
        t.x();
        t.y();
    } && !serializable<T>;
}
#endif //SLOWJSON_CONCETPS_HPP
