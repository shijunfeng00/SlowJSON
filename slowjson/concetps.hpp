/**
 * @author shijunfeng
 * @date 2024/7/13
 * @details 用来实现模板偏特化的各种概念
 */

#ifndef SLOWJSON_CONCETPS_HPP
#define SLOWJSON_CONCETPS_HPP

#include<type_traits>
#include<optional>
#include<memory>
#include<variant>
#include "static_string.hpp"

namespace slow_json {
    template<typename...Args>
    struct static_dict;

    class dynamic_dict;

    struct ISerializable;
}
namespace slow_json::concepts {
    namespace helper {

        /**
         * 匹配std::variant的辅助类，顺便获得模板参数列表，用于序列化
         * @tparam Args std::variant<Args...>，用于类型匹配
         */
        template<typename...Args>
        struct variant_traits{
            template<std::size_t i>
            using maybe_types=std::tuple_element_t<i,std::tuple<Args...>>;
            constexpr static std::size_t size_v=std::tuple_size_v<std::tuple<Args...>>;
            explicit variant_traits(const std::variant<Args...>&value):_value(value){}
            const std::variant<Args...>&_value;
        };

        /**
         * 匹配std::tuple的辅助函数，无实际调用意义
         * @tparam Args
         * @return
         */
        template<typename...Args>
        auto match_tuple(std::tuple<Args...> &) {}

        /**
         * 匹配slow_json::static_dict的辅助函数，无实际调用意义
         * @tparam Args
         * @return
         */
        template<typename...Args>
        auto match_static_dict(const static_dict<Args...> &) {}

        /**
         * 匹配std::optional的辅助函数，无实际调用意义
         * @tparam Args
         * @return
         */
        template<typename T>
        auto match_optional(std::optional<T> &) {}

        /**
         * 匹配std::reference_wrapper的辅助函数，无实际调用意义
         * @tparam Args
         * @return
         */
        template<typename T>
        auto match_reference_wrapper(const std::reference_wrapper<T> &) {}

        template<typename T>
        void match_shared_ptr(const std::shared_ptr<T> &) {}

        template<typename T>
        void match_unique_ptr(const std::unique_ptr<T> &) {}

        template<typename T>
        void match_weak_ptr(const std::weak_ptr<T> &) {}

        template<typename T>
        void match_atomic(const std::atomic<T> &) {}

        /**
         * 根据成员属性指针类型萃取class类型，用于类型推断，不需要具体实现
         * @tparam FieldType 成员属性指针
         * @tparam ClassType 成员属性所属的class的类型
         * @return
         */
        template<typename FieldType, typename ClassType>
        auto match_field_type(FieldType ClassType::*) -> FieldType;

        /**
         * 根据成员属性指针类型萃取class类型，用于类型推断，针对于 int a[3]这样的C指针类型的属性，不需要具体实现
         * @tparam FieldType
         * @tparam ClassType
         * @tparam N
         * @return
         */
        template<typename FieldType, typename ClassType, std::size_t N>
        auto match_field_type(FieldType (ClassType::*)[N]) -> FieldType(&)[N];

        /**
         * 根据成员属性指针类型萃取成员属性类型，用于类型推断，不需要具体实现
         * @tparam FieldType 成员属性指针
         * @tparam ClassType 成员属性所属的类型
         * @return
         */
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
     * 类型T是否是一个形参为void且返回值非空的可调用函数或仿函数
     * @tparam T
     */
    template<typename T>
    concept callable=!std::is_void_v<decltype(std::declval<T>()())>;

    /**
     * 类型T是否是一个枚举形变量
     * @tparam T
     */
    template<typename T>
    concept enumerate=std::is_enum_v<T>;

    /**
     * 类型T是否是Args中的一个
     * @tparam T 被判断的类型
     * @tparam Args 类型列表
     */
    template<typename T, typename... Args>
    concept contains = std::disjunction_v<std::is_same<T, Args>...>;

    /**
     * 有的场景用contains会编译失败，用is_contains_v才可以，原因未知，可能是gcc版本过低（9.4），对于C++20特性支持不完善
     * @tparam T
     * @tparam Args
     */
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

    /**
     * C语言基本类型
     * @tparam T
     */
    template<typename T>
    concept fundamental=std::is_fundamental_v<T>;

    /**
     * 字符串类型，std::string,std::string_view,char*,const char*或者实现了operator const char*()的对象
     * @tparam T
     */
    template<typename T>
    concept string=(
                           contains<std::decay_t<T>, std::string, std::string_view, char *, const char *> ||
                           std::is_convertible_v<T, const char *>) && !std::is_same_v<T, std::nullptr_t>;

    /**
     * 类型T是否是slow_json::static_string
     * @tparam T
     */
    template<typename T>
    concept static_string=requires(T t){
        helper::match_static_string(t);
    };

    /**
     * 类型T是否是std::reference_wrapper<U>
     * @tparam T
     */
    template<typename T>
    concept reference_wrapper=requires(T t){
        helper::match_reference_wrapper(t);
    };

    /**
     * 字典类型，例如std::unordered_map，std::map
     * @tparam T
     */
    template<typename T>
    concept dict=requires(T a){
        *std::begin(a);
        *std::end(a);
        requires !string<T>;
        typename T::mapped_type;
    };

    /**
     * 集合类型，例如std::unordered_set，std::set
     * @tparam T
     */
    template<typename T>
    concept set=requires(T a){
        *std::begin(a);
        *std::end(a);
        a.insert(std::declval<typename T::key_type>());
    };

    /**
     * 是否是可迭代的类型，例如std::vector，std::list
     * @tparam T
     */
    template<typename T>
    concept iterable=requires(const T &a){
        *std::begin(a);
        *std::end(a);
        requires !string<T>;
        requires !dict<T>;
    };

    /**
     * std::tuple<Args...>类型
     * @tparam T
     */
    template<typename T>
    concept tuple=requires(T t){
        helper::match_tuple(t);
        std::tuple_size<T>::value;
    };

    /**
     * slow_json::static_dict<Args...>类型
     * @tparam T
     */
    template<typename T>
    concept slow_json_static_dict=requires(T t){
        helper::match_static_dict(t);
    };

    /**
     * std::pair<K,V>类型
     * @tparam T
     */
    template<typename T>
    concept pair=requires(T t){
        t.first;
        t.second;
        std::get<0>(t);
    };

    /**
     * std::optional类型
     * @tparam T
     */
    template<typename T>
    concept optional=requires(T t){
        helper::match_optional(t);
    } || std::is_same_v<T, std::nullopt_t>;

    /**
     * 是否是std::variant<Args...>类型
     * @tparam T
     */
    template<typename T>
    concept variant=requires(T t){helper::variant_traits{t};};

    /**
     * 指针类型，包括C指针和三种C++智能指针
     * @tparam T
     */
    template<typename T>
    concept pointer=(std::is_pointer_v<T> ||
                     std::is_same_v<T, std::nullptr_t> || requires(T t){ helper::match_shared_ptr(t); } ||
                     requires(T t){ helper::match_unique_ptr(t); } || requires(T t){ helper::match_weak_ptr(t); }) &&
                    !string<T>;

    template<typename T>
    concept atomic=requires(T t){ helper::match_atomic(t);};

    /**
     * 支持序列化的类型，通常为用户自定义类型，且正确实现了get_config方法
     * @tparam T
     */
    template<typename T>
    concept serializable=requires(T t){
        T::get_config();
    };

    /**
     * 类型T是否是一个ISerializable子类
     * @tparam T
     */
    template<typename T>
    concept serializable_oop=std::is_base_of_v<ISerializable, T>;

    /**
     * opencv的cv::Point2f,cv::Point2i等点类型
     * @tparam T
     */
    template<typename T>
    concept cv_point=requires(T t){ t.x;t.y; } && !serializable<T> &&
                     (std::is_same_v<decltype(T::x), decltype(T::y)>) &&
                     (sizeof(T) == sizeof(T::x) + sizeof(T::y));

    /**
     * eigen中的点类型，类似opencv的情况
     * @tparam T
     */
    template<typename T>
    concept eigen_point=requires(T t){
        t.x();
        t.y();
    } && !serializable<T>;
}
#endif //SLOWJSON_CONCETPS_HPP
