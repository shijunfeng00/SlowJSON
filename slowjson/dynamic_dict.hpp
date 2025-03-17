//
// Created by hy-20 on 2024/7/23.
//

#ifndef SLOWJSON_DYNAMIC_DICT_HPP
#define SLOWJSON_DYNAMIC_DICT_HPP

#include "document.h"
#include "writer.h"
#include "concetps.hpp"
#include "dump_to_string_interface.hpp"
#include "enum.hpp"

namespace slow_json {

    template<typename T>
    struct LoadFromDict;

    template<typename T>
    struct ILoadFromDict;

    using namespace rapidjson;

    /**
    * @brief 一个可以动态访问和转换 JSON 数据的结构体，采用引用计数管理内存
    * @details 该结构体使用 rapidjson 库存储和操作 JSON 数据，通过内部共享数据结构实现引用计数，
    *          从根对象派生出来的所有子对象都会增加引用计数，确保即使根对象被析构，
    *          只要有子对象存在，底层的 JSON 数据依然有效。注释中的变量和函数命名风格与原代码保持一致。
    */
    class dynamic_dict {
    public:
        dynamic_dict():_shared{nullptr},_value{nullptr}{}
        /**
         * @brief 从 JSON 字符串创建一个 dynamic_dict 对象（根对象）
         * @param json_data 一个 std::string_view 类型的参数，表示 JSON 字符串
         * @throws std::runtime_error 当 JSON 字符串为空或解析错误时
         */
        explicit dynamic_dict(std::string_view json_data)
        {
            auto allocator = new rapidjson::MemoryPoolAllocator<>(std::max(1024ul,json_data.size()*4));

#ifndef NDEBUG
            _json_str = std::string(json_data);
#endif
            auto document=new rapidjson::Document(allocator);
            document->SetNull();
            document->Parse(json_data.data());
            if (json_data.empty()) {
                throw std::runtime_error("空JSON字符串");
            }
            if (document->HasParseError()) {
                std::string error_message = std::string("JSON解析错误:") + std::string(json_data);
                delete document;
                delete allocator;
                throw std::runtime_error(error_message);
            }
            _shared = new SharedData(document, allocator, true);
            _value = document;
        }

        /**
         * @brief 拷贝构造函数，增加引用计数
         * @param other 另一个 dynamic_dict 对象
         */
        dynamic_dict(const dynamic_dict &other)
                : _shared(other._shared), _value(other._value)
#ifndef NDEBUG
                , _json_str(other._json_str)
#endif
        {
            if (_shared) {
                ++_shared->_ref_count;
            }
        }

        /**
         * @brief 移动构造函数，转移内部指针
         * @param other 另一个 dynamic_dict 对象
         */
        dynamic_dict(dynamic_dict &&other) noexcept
                : _shared(other._shared), _value(other._value)
#ifndef NDEBUG
                , _json_str(std::move(other._json_str))
#endif
        {
            other._shared = nullptr;
            other._value = nullptr;
        }

        /**
         * @brief 析构函数，减少引用计数，释放内存（如果引用计数为0）
         */
        ~dynamic_dict() {
            if (_shared) {
                --_shared->_ref_count;
                if (_shared->_ref_count == 0) {
                    delete _shared;
                }
            }
        }

        /**
         * @brief 拷贝赋值运算符
         * @param other 另一个 dynamic_dict 对象
         * @return 当前对象的引用
         */
        dynamic_dict &operator=(const dynamic_dict &other) {
            if (this != &other) {
                if (_shared) {
                    --_shared->_ref_count;
                    if (_shared->_ref_count == 0) {
                        delete _shared;
                    }
                }
                _shared = other._shared;
                _value = other._value;
#ifndef NDEBUG
                _json_str = other._json_str;
#endif
                if (_shared) {
                    ++_shared->_ref_count;
                }
            }
            return *this;
        }

        /**
         * @brief 移动赋值运算符
         * @param other 另一个 dynamic_dict 对象
         * @return 当前对象的引用
         */
        dynamic_dict &operator=(dynamic_dict &&other) noexcept {
            if (this != &other) {
                if (_shared) {
                    --_shared->_ref_count;
                    if (_shared->_ref_count == 0) {
                        delete _shared;
                    }
                }
                _shared = other._shared;
                _value = other._value;
#ifndef NDEBUG
                _json_str = std::move(other._json_str);
#endif
                other._shared = nullptr;
                other._value = nullptr;
            }
            return *this;
        }

        /**
         * @brief 从 rapidjson::Value 创建一个 dynamic_dict 对象（包装），不拥有内存
         * @param value rapidjson::Value 的引用
         * @return dynamic_dict 类型的包装对象
         * @note 通过此方法创建的对象不会管理内存，调用修改接口可能导致未定义行为
         */
        static dynamic_dict wrap(const rapidjson::Value &value) {
            // 不会涉及到数据修改，因此不需要传入allocator
            auto *shared = new SharedData(const_cast<rapidjson::Value *>(&value), nullptr, false);
            return {shared, const_cast<rapidjson::Value *>(&value)};
        }

        /**
         * @brief 设置属性值
         * @tparam T 属性类型
         * @param val 新的属性值
         * @return 当前对象的引用
         */
        template<typename T>
        requires std::is_fundamental_v<T> || concepts::contains<T, std::string, std::string_view>
        dynamic_dict &operator=(const T &val) {
            if constexpr (std::is_same_v<T, std::string>) {
                _value->Set(val.c_str(), *(_shared->_allocator));
            } else if constexpr (std::is_same_v<T, std::string_view>) {
                std::string val2{val};
                _value->Set(val2.c_str(), *(_shared->_allocator));
            } else {
                _value->Set(val, *(_shared->_allocator));
            }
#ifndef NDEBUG
            update_json_str();
#endif
            return *this;
        }

        /**
         * @brief 设置属性值，仅支持基本类型
         * @tparam T 属性类型
         * @param val 新的属性值
         */
        template<typename T>
        void set(const T &val) {
            static_assert(std::is_fundamental_v<T>, "该接口只能支持C++的基本变量，不支持更加复杂的局部JSON重构");
            _value->Set(val, *(_shared->_allocator));
#ifndef NDEBUG
            update_json_str();
#endif
        }

        /**
         * @brief 判断 JSON 是否为空（null）
         * @return 如果 JSON 是 null 则返回 true，否则返回 false
         */
        [[nodiscard]] bool empty() const noexcept {
            return _value->IsNull();
        }

        /**
         * @brief 通过函数调用形式获取元素，与 operator[] 等效
         * @tparam T 键或索引类型
         * @param key_or_index 键或索引
         * @return 对应的 dynamic_dict 子对象
         */
        template<typename T>
        auto at(T key_or_index) const {
            return this->operator[](key_or_index);
        }

        /**
         * @brief 重载 [] 运算符，根据整数索引访问 JSON 数组中的元素
         * @param index 数组索引
         * @return 对应的 dynamic_dict 子对象
         */
        dynamic_dict operator[](std::size_t index) const {
            assert_with_message(_value->IsArray(),
                                "试图把JSON当作数组访问，但实际他并不是个数组，发生错误的JSON字符串为:%s",_json_str.c_str());
            const auto &array = _value->GetArray();
            assert_with_message(index < array.Size(), "数组访问越界");
            return {_shared, const_cast<rapidjson::Value *>(&array[index])};
        }

        /**
         * @brief 重载 [] 运算符，根据字符串键访问 JSON 对象中的元素
         * @param key 字符串键
         * @return 对应的 dynamic_dict 子对象
         */
        dynamic_dict operator[](std::string_view key) const {
            assert_with_message(_value->IsObject(),
                                "试图把JSON当作字典访问，但实际他并不是个字典，实际类型为%s,发生错误的JSON字符串为:%s",
                                slow_json::enum2string(_value->GetType()).data(),_json_str.c_str());
            if (!_value->HasMember(key.data())) {
                throw std::runtime_error(std::string("没有找到对应的key:") + std::string(key));
            }
            return {_shared, &_value->operator[](key.data())};
        }

        /**
         * @brief 类型转换运算符，将 JSON 数据转换为目标类型 T
         * @tparam T 目标类型
         * @return 转换后的对象
         */
        template<typename T>
        explicit operator T() const {
            if constexpr (!concepts::optional<T>) {
                assert_with_message(!_value->IsNull(),
                                    "尝试解析null空对象为非空对象，对象类型必须为std::optional<T>，发生错误的JSON字符串为:%s",_json_str.c_str());
                T object;
                LoadFromDict<T>::load(object, *this);
                return object;
            } else {
                if (_value->IsNull()) {
                    return std::nullopt;
                } else {
                    return (typename T::value_type)(*this);
                }
            }
        }

        /**
         * @brief 显式类型转换方法
         * @tparam T 目标类型
         * @return 转换后的对象
         * @see operator T
         */
        template<typename T>
        [[nodiscard]] T cast() const {
            return this->operator T();
        }

        /**
         * @brief 使用 JSON 的值填充给定对象的数据
         * @tparam T 数据类型
         * @param value 被填充的对象
         */
        template<typename T>
        void fit(T &value) const {
            if constexpr (!concepts::optional<T>) {
                assert_with_message(!_value->IsNull(),
                                    "尝试解析null空对象为非空对象，value类型必须为std::optional<T>，发生错误的JSON字符串为:%s",_json_str.c_str());
                LoadFromDict<T>::load(value, *this);
            } else {
                if (_value->IsNull()) {
                    value = std::nullopt;
                } else {
                    LoadFromDict<T>::load(value, *this);
                }
            }
        }

        /**
         * @brief 返回 JSON 数组的大小
         * @return 数组中元素的个数
         */
        [[nodiscard]] std::size_t size() const {
            assert_with_message(_value->IsArray(),
                                "试图把JSON当作数组访问，但实际他并不是个数组，发生错误的JSON字符串为:%s",_json_str.c_str());
            const auto &array = _value->GetArray();
            return array.Size();
        }

        /**
         * @brief 判断 JSON 对象中是否包含指定键
         * @param key 键名称
         * @return 如果包含该键返回 true，否则返回 false
         */
        [[nodiscard]] bool contains(std::string_view key) const noexcept {
            assert_with_message(_value->IsObject(),
                                "试图把JSON当作字典访问，但实际他并不是个数组，发生错误的JSON字符串为:%s",_json_str.c_str());
            return _value->HasMember(key.data());
        }

        /**
         * @brief 获取原始的 rapidjson::Value 指针
         * @return 指向 rapidjson::Value 的常量指针
         */
        [[nodiscard]] const rapidjson::Value *value() const noexcept {
            return _value;
        }

        /**
         * @brief 判断 JSON 是否为数组
         * @return 如果为数组返回 true，否则返回 false
         */
        [[nodiscard]] bool is_array() const noexcept {
            return _value->IsArray();
        }

        /**
         * @brief 判断 JSON 是否为对象
         * @return 如果为对象返回 true，否则返回 false
         */
        [[nodiscard]] bool is_object() const noexcept {
            return _value->IsObject();
        }

#ifndef NDEBUG
        /**
         * @brief 获取当前 JSON 的字符串表示，用于调试
         * @return JSON 字符串
         */
        [[nodiscard]] const std::string &json_str() const noexcept {
            return _json_str;
        }
#endif

    private:
        /**
         * @brief 内部共享数据结构，管理 JSON 数据及其内存分配器的生命周期
         */
        struct SharedData {
            rapidjson::Value* _root; ///< 指向根 JSON 对象的指针
            rapidjson::MemoryPoolAllocator<>* _allocator; ///< 内存池分配器
            int _ref_count; ///< 引用计数
            bool _owns_memory; ///< 是否拥有内存所有权

            /**
             * @brief 构造函数
             * @param root 指向根 JSON 对象的指针
             * @param allocator 内存池分配器
             * @param owns_memory 是否拥有内存所有权
             */
            SharedData(rapidjson::Value* root, rapidjson::MemoryPoolAllocator<>* allocator, bool owns_memory)
                    : _root(root), _allocator(allocator), _ref_count(1), _owns_memory(owns_memory) {}

            /**
             * @brief 析构函数，根据所有权释放内存
             */
            ~SharedData() {
                if (_owns_memory) {
                    delete _root;
                    delete _allocator;
                }
            }
        };

        SharedData* _shared;         ///< 内部共享数据指针
        rapidjson::Value* _value;      ///< 当前 JSON 值的指针
#ifndef NDEBUG
        std::string _json_str;         ///< 当前 JSON 的字符串表示，用于调试
#endif

        /**
         * @brief 私有构造函数，用于创建子对象，增加共享数据的引用计数
         * @param shared 内部共享数据指针
         * @param value 当前 JSON 值的指针
         */
        dynamic_dict(SharedData* shared, rapidjson::Value* value)
                : _shared(shared), _value(value)
        {
            if (_shared) {
                ++_shared->_ref_count;
            }
#ifndef NDEBUG
            update_json_str();
#endif
        }

#ifndef NDEBUG
        /**
         * @brief 更新调试用的 JSON 字符串表示
         */
        void update_json_str() noexcept {
            rapidjson::StringBuffer rapid_buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(rapid_buffer);
            _value->Accept(writer);
            _json_str = rapid_buffer.GetString();
        }
#endif
    };

    /**
     * 为将slow_json::dynamic_dict重新转换为JSON字符串提供支持
     */
    template<>
    struct DumpToString<dynamic_dict> : public IDumpToString<DumpToString<dynamic_dict>> {
        static void dump_impl(Buffer &buffer, const dynamic_dict &value) {
            rapidjson::StringBuffer rapid_buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(rapid_buffer);
            value.value()->Accept(writer);
            buffer += rapid_buffer.GetString();
        }
    };


}
#endif //SLOWJSON_DYNAMIC_DICT_HPP
