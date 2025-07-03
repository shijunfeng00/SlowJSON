/**
 * @file dict.hpp
 * @brief 实现动态字典结构及相关序列化功能
 * @author hyzh
 * @date 2025/3/4
 */

#ifndef SLOWJSON_DICT_HPP
#define SLOWJSON_DICT_HPP
#include <vector>
#include <map>
#include "load_from_dict_interface.hpp"

namespace slow_json::helper {
    struct pair;
    struct dict;
    struct serializable_wrapper;

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
                : _object_ptr(nullptr),
                  _offset((std::size_t) (&((Class * )nullptr->*field_ptr))),
                  _dump_fn([](
                    slow_json::Buffer &buffer,
                    const void *object_ptr, std::size_t
                    offset) {
                        const auto &object = *(Class *) object_ptr;
                        Field &field = std::ref(*(Field * )((std::uintptr_t) &object + offset));
                        DumpToString<Field>::dump(buffer, field);
                    }),
        _load_fn([](
            const void *object_ptr, std::size_t
            offset,
            const slow_json::dynamic_dict &dict
            ) {
                auto &object = *(Class *) object_ptr;
                Field &field = std::ref(*(Field * )((std::uintptr_t) &object + offset));
                LoadFromDict<Field>::load(field, dict);
            }) {}

        /**
         * @brief 执行字段序列化
         * @param buffer 输出缓冲区
         * @throws 当_object_ptr为空时触发断言
         */
        void dump_fn(slow_json::Buffer &buffer) const {
            assert_with_message(_object_ptr != nullptr, "对象指针为空");
            _dump_fn(buffer, _object_ptr, _offset);
        }

        /**
         * @brief 执行字段反序列化
         * @param dict 输入字典
         * @throws 当_object_ptr为空时触发断言
         */
        void load_fn(const slow_json::dynamic_dict &dict) const {
            assert_with_message(_object_ptr != nullptr, "对象指针为空");
            _load_fn(_object_ptr, _offset, dict);
        }

    private:
        mutable const void *_object_ptr; ///< 绑定的对象指针
        std::size_t _offset; ///< 成员变量相对于对象地址的偏移量
        void (*_dump_fn)(slow_json::Buffer &, const void *, std::size_t); ///< 序列化函数指针
        void (*_load_fn)(const void *, std::size_t, const slow_json::dynamic_dict &); ///< 反序列化函数指针
    };

    /**
     * @brief 可序列化值包装器
     * @details 封装任意可序列化值，提供统一的dump接口，优化小对象存储以减少堆分配
     */
    struct serializable_wrapper {
        enum type{
            FUNDAMENTAL_TYPE,
            LIST_TYPE,
            DICT_TYPE
        };
    public:
        friend struct DumpToString<helper::serializable_wrapper>;
        friend struct LoadFromDict<helper::serializable_wrapper>;

        /**
         * @brief 构造函数，绑定任意可序列化值（复制构造）
         * @tparam T 值类型，需满足可序列化要求且非成员指针
         * @param value 要包装的值
         * @details 小对象（<=24字节）存储在内部缓冲区，大对象分配在堆上
         */
        template<typename T>
        requires(std::is_member_pointer_v<T>)
        serializable_wrapper(const T &value):serializable_wrapper{field_wrapper{value}} { // NOLINT(google-explicit-constructor)

        }
        /**
         * @brief 构造函数，绑定任意可序列化值（复制构造）
         * @tparam T 值类型，需满足可序列化要求且非成员指针
         * @param value 要包装的值
         * @details 小对象（<=24字节）存储在内部缓冲区，大对象分配在堆上
         */
        template<typename T>
        requires(!std::is_member_pointer_v<T>)
        constexpr serializable_wrapper(const T &value) { // NOLINT(google-explicit-constructor)
            using U = std::decay_t<decltype(value)>;
            constexpr std::size_t align_size = alignof(U);
            _type_name = slow_json::type_name_v<U>.str;
            if constexpr (sizeof(U) <= buffer_size && align_size <= alignof(std::max_align_t)) {
                new(&_buffer) U(value);
                set_heap_allocated(false);
            } else {
                _buffer_ptr = new U(value);
                set_heap_allocated(true);
            }
            initialize_functions<U>();
        }

        /**
         * @brief 构造函数，绑定任意可序列化值（移动构造）
         * @tparam T 值类型，需满足可序列化要求且非成员指针
         * @param value 要包装的值
         * @details 小对象（<=24字节）存储在内部缓冲区，大对象分配在堆上
         */
        template<typename T>
        requires(!std::is_member_pointer_v<T> && std::is_rvalue_reference_v<T &&>)
        constexpr serializable_wrapper(T &&value) noexcept {
            using U = std::decay_t<T>;
            constexpr std::size_t align_size = alignof(U);
            _type_name = slow_json::type_name_v<U>.str;
            assert_with_message(((uintptr_t) _type_name) % 8 == 0, "指针地址非8对齐");
            if constexpr (sizeof(U) <= buffer_size && align_size <= alignof(std::max_align_t)) {
                new(&_buffer) U(std::forward<T>(value));
                set_heap_allocated(false);
            } else {
                _buffer_ptr = new U(std::forward<T>(value));
                set_heap_allocated(true);
            }
            initialize_functions<U>();
        }

        /**
         * @brief 拷贝构造函数，模拟移动语义
         * @param other 源对象
         * @details 拷贝构造模拟移动语义以避免std::variant中std::vector的初始化问题
         */
        serializable_wrapper(const serializable_wrapper &other)
                : _type_name(other._type_name),
                  _dump_fn(other._dump_fn),
                  _move_fn(other._move_fn),
                  _deleter(other._deleter) {
            set_heap_allocated(other.is_heap_allocated());
            if (is_heap_allocated()) {
                _buffer_ptr = other._buffer_ptr;
                other._buffer_ptr = nullptr;
            } else {
                other._move_fn(&_buffer, const_cast<void *>(static_cast<const void *>(&other._buffer)));
            }
        }

        /**
         * @brief 移动构造函数
         * @param other 源对象
         * @details 为避免std::variant中std::vector的初始化问题，拷贝构造模拟移动语义。大对象转移指针并置空源指针以防double free，小对象复制缓冲区
         */
        serializable_wrapper(serializable_wrapper &&other) noexcept
                : _type_name(other._type_name),
                  _dump_fn(other._dump_fn),
                  _move_fn(other._move_fn),
                  _deleter(other._deleter) {
            set_heap_allocated(other.is_heap_allocated());
            if (is_heap_allocated()) {
                _buffer_ptr = other._buffer_ptr;
                other._buffer_ptr = nullptr;
            } else {
                other._move_fn(&_buffer, const_cast<void *>(static_cast<const void *>(&other._buffer)));
            }
        }

        /**
         * @brief 析构函数
         * @details 根据堆分配标志释放堆内存或调用小对象的析构函数
         */
        ~serializable_wrapper() {
            if (is_heap_allocated()) {
                if(_buffer_ptr) {
                    _deleter(_buffer_ptr, true);
                }
            } else {
                _deleter(&_buffer, false);
            }
        }

        /**
         * @brief 执行序列化
         * @param buffer 输出缓冲区
         * @details 调用存储的序列化函数，处理小对象或大对象
         */
        void dump_fn(slow_json::Buffer &buffer) const {
            void *ptr = is_heap_allocated() ? _buffer_ptr : (void *) &_buffer;
            _dump_fn(buffer, ptr);
        }

        /**
         * @brief 获取值的指针
         * @return void* 指向值的指针
         */
        [[nodiscard]] const void *value()const{
            return is_heap_allocated() ? _buffer_ptr : (void *) &_buffer;
        }

        /**
         * @brief 获取值的类型名称
         * @return std::string_view 类型名称
         */
        [[nodiscard]] std::string_view type_name() const noexcept {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            ptr &= ~static_cast<uintptr_t>(1);
            assert_with_message(ptr % 8 == 0, "指针地址非8对齐");
            return std::string_view{reinterpret_cast<const char *>(ptr)};
        }

    private:
        static constexpr std::size_t buffer_size = 24; ///< 小对象缓冲区大小
        union {
            mutable void *_buffer_ptr; ///< 大对象指针，指向堆内存
            alignas(8) char _buffer[buffer_size]; ///< 小对象缓冲区
        };
        mutable const char *_type_name; ///< 类型名称，带堆分配标志
        void (*_dump_fn)(slow_json::Buffer &, void *)=nullptr; ///< 序列化函数指针
        void (*_move_fn)(void *, void *)=nullptr; ///< 移动/复制函数指针
        void (*_deleter)(void *, bool)=nullptr; ///< 析构函数指针

        /**
         * @brief 初始化序列化、移动和析构函数
         * @tparam U 值类型
         */
        template<typename U>
        void initialize_functions() {
            _dump_fn = [](slow_json::Buffer &buffer, void *object) {
                slow_json::DumpToString<U>::dump(buffer, *static_cast<U *>(object));
            };
            _move_fn = [](void *dest, void *src) {
                if constexpr (std::is_trivially_copyable_v<U>) {
                    std::memcpy(dest, src, sizeof(_buffer));
                } else if constexpr (std::is_move_constructible_v<U>) {
                    new(dest) U(std::move(*static_cast<U *>(src)));
                } else if constexpr (std::is_copy_constructible_v<U>) {
                    new(dest) U(*static_cast<U *>(src));
                } else {
                    assert_with_message(false, "类型既不可复制也不可移动: %s", slow_json::type_name_v<U>.str);
                }
            };
            _deleter = [](void *object, bool is_heap_allocated) {
                if (is_heap_allocated) {
                    delete static_cast<U *>(object);
                } else {
                    static_cast<U *>(object)->~U();
                }
            };
        }

        /**
         * @brief 设置堆分配标志
         * @param value 是否为堆分配
         */
        void set_heap_allocated(bool value) const noexcept {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            ptr = value ? (ptr | 1) : (ptr & ~static_cast<uintptr_t>(1));
            _type_name = reinterpret_cast<char *>(ptr);
        }

        /**
         * @brief 获取堆分配标志
         * @return bool 是否为堆分配
         */
        [[nodiscard]] bool is_heap_allocated() const noexcept {
            return reinterpret_cast<uintptr_t>(_type_name) & 1;
        }
    };

    /**
    * @brief 动态字典结构，用于存储键值对集合
    * @details 提供键值对的存储、序列化和反序列化功能，支持动态数据操作
    */
    struct dict {
        friend struct pair;
        friend struct LoadFromDict<dict>;
        friend struct DumpToString<dict>;

        /**
         * @brief 构造函数，使用初始化列表构造字典
         * @param data 初始化列表，包含键值对
         * @details 使用移动语义将键值对列表存储到字典中
         */
        dict(std::initializer_list<pair>&& data) : _data(data) {}

        /**
         * @brief 构造函数，使用可变参数模板构造字典，兼容基于std::pair的老接口
         * @tparam K 键类型
         * @tparam V 值类型
         * @param data 键值对参数包
         * @details 将参数包中的键值对转换为pair对象并存储
         */
        template<typename... K, typename... V>
        constexpr dict(std::pair<K, V>&&... data)
                : _data{{pair{std::move(data.first), std::move(data.second)}...}} {}

        /**
         * @brief 拷贝构造函数（禁用）
         * @details 禁用拷贝构造函数以防止意外拷贝
         */
        dict(const dict&) = delete;

        /**
         * @brief 移动构造函数
         * @param d 源字典
         * @details 移动源字典的键值对数据到新对象
         */
        dict(dict&& d) : _data{std::move(d._data)} {}

    private:
        std::vector<pair> _data; ///< 存储键值对的向量
    };

    /**
    * @brief 键值对结构，用于表示字典中的单个键值对
    * @details 封装键（字符串）和值（可序列化对象），支持序列化和反序列化操作
    */
    struct pair {
        friend struct LoadFromDict<pair>;
        friend struct DumpToString<pair>;
        friend struct DumpToString<dict>;
        friend struct LoadFromDict<dict>;
        friend struct dict;

        /**
         * @brief 构造函数，绑定键和值（列表类型）
         * @param key 键，字符串形式
         * @param value 值，移动构造的序列化包装器列表
         * @details 构造一个键值对，值存储为序列化包装器列表
         */
        constexpr pair(const char *key, std::vector<serializable_wrapper> &&value)
                : _key{key}, _value{std::move(value)} {}

        /**
         * @brief 构造函数，绑定键和值（单一序列化对象）
         * @param key 键，字符串形式
         * @param value 值，移动构造的序列化包装器
         * @details 构造一个键值对，值存储为单一序列化包装器
         */
        pair(const char *key, serializable_wrapper &&value)
                : _key{key}, _value(std::move(value)) {}

        /**
         * @brief 构造函数，绑定键和值（字典类型）
         * @param key 键，字符串形式
         * @param value 值，移动构造的字典
         * @details 构造一个键值对，值存储为字典
         */
        constexpr pair(const char *key, dict &&value)
                : _key{key}, _value{std::move(value)} {}

    private:
        const char *_key; ///< 键，字符串形式
        serializable_wrapper _value; ///< 值，序列化包装器形式
    };

}
namespace slow_json{
    using helper::pair;
    using helper::dict;
    using list=std::vector<helper::serializable_wrapper>;
}
#endif // SLOWJSON_DICT_HPP