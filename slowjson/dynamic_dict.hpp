//
// Created by hy-20 on 2024/7/23.
//

#ifndef SLOWJSON_DYNAMIC_DICT_HPP
#define SLOWJSON_DYNAMIC_DICT_HPP

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "concetps.hpp"
#include "dump_to_string_interface.hpp"

namespace slow_json {

    template<typename T>
    struct LoadFromDict;

    template<typename T>
    struct ILoadFromDict;

    using namespace rapidjson;

    static rapidjson::MemoryPoolAllocator<> *get_allocator() {
//        static thread_local std::size_t max_memory_size = 4096*64;
        static thread_local auto *allocator = new rapidjson::MemoryPoolAllocator(1024 * 64);
//        if (size > max_memory_size) {
//            max_memory_size = size;
//            delete allocator;
//            allocator = new rapidjson::MemoryPoolAllocator(max_memory_size);
//        }
        return allocator;
    }

    /**
     * @brief 一个可以动态访问和转换 JSON 数据的结构体
     * @details 这个结构体使用了 rapidjson 库来存储和操作 JSON 数据，提供了方便的重载运算符和类型转换方法，可以根据不同的键或索引来获取 JSON 对象或数组中的元素，并将其转换为所需的类型。
     * @note 这个结构体不负责内存管理，如果是从 JSON 字符串创建的对象，则会在析构时释放内存，如果是从已有的 Value 引用创建的对象，则不会释放内存。
     */
    class dynamic_dict {
    public:
        dynamic_dict() : _value(nullptr), _is_root(false), _allocator(nullptr) {}

        /**
         * @brief 从 JSON 字符串创建一个 dynamic_dict 对象
         * @param json_data 一个 std::string_view 类型的参数，表示 JSON 字符串
         */
        explicit dynamic_dict(std::string_view json_data) : _value(new Value), _is_root(true),
                                                            _allocator(get_allocator()) {
            std::string json_data_str{json_data};
            static thread_local Document document(_allocator);
            document.SetNull();
            document.Parse(json_data_str.c_str());
            if (json_data.empty()) {
                throw std::runtime_error("空JSON字符串");
            }
            if (document.HasParseError()) {
                std::string error_message = std::string{"JSON解析错误:"} + std::string{json_data};
                delete _value;
                throw std::runtime_error(error_message);
            }
            _value->CopyFrom(document, document.GetAllocator());
        }

        /**
         * @brief 析构函数，如果是根对象，则释放Value内存，否则不需要清理，在释放Value的时候，所有的相关联的JSON对象都会一并释放
         */
        ~dynamic_dict() {
            if (_is_root) {
                delete _value;
            }
        }

        /**
         * 禁止复制构造
         */
        dynamic_dict(const dynamic_dict &) = delete;

        /**
         * 移动构造的时候，转移value所有权，并清空之前的数据
         * @param o
         */
        dynamic_dict(dynamic_dict &&o) noexcept {
            this->_is_root = o._is_root;
            this->_value = o._value;
            this->_allocator = o._allocator;
            o._value = nullptr;
            o._is_root = false;
            o._allocator = nullptr;
        }

        /**
         * 从rapidjson::Value直接构造一个对象，但仅仅只是起到一个封装作用，不会进行任何内存上的管理
         * @note 用这个方式生成的对象，不要尝试调用任何可能改变对象数据的接口，否则可能会当场死亡
         * @param value rapidJson对象
         * @return dynamic_dict类型的封装
         */
        static dynamic_dict wrap(const Value &value) {
            Value &v = *const_cast<Value *>(&value);
            return dynamic_dict(v, get_allocator(), false);
        }

        dynamic_dict &operator=(dynamic_dict &&o) noexcept {
            if (this == &o) {
                return *this;
            }
            this->_is_root = o._is_root;
            this->_value = o._value;
            o._value = nullptr;
            o._is_root = false;
            return *this;
        }

        /**
         * 设置属性值
         * @tparam T 属性类型
         * @param val 新的属性值
         */
        template<typename T>
        requires std::is_fundamental_v<T> || concepts::contains<T, std::string, std::string_view>
        dynamic_dict &operator=(const T &val) {
            if constexpr (std::is_same_v<T, std::string>) {
                this->_value->Set(val.c_str(), *_allocator);
            } else if constexpr (std::is_same_v<T, std::string_view>) {
                std::string val2{val};
                this->_value->Set(val2.c_str(), *_allocator);
            } else {
                this->_value->Set(val, *_allocator);
            }
            return *this;
        }

        /**
         * 安全的复制一个拷贝对象，但这个新的对象不会试图去删除指针数据
         * 注意，这里并没有考虑引用计数的问题，如果最原始的对象丢失，将会导致内存泄漏问题
         * @return
         */
        [[nodiscard]] dynamic_dict copy() const {
            return dynamic_dict(*this->_value, _allocator, false);
        }

        /**
         * 设置属性值
         * @tparam T 属性类型
         * @param val 新的属性值
         */
        template<typename T>
        void set(const T &val) {
            static_assert(std::is_fundamental_v<T>, "该接口只能支持C++的基本变量，不支持更加复杂的局部JSON重构");
            this->_value->Set(val, *_allocator);
        }

        /**
         * 判断字典的某个键是否为空（null）
         * @return
         */
        [[nodiscard]] bool empty() const noexcept {
            return this->_value->IsNull();
        }

        /**
         * 一通过函数调用的形式获取元素，功能与中括号传参一致，主要是为了方便指针调用
         */
        template<typename T>
        auto at(T key_or_index) const {
            return this->operator[](key_or_index);
        }

        /**
         * @brief 重载 [] 运算符，根据整数作为索引来访问 JSON 数组中的元素
         * @param index 一个 int 类型的参数，表示索引
         * @return 一个 dynamic_dict 类型的对象，表示 JSON 数组中对应索引的值
         */
        dynamic_dict operator[](std::size_t index) const {
            assert_with_message(this->_value->IsArray(), "试图把JSON当作数组访问，但实际他并不是个数组");
            const auto &array = this->_value->GetArray();
            assert_with_message(index < array.Size(), "数组访问越界");
            return dynamic_dict(array[index], _allocator);
        }

        /**
         * @brief 重载 [] 运算符，根据字符串作为键来访问 JSON 对象中的元素
         * @param key 一个 std::string_view 类型的参数，表示键
         * @return 一个 dynamic_dict 类型的对象，表示 JSON 对象中对应键的值
         */
        dynamic_dict operator[](std::string_view key) const {
            assert_with_message(this->_value->IsObject(), "试图把JSON当作字典访问，但实际他并不是个字典");
            if (!this->_value->HasMember(key.data())) {
                throw std::runtime_error(std::string{"没有找到对应的key:"} + std::string{key});
            }
            return dynamic_dict(this->_value->operator[](key.data()), _allocator);
        }

        /**
         * @brief 重载类型转换运算符，根据不同的模板参数 T 来将 JSON 数据转换为对应的类型
         * @details 支持基本类型、字符串类型、容器类型和自定义类型
         * @tparam T 要转换为的目标类型，可以是int,float,std::string这样的基础类型，也可以是std::vector<int>,std::unordered_map<int,std::vector<std::string>>这样的容器类型
         * @return 转换后的目标类型对象
         */
        template<typename T>
        explicit operator T() const {
            if constexpr (!concepts::optional<T>) {
                assert_with_message(!this->_value->IsNull(), "尝试解析null空对象为非空对象");
                T object;
                LoadFromDict<T>::load(object, *this);
                return object;
            } else {
                if (this->_value->IsNull()) {
                    return std::nullopt;
                } else {
                    return (typename T::value_type) (*this);
                }
            }
        }

        /**
         * @brief 提供一个显式的类型转换方法，调用类型转换运算符
         * @tparam T 要转换为的目标类型
         * @return 转换后的目标类型对象
         * @see operator T
         */
        template<typename T>
        [[nodiscard]] T cast() const {
            return this->operator T();
        }

        /**
         * @brief 用JSON的值反序列化后去填充对象的数据
         * @details 相比于a=json["key"}.cast<std::vector<int>>()的方法
         *          采用json["key"].fit(a)可以写得更加简洁，不需要显式指定类型
         * @tparam T 被填充的数据的类型
         * @param _value 被填充的数据
         */
        template<typename T>
        void fit(T &value) const {
            if constexpr (!concepts::optional<T>) {
                assert_with_message(!this->_value->IsNull(), "尝试解析null空对象为非空对象");
                LoadFromDict<T>::load(value, *this);
            } else {
                if (this->_value->IsNull()) {
                    value = std::nullopt;
                } else {
                    LoadFromDict<T>::load(value, *this);
                }
            }
        }

        /**
         * @brief 返回 JSON 数组的大小
         * @return 一个 size_t 类型的值，表示 JSON 数组的元素个数
         */
        [[nodiscard]] std::size_t size() const {
            assert_with_message(this->_value->IsArray(), "试图把JSON当作数组访问，但实际他并不是个数组");
            const auto &array = this->_value->GetArray();
            return array.Size();
        }

        /**
         * 返回JSON字典中是否包含某个键值对
         * @param key 查询的健名称
         * @return 是否包含这个键值对
         */
        [[nodiscard]] bool contains(std::string_view key) const noexcept {
            assert_with_message(this->_value->IsObject(), "试图把JSON当作字典访问，但实际他并不是个数组");
            return this->_value->HasMember(key.data());
        }

        /**
         * 获得原始的rapidjson::Value指针数据
         * @return
         */
        [[nodiscard]] const Value *value() const noexcept {
            return this->_value;
        }

        /**
         * JSON是否可以被解析为数组
         * @return
         */
        [[nodiscard]] bool is_array() const noexcept {
            return this->_value->IsArray();
        }

        /**
         * JSON是否可以被解析为对象
         * @return
         */
        [[nodiscard]] bool is_object() const noexcept {
            return this->_value->IsObject();
        }

    private:
        /**
         * @brief 从已有的 Value 引用创建一个 dynamic_dict 对象
         * @param _value 一个 Value 类型的引用，表示已经解析好的 JSON 数据
         * @param _is_root 是否是最根本的对象，如果是的话，那么析构的时候需要将其delete，否则不需要delete
         */
        explicit dynamic_dict(Value &_value, MemoryPoolAllocator<> *allocator, bool _is_root = false) :
                _value(&_value),
                _is_root(_is_root),
                _allocator(allocator) {}

        /**
         * @brief 从已有的 Value 引用创建一个 dynamic_dict 对象
         * @param _value 一个 Value 类型的指针，表示已经解析好的 JSON 数据
         * @param _is_root 是否是最根本的对象，如果是的话，那么析构的时候需要将其delete，否则不需要delete
         */
        explicit dynamic_dict(Value *_value, MemoryPoolAllocator<> *allocator, bool _is_root = false) :
                _value(_value),
                _is_root(_is_root),
                _allocator(allocator) {}

        Value *_value;///< 一个指向 Value 类型的指针，用来存储 JSON 数据
        bool _is_root;  ///< 一个布尔值，表示是否是根对象（Document对象），用来判断是否需要释放内存
        MemoryPoolAllocator<> *_allocator; ///<内存池对象，存储解析之后的对象结果
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
