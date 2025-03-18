/**
 * @file dict.hpp
 * @brief 实现动态字典结构及相关序列化功能
 * @author hyzh
 * @date 2025/3/4
 */

#ifndef SLOWJSON_DICT_HPP
#define SLOWJSON_DICT_HPP

#include "dump_to_string_interface.hpp"
#include "load_from_dict_interface.hpp"
#include "static_dict.hpp"
#include "dynamic_dict.hpp"
#include <functional>
#include <vector>
#include <variant>
#include <memory>

namespace slow_json {

    namespace helper {
        /**
         * @brief 字段包装器，用于处理类成员指针的序列化/反序列化
         * @details 通过捕获成员指针信息，实现运行时动态访问类成员
         */
        struct field_wrapper {
            friend struct DumpToString<field_wrapper>;
            friend struct LoadFromDict<field_wrapper>;

        public:
            /**
             * @brief 构造函数，绑定类成员指针
             * @tparam Class 所属类类型
             * @tparam Field 字段类型
             * @param field_ptr 指向类成员的指针
             * @details 通过lambda捕获成员指针，生成序列化和反序列化的闭包
             */
            template<typename Class, typename Field>
            constexpr field_wrapper(Field Class::*field_ptr)
            : _dump_fn([=](slow_json::Buffer& buffer, const void* object_ptr) {
                // 将void指针转换为具体类指针
                const auto& object = *(Class*)object_ptr;
                // 获取成员引用（添加std::ref是为了处理原生数组的特殊情况）
                const Field& field = std::ref(object.*field_ptr);
                // 调用类型特化的序列化方法
                DumpToString<Field>::dump(buffer, field);
            }),
            _load_fn([=](const void* object_ptr, const slow_json::dynamic_dict& dict) {
                // 转换指针并获取成员引用
                auto &object = *(Class *) object_ptr;
                Field &field = std::ref(object.*field_ptr);
                // 调用类型特化的反序列化方法
                LoadFromDict<Field>::load(field, dict);
            }),
            _object_ptr{nullptr} {}

            /**
             * @brief 执行字段序列化
             * @param buffer 输出缓冲区
             * @throws 当_object_ptr为空时触发断言
             */
            void dump_fn(slow_json::Buffer& buffer) const {
                assert_with_message(this->_object_ptr != nullptr, "this->_object_ptr为空");
                this->_dump_fn(buffer, this->_object_ptr);
            }

            /**
             * @brief 执行字段反序列化
             * @param dict 输入字典
             * @throws 当_object_ptr为空时触发断言
             */
            void load_fn(const slow_json::dynamic_dict& dict) const {
                assert_with_message(this->_object_ptr != nullptr, "this->_object_ptr为空");
                this->_load_fn(this->_object_ptr, dict);
            }

        private:
            std::function<void(slow_json::Buffer&, const void*)> _dump_fn; ///< 序列化函数闭包
            std::function<void(const void*, const slow_json::dynamic_dict&)> _load_fn; ///< 反序列化函数闭包
            const void* _object_ptr; ///< 绑定的对象指针
        };

        /**
         * @brief 可序列化值包装器
         * @details 封装任意可序列化值，提供统一的dump接口
         */
        struct serializable_wrapper {
        public:
            struct function{
                template<typename T>
                constexpr void dump_fn(const slow_json::Buffer &buffer){
                    DumpToString<T>::dump(buffer,*(T*)value);
                }
                void*value=nullptr;
            };
            /**
             * @brief 构造函数，绑定任意可序列化值
             * @tparam T 值类型，需满足可序列化要求且非成员指针
             * @param value 要包装的值
             * @details 通过lambda捕获值副本，生成序列化闭包
             */
            template<typename T>
            requires(!std::is_member_pointer_v<T>)
            constexpr serializable_wrapper(const T& value)
                    : dump_fn([value](slow_json::Buffer& buffer) {
                DumpToString<T>::dump(buffer, value);
            }) {}

            std::function<void(slow_json::Buffer&)> dump_fn; ///< 序列化函数闭包
        };
    }

    /**
     * @brief JSON键值对结构
     * @details 支持多种值类型：
     * - 基本类型：通过serializable_wrapper包装
     * - 类成员指针：通过field_wrapper包装
     * - 列表：std::vector或std::initializer_list包装的基本类型
     * - 嵌套字典：std::vector<pair>或std::initializer_list<pair>
     */
    struct pair {
        /**
         * @brief 构造基本类型键值对
         * @param key JSON键
         * @param value 基本类型值
         */
        constexpr pair(const char* key, const helper::serializable_wrapper& value)
                : _key{key}, _value{value} {}

        /**
         * @brief 构造基本类型键值对
         * @tparam Key slow_json::static_string类型，兼容以前的接口
         * @param key JSON键
         * @param value 基本类型值
         */
        template<concepts::static_string Key>
        constexpr pair(const Key&key, const helper::serializable_wrapper& value)
                : _key{Key::with_end()}, _value{value} {}

        /**
         * @brief 构造列表类型键值对
         * @param key JSON键
         * @param value 已构建的vector列表
         */
        constexpr pair(const char* key, const std::vector<helper::serializable_wrapper>& value)
                : _key{key}, _value{value} {}

        /**
         * @brief 构造列表类型键值对
         * @tparam Key slow_json::static_string类型，兼容以前的接口
         * @param key JSON键
         * @param value 已构建的vector列表
         */
        template<concepts::static_string Key>
        constexpr pair(const Key&key, const std::vector<helper::serializable_wrapper>& value)
                : _key{Key::with_end()}, _value{value} {}

        /**
         * @brief 构造嵌套字典键值对
         * @param key JSON键
         * @param value 字典vector
         */
        pair(const char* key, const std::vector<pair>& value)
                : _key{key}, _value{value} {}

        /**
         * @brief 构造嵌套字典键值对
         * @tparam Key slow_json::static_string类型，兼容以前的接口
         * @param key JSON键
         * @param value 字典vector
         */
        template<concepts::static_string Key>
        constexpr pair(const Key&key, const std::vector<pair>& value)
                : _key{Key::with_end()}, _value{value} {}

        /**
         * @brief 构造类成员指针键值对
         * @param key JSON键
         * @param value 类成员指针包装器
         */
        constexpr pair(const char* key, const helper::field_wrapper& value)
                : _key{key}, _value{value} {}

        /**
         * @brief 构造类成员指针键值对
         * @tparam Key slow_json::static_string类型，兼容以前的接口
         * @param key JSON键
         * @param value 类成员指针包装器
         */
        template<concepts::static_string Key>
        constexpr pair(const Key&key, const helper::field_wrapper& value)
                : _key{Key::with_end()}, _value{value} {}

        const char* _key; ///< JSON键字符串
        using value_t = std::variant<  ///< 值类型的variant定义
                helper::serializable_wrapper,   // 基本类型
                helper::field_wrapper,          // 类成员指针
                std::vector<helper::serializable_wrapper>,   // 显式列表
                std::initializer_list<helper::serializable_wrapper>, // 初始化列表
                std::vector<pair>,              // 显式嵌套字典
                std::initializer_list<pair>     // 初始化列表嵌套字典
        >;
        value_t _value; ///< 存储的值

        /**
         * @brief 获取键值
         * @return const char* 键字符串
         */
        const char* key() const noexcept { return this->_key; }

        /**
         * @brief 获取值引用
         * @return const value_t& 值variant的常量引用
         */
        const value_t& value() const noexcept { return _value; }

        /**
         * @brief 类型安全的取值方法
         * @tparam T 期望的类型
         * @return const void* 类型指针或nullptr
         */
        template<typename T>
        const void* get_if() const noexcept {
            return std::get_if<T>(&this->_value);
        }
    };

    using list = std::initializer_list<helper::serializable_wrapper>; ///< JSON列表类型别名

    /**
     * @brief 动态字典类
     * @details 提供灵活运行时构建能力，支持：
     * - 嵌套字典和列表
     * - 混合类型值存储
     * - 类成员指针绑定
     * @note 性能略低于static_dict，适合需要动态构建的场景
     */
    struct dict {

        dict(){};

        /**
         * 从std::vector<std::pair<const char*,V>>中构造
         * 这是为了兼容之前的接口
         * @tparam V
         * @param args
         */
        template<typename...K,typename...V>
        requires (( concepts::string<K> && !concepts::static_string<K>) && ...)
        dict(const std::pair<K,V>&...args):dict(std::vector<pair>{pair{args.first,args.second}...}){}

        /**
         * 从std::vector<std::pair<static_string,V>>中构造
         * 这是为了兼容之前的接口
         * @tparam K
         * @tparam V
         */
        template<typename ...K,typename...V>
        requires (concepts::static_string<K> && ...)
        dict(const std::pair<K,V>&...args):dict(std::vector<pair>{pair{K::with_end(),args.second}...}){}

        /**
         * @brief 从pair vector构造
         * @param pairs 包含键值对的vector
         */
        dict(const std::vector<pair>& pairs)
                : dict(pairs.data(), pairs.data() + pairs.size()) {}

        /**
         * @brief 从初始化列表构造
         * @param pairs 初始化列表形式的键值对
         */
        dict(const std::initializer_list<pair>& pairs)
                : dict(pairs.begin(), pairs.end()) {}
        /// @}

        /**
         * @brief 范围构造函数（核心实现）
         * @param begin pair数组起始指针
         * @param end pair数组结束指针
         * @details 遍历处理每个pair，根据值的实际类型进行存储：
         * 1. 基本类型直接存储
         * 2. 列表类型展开存储
         * 3. 嵌套字典递归构造
         * 4. 类成员指针特殊处理
         */
        dict(const pair* begin, const pair* end) {
            for (; begin != end; ++begin) {
                const auto& p = *begin;
                const void* value_ptr = nullptr;

                // 类型别名简化代码
                using type1 = helper::serializable_wrapper;
                using type2 = std::vector<helper::serializable_wrapper>;

                using type3 = std::vector<pair>;
                using type4 = std::initializer_list<pair>;
                using type5 = helper::field_wrapper;

                // 类型分发处理
                if ((value_ptr = p.get_if<type1>()) != nullptr) {
                    // 处理基本类型
                    this->mp.emplace(p.key(), std::forward<type1>(*(type1*)value_ptr));
                    continue;
                }
                if ((value_ptr = p.get_if<type2>()) != nullptr) {
                    // 处理显式列表
                    this->mp.emplace(p.key(), std::forward<type2>(*(type2*)value_ptr));
                    continue;
                }
                if ((value_ptr = p.get_if<type3>()) != nullptr) {
                    // 处理显式嵌套字典
                    this->mp.emplace(p.key(), std::make_unique<dict>(*(type3*)value_ptr));
                    continue;
                }
                if ((value_ptr = p.get_if<type4>()) != nullptr) {
                    // 处理初始化列表嵌套字典
                    this->mp.emplace(p.key(), std::make_unique<dict>(*(type4*)value_ptr));
                    continue;
                }
                if ((value_ptr = p.get_if<type5>()) != nullptr) {
                    // 处理类成员指针
                    this->mp.emplace(p.key(), *(type5*)value_ptr);
                    continue;
                }
                assert_with_message(value_ptr != nullptr, "序列化失败：未知的值类型");
            }
        }
    private:
        /// @brief 内部存储结构
        std::unordered_map<const char*, std::variant<
                helper::serializable_wrapper,  // 基本类型元素
                helper::field_wrapper,         // 类成员指针
                std::vector<helper::serializable_wrapper>,  // 显式列表
                std::unique_ptr<dict>          // 嵌套字典
        >> mp;
    };
}

#endif // SLOWJSON_DICT_HPP