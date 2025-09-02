/**
 * @file concepts.hpp
 * @author shijunfeng
 * @date 2024/7/13
 * @brief 提供 SlowJSON 内部使用的 C++20 Concepts，用于模板约束和偏特化
 *
 * @details
 * 本文件定义了大量概念（concepts），用于序列化框架的 SFINAE / Concepts 检查。
 * 所有 concept 均排除了 `slow_json::details::dict`，以避免 dict 类型被错误匹配，
 * 从而导致隐式转换过宽的问题。
 */

#ifndef SLOWJSON_CONCETPS_HPP
#define SLOWJSON_CONCETPS_HPP

#include <type_traits>
#include <optional>
#include <memory>
#include <variant>
#include <atomic>
#include "static_string.hpp"

namespace slow_json {
    template<typename...Args>
    struct static_dict;

    class dynamic_dict;

    struct ISerializable;

    namespace details {
        struct dict; ///< SlowJSON 内部的动态字典类型
    };
}

namespace slow_json::concepts {
    namespace details {

        /**
         * @brief std::variant 的 traits，用于萃取参数类型
         * @tparam Args std::variant 的参数类型
         */
        template<typename...Args>
        struct variant_traits {
            /// @brief 获取第 i 个参数类型
            template<std::size_t i>
            using maybe_types = std::tuple_element_t<i, std::tuple<Args...>>;
            /// @brief 参数个数
            constexpr static std::size_t size_v = std::tuple_size_v<std::tuple<Args...>>;
            explicit variant_traits(const std::variant<Args...> &value) : _value(value) {}
            const std::variant<Args...> &_value;
        };

        /// 匹配 std::tuple
        template<typename...Args>
        auto match_tuple(std::tuple<Args...> &) {}

        /// 匹配 slow_json::static_dict
        template<typename...Args>
        auto match_static_dict(const static_dict<Args...> &) {}

        /// 匹配 std::optional
        template<typename T>
        auto match_optional(std::optional<T> &) {}

        /// 匹配 std::reference_wrapper
        template<typename T>
        auto match_reference_wrapper(const std::reference_wrapper<T> &) {}

        /// 匹配 std::shared_ptr
        template<typename T>
        void match_shared_ptr(const std::shared_ptr<T> &) {}

        /// 匹配 std::unique_ptr
        template<typename T>
        void match_unique_ptr(const std::unique_ptr<T> &) {}

        /// 匹配 std::weak_ptr
        template<typename T>
        void match_weak_ptr(const std::weak_ptr<T> &) {}

        /// 匹配 std::atomic
        template<typename T>
        void match_atomic(const std::atomic<T> &) {}

        /// 萃取成员字段类型
        template<typename FieldType, typename ClassType>
        auto match_field_type(FieldType ClassType::*) -> FieldType;

        /// 萃取数组成员字段类型
        template<typename FieldType, typename ClassType, std::size_t N>
        auto match_field_type(FieldType (ClassType::*)[N]) -> FieldType(&)[N];

        /// 萃取成员所属类
        template<typename FieldType, typename ClassType>
        auto match_class_type(FieldType ClassType::*) -> ClassType;

        /// 匹配 static_string
        template<char...chs>
        void match_static_string(slow_json::StaticString<chs...> &) {}

        /// 获取原生数组大小
        template<typename T, std::size_t N>
        std::size_t get_array_size(const T(&arr)[N]) { return N; }

        /// 获取 std::array 大小
        template<typename T, std::size_t N>
        std::size_t get_array_size(const std::array<T, N> &) { return N; }
    }

    /**
     * @brief 可调用对象，形参为 void 且返回值非 void
     */
    template<typename T>
    concept callable = !std::is_void_v<decltype(std::declval<T>()())>;

    /**
     * @brief 枚举类型
     */
    template<typename T>
    concept enumerate = std::is_enum_v<T>;

    /**
     * @brief 判断类型是否在 Args 中
     */
    template<typename T, typename... Args>
    concept contains = std::disjunction_v<std::is_same<T, Args>...>;

    /// 辅助版本，部分 GCC 版本下 contains 会报错
    template<typename T, typename...Args>
    constexpr bool is_contains_v = contains<T, Args...>;

    /**
     * @brief 数组类型，支持原生数组和 std::array
     */
    template<typename T>
    concept array = requires(T &&t) {
        details::get_array_size(t);
    };

    /**
     * @brief 整数类型（排除 bool）
     */
    template<typename T>
    concept integer = std::is_integral_v<T> && !std::is_same_v<T, bool>;

    /**
     * @brief 浮点数类型
     */
    template<typename T>
    concept floating_point = std::is_floating_point_v<T>;

    /**
     * @brief 基本类型（C++ fundamental types）
     */
    template<typename T>
    concept fundamental = std::is_fundamental_v<T>;

    /**
     * @brief 字符串类型（排除 dict）
     *
     * 支持 std::string, std::string_view, char*, const char*，以及能隐式转换为 const char* 的类型
     */
    template<typename T>
    concept string =
    !std::is_same_v<std::decay_t<T>, slow_json::details::dict> &&
    (
            contains<std::decay_t<T>, std::string, std::string_view, char *, const char *> ||
            std::is_convertible_v<T, const char *>
    ) &&
    !std::is_same_v<T, std::nullptr_t>;

    /**
     * @brief slow_json::StaticString 类型
     */
    template<typename T>
    concept static_string = requires(T t) {
        details::match_static_string(t);
    };

    /**
     * @brief std::reference_wrapper<T>
     */
    template<typename T>
    concept reference_wrapper = requires(T t) {
        details::match_reference_wrapper(t);
    };

    /**
     * @brief STL 字典类型（map/unordered_map），排除 slow_json::details::dict
     */
    template<typename T>
    concept dict =
    !std::is_same_v<std::decay_t<T>, slow_json::details::dict> &&
    requires(T a) {
        *std::begin(a);
        *std::end(a);
        requires !string<T>;
        typename T::mapped_type;
    };

    /**
     * @brief 集合类型（set/unordered_set）
     */
    template<typename T>
    concept set = requires(T a) {
        *std::begin(a);
        *std::end(a);
        a.insert(std::declval<typename T::key_type>());
    };

    /**
     * @brief 可迭代类型（vector/list 等），排除 string 和 dict
     */
    template<typename T>
    concept iterable =
    !std::is_same_v<std::decay_t<T>, slow_json::details::dict> &&
    requires(const T &a) {
        *std::begin(a);
        *std::end(a);
        requires !string<T>;
        requires !dict<T>;
    };

    /**
     * @brief std::tuple 类型
     */
    template<typename T>
    concept tuple = requires(T t) {
        details::match_tuple(t);
        std::tuple_size<T>::value;
    };

    /**
     * @brief slow_json::static_dict 类型
     */
    template<typename T>
    concept slow_json_static_dict = requires(T t) {
        details::match_static_dict(t);
    };

    /**
     * @brief std::pair<K,V> 类型
     */
    template<typename T>
    concept pair = requires(T t) {
        t.first;
        t.second;
        std::get<0>(t);
    };

    /**
     * @brief std::optional<T> 或 std::nullopt_t
     */
    template<typename T>
    concept optional =
    requires(T t) { details::match_optional(t); } ||
    std::is_same_v<T, std::nullopt_t>;

    /**
     * @brief std::variant 类型
     */
    template<typename T>
    concept variant = requires(T t) { details::variant_traits{t}; };

    /**
     * @brief 指针类型（含智能指针），排除 string 和 dict
     */
    template<typename T>
    concept pointer =
    !std::is_same_v<std::decay_t<T>, slow_json::details::dict> &&
    (
            std::is_pointer_v<T> ||
            std::is_same_v<T, std::nullptr_t> ||
            requires(T t) { details::match_shared_ptr(t); } ||
            requires(T t) { details::match_unique_ptr(t); } ||
            requires(T t) { details::match_weak_ptr(t); }
    ) &&
    !string<T>;

    /**
     * @brief std::atomic 类型
     */
    template<typename T>
    concept atomic = requires(T t) { details::match_atomic(t); };

    /**
     * @brief 可序列化类型（需实现静态 get_config 方法）
     */
    template<typename T>
    concept serializable = requires(T t) {
        T::get_config();
    };

    /**
     * @brief ISerializable 的子类
     */
    template<typename T>
    concept serializable_oop = std::is_base_of_v<ISerializable, T>;

    /**
     * @brief OpenCV Point 类型（cv::Point2f, cv::Point2i 等），排除 dict
     */
    template<typename T>
    concept cv_point =
    !std::is_same_v<std::decay_t<T>, slow_json::details::dict> &&
    requires(T t) { t.x; t.y; } &&
    !serializable<T> &&
    (std::is_same_v<decltype(T::x), decltype(T::y)>) &&
    (sizeof(T) == sizeof(T::x) + sizeof(T::y));

    /**
     * @brief Eigen 点类型，排除 dict
     */
    template<typename T>
    concept eigen_point =
    !std::is_same_v<std::decay_t<T>, slow_json::details::dict> &&
    requires(T t) { t.x(); t.y(); } &&
    !serializable<T>;
}

#endif //SLOWJSON_CONCETPS_HPP
