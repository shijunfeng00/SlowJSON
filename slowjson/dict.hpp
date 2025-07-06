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
#include <unordered_map>
#include "load_from_dict_interface.hpp"

namespace slow_json::details {
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
        constexpr field_wrapper(Field Class::*field_ptr)  // NOLINT(google-explicit-constructor)
                : _object_ptr(nullptr),
                  _offset((std::size_t) (&((Class *)nullptr->*field_ptr))),
                  _dump_fn([](
                    slow_json::Buffer &buffer,
                    const void *object_ptr, std::size_t
                    offset) {
            const auto &object = *(Class *)object_ptr;
            Field &field = std::ref(*(Field *)((std::uintptr_t)&object + offset));
            DumpToString<Field>::dump(buffer, field);
        }),
        _load_fn([](
            const void *object_ptr, std::size_t
            offset,
            const slow_json::dynamic_dict &dict
            ) {
            auto &object = *(Class *)object_ptr;
            Field &field = std::ref(*(Field *)((std::uintptr_t)&object + offset));
            LoadFromDict<Field>::load(field, dict);
        }) {}

        /**
         * @brief 执行字段序列化
         * @param buffer 输出缓冲区
         * @throws 当_object_ptr为空时触发断言
         */
        void dump_fn(slow_json::Buffer &buffer) const SLOW_JSON_NOEXCEPT {
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

    struct serializable_wrapper {
        /**
         * @brief 对dict元素访问提供支持
         * @details 从dict解析出原本的C++对象需要一定的类型信息的支持，需要知道serializable_wrapper内部存储的是原始C++对象，列表，字典还是根字典
         * @note 每一种类型的定义如下：
         * - 原始C++对象：非dict非pair的无slow_json封装的对象
         * - 列表：std::vector<serializable_wrapper>
         * - 字典：slow_json::dict（嵌套字典）
         * - 根字典：slow_json::dict（键值对集合）
         */
        enum ElementType {
            FUNDAMENTAL_TYPE = 0,  // 原始C++对象 00
            LIST_TYPE = 1,         // 列表 01
            DICT_TYPE = 2,         // 嵌套字典 10
            ROOT_DICT_TYPE = 3     // 根字典 11
        };

    public:
        friend struct DumpToString<details::serializable_wrapper>;
        friend struct LoadFromDict<details::serializable_wrapper>;

        /**
         * @brief 构造函数，绑定类成员指针
         * @tparam T 值类型，必须为成员指针
         * @param value 要包装的成员指针
         * @details 将成员指针包装为field_wrapper
         */
        template<typename T>
        requires(std::is_member_pointer_v<T>)
        constexpr serializable_wrapper(const T &value):serializable_wrapper{field_wrapper{value}} { // NOLINT(google-explicit-constructor)

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
            // 设置原始类型名称并添加类型信息
            _type_name = slow_json::type_name_v<U>.str;
            set_type<U>();
            // 根据大小决定存储方式
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
        constexpr serializable_wrapper(T &&value) SLOW_JSON_NOEXCEPT { // NOLINT(google-explicit-constructor)
            using U = std::decay_t<T>;
            constexpr std::size_t align_size = alignof(U);

            // 设置原始类型名称并添加类型信息
            _type_name = slow_json::type_name_v<U>.str;
            set_type<U>();

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
        serializable_wrapper(const serializable_wrapper &other) SLOW_JSON_NOEXCEPT
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
         * @brief 移动赋值运算符
         * @param other 源对象
         * @return 当前对象的引用
         * @details 清理当前资源，转移源对象的资源，避免重复释放
         */
        serializable_wrapper& operator=(serializable_wrapper&& other) noexcept {
            if (this != &other) {
                // 清理当前对象的资源
                if (is_heap_allocated()) {
                    if (_buffer_ptr) {
                        _deleter(_buffer_ptr, true);
                    }
                } else {
                    _deleter(&_buffer, false);
                }

                // 转移 other 的资源
                _type_name = other._type_name;
                _dump_fn = other._dump_fn;
                _move_fn = other._move_fn;
                _deleter = other._deleter;
                set_heap_allocated(other.is_heap_allocated());
                if (is_heap_allocated()) {
                    _buffer_ptr = other._buffer_ptr;
                    other._buffer_ptr = nullptr;
                } else {
                    other._move_fn(&_buffer, const_cast<void *>(static_cast<const void *>(&other._buffer)));
                }
            }
            return *this;
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
                if (_buffer_ptr) {
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
        void dump_fn(slow_json::Buffer &buffer) const SLOW_JSON_NOEXCEPT {
            void *ptr = is_heap_allocated() ? _buffer_ptr : (void *)&_buffer;
            _dump_fn(buffer, ptr);
        }

        /**
         * @brief 获取值的指针
         * @return void* 指向值的指针
         */
        [[nodiscard]] const void *value() const SLOW_JSON_NOEXCEPT {
            return is_heap_allocated() ? _buffer_ptr : (void *)&_buffer;
        }

        /**
         * @brief 获取值的指针
         * @return void* 指向值的指针
         */
        [[nodiscard]] void *value() SLOW_JSON_NOEXCEPT {
            return is_heap_allocated() ? _buffer_ptr : (void *)&_buffer;
        }

        /**
         * @brief 获取被存储的原本类型名称，也即构造函数模板参数类型字符串
         * @return std::string_view 类型名称
         */
        [[nodiscard]] std::string_view type_name() const SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            ptr &= CLEAR_MASK;  // 清除低3位
            assert_with_message(ptr % 8 == 0, "指针地址非8对齐");
            return std::string_view{reinterpret_cast<const char *>(ptr)};
        }

        /**
         * @brief 获取存储的类型信息
         * @return ElementType 类型枚举值
         * @details 返回基本类型、列表、字典或根字典的类型
         */
        [[nodiscard]] ElementType get_value_element_type() const SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            return static_cast<ElementType>((ptr & TYPE_MASK) >> 1);
        }

    private:
        static constexpr std::size_t buffer_size = 24; ///< 小对象缓冲区大小

        // 位掩码定义
        static constexpr uintptr_t HEAP_MASK = 0x1;    // 最低位 - 堆分配标志
        static constexpr uintptr_t TYPE_MASK = 0x6;    // 中间两位 - 类型信息 (0b0110)
        static constexpr uintptr_t CLEAR_MASK = ~0x7;  // 清除低3位的掩码

        union {
            mutable void *_buffer_ptr; ///< 大对象指针，指向堆内存
            alignas(8) char _buffer[buffer_size]; ///< 小对象缓冲区
        };
        mutable const char *_type_name; ///< 类型名称，内存地址8对齐，末尾3位存储类型和堆分配信息
        void (*_dump_fn)(slow_json::Buffer &, void *) = nullptr; ///< 序列化函数指针
        void (*_move_fn)(void *, void *) = nullptr; ///< 移动/复制函数指针
        void (*_deleter)(void *, bool) = nullptr; ///< 析构函数指针

        /**
         * @brief 设置类型信息
         * @tparam U 实际存储的类型
         * @details 根据类型设置类型标志位
         */
        template<typename U>
        void set_type() SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            // 清除原有类型信息
            ptr &= ~TYPE_MASK;

            // 设置新类型信息
            if constexpr (std::is_same_v<U, std::vector<serializable_wrapper>>) {
                ptr |= (LIST_TYPE << 1);
            } else if constexpr (std::is_same_v<U, dict>) {
                ptr |= (DICT_TYPE << 1);
            } else {
                ptr |= (FUNDAMENTAL_TYPE << 1);
            }

            _type_name = reinterpret_cast<const char*>(ptr);
        }

        /**
         * @brief 设置堆分配标志
         * @param value 是否为堆分配
         */
        void set_heap_allocated(bool value) const SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            ptr = value ? (ptr | HEAP_MASK) : (ptr & ~HEAP_MASK);
            _type_name = reinterpret_cast<char *>(ptr);
        }

        /**
         * @brief 获取堆分配标志
         * @return bool 是否为堆分配
         */
        [[nodiscard]] bool is_heap_allocated() const SLOW_JSON_NOEXCEPT {
            return reinterpret_cast<uintptr_t>(_type_name) & HEAP_MASK;
        }

        /**
         * @brief 初始化序列化、移动和析构函数
         * @tparam U 值类型
         */
        template<typename U>
        void initialize_functions() SLOW_JSON_NOEXCEPT {
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
    };

    class key_to_index {
    private:
        std::map<std::string_view, std::size_t, std::less<>> index_map;

    public:
        /**
         * @brief 默认构造函数
         * @details 初始化空的键到索引映射
         */
        key_to_index() = default;

        /**
         * @brief 拷贝构造函数（禁用）
         * @details 禁用拷贝以防止意外复制
         */
        key_to_index(const key_to_index&) = delete;

        /**
         * @brief 拷贝赋值运算符（禁用）
         * @details 禁用拷贝赋值以防止意外复制
         */
        key_to_index& operator=(const key_to_index&) = delete;

        /**
         * @brief 移动构造函数
         * @details 转移键到索引映射
         */
        key_to_index(key_to_index&&) = default;

        /**
         * @brief 移动赋值运算符（禁用）
         * @details 禁用移动赋值以确保一致性
         */
        key_to_index& operator=(key_to_index&&) = delete;

        /**
         * @brief 插入键和索引
         * @param key 键
         * @param index 索引值
         * @details 将键映射到指定索引
         */
        void insert(const char* key, std::size_t index) SLOW_JSON_NOEXCEPT {
            index_map[key] = index;
        }

        /**
         * @brief 获取键对应的索引
         * @param key 键
         * @return std::size_t 索引值
         * @details 返回键对应的索引值
         */
        std::size_t at(const char* key) SLOW_JSON_NOEXCEPT {
            return index_map[key];
        }

        /**
         * @brief 检查是否包含键
         * @param key 键
         * @return bool 是否包含
         * @details 检查键是否存在于映射中
         */
        bool contains(const char* key) SLOW_JSON_NOEXCEPT {
            return index_map.contains(key);
        }

        /**
         * @brief 获取映射大小
         * @return std::size_t 键值对数量
         */
        [[nodiscard]] std::size_t size() const SLOW_JSON_NOEXCEPT {
            return index_map.size();
        }

        /**
         * @brief 检查映射是否为空
         * @return bool 是否为空
         */
        [[nodiscard]] bool empty() const SLOW_JSON_NOEXCEPT {
            return index_map.empty();
        }
    };

    /**
     * @brief 动态字典结构，用于存储键值对或包装值(例如dict["a"]产生的子字典对象)
     * @details 提供键值对的存储、序列化和反序列化功能，支持动态数据操作
     *          对于子字典对象，不会任何数据拷贝，也不应该析构任何数据
     *          使用=修改（例如dict["list"]={1,2,3}）会自动释放原有数据指针，是安全的操作
     */
    struct dict {
        friend struct pair;
        friend struct LoadFromDict<dict>;
        friend struct DumpToString<dict>;
        friend dict merge(dict&&, dict&&);

        //与下面pair内存布局一致，解决pair和dict交叉依赖的问题，后续预计通过分离定义和实现（hpp/tpp）来解决。
        struct _pair {
            const char *_key; ///< 键，字符串形式
            serializable_wrapper _value; ///< 值，序列化包装器形式
        };

        /**
         * @brief 构造函数，使用初始化列表构造根字典
         * @param data 初始化列表，包含键值对
         * @details 使用移动语义将键值对列表存储到字典中
         */
        dict(std::initializer_list<pair>&& data) :
        _type(serializable_wrapper::ROOT_DICT_TYPE),
        _key_to_index(nullptr),
        _data_ptr{nullptr}{
            new (&_data) std::vector<pair>(data);
        }

        /**
         * @brief 构造函数，使用可变参数模板构造根字典
         * @tparam K 键类型
         * @tparam V 值类型
         * @param data 键值对参数包
         * @details 将参数包中的键值对转换为pair对象并存储
         */
        template<typename... K, typename... V>
        constexpr dict(std::pair<K, V>&&... data)  // NOLINT(google-explicit-constructor)
                : _type(serializable_wrapper::ROOT_DICT_TYPE), _key_to_index(nullptr),_data_ptr{nullptr} {
            new (&_data) std::vector<pair>{{pair{std::move(data.first), std::move(data.second)}...}};
        }

        /**
         * @brief 拷贝构造函数（禁用）
         * @details 禁用拷贝构造函数以防止意外拷贝
         */
        dict(const dict&) = delete;

        /**
         * @brief 移动构造函数
         * @param other 源字典
         * @details 移动源字典的数据到新对象
         */
        dict(dict&& other) noexcept: _type(other._type), _key_to_index(nullptr), _data_ptr(nullptr) {
            assert_with_message(
                    (other._type == serializable_wrapper::ROOT_DICT_TYPE && _type == serializable_wrapper::ROOT_DICT_TYPE) ||
                    (other._type != serializable_wrapper::ROOT_DICT_TYPE && _type != serializable_wrapper::ROOT_DICT_TYPE),
                    "类型不匹配");
            if (other._type == serializable_wrapper::ROOT_DICT_TYPE) {
                _type = serializable_wrapper::ROOT_DICT_TYPE;
                new (&_data) std::vector<pair>(std::move(other._data));
                _key_to_index = other._key_to_index;
                other._key_to_index = nullptr;
            } else {
                _type = other._type;
                _data_ptr = other._data_ptr;
                _key_to_index = other._key_to_index;
                other._data_ptr = nullptr;
                other._key_to_index = nullptr;
            }
        }

        /**
         * @brief 析构函数
         * @details 释放键值对数据或包装值的资源
         */
        ~dict() {
            if (_type == serializable_wrapper::ROOT_DICT_TYPE) {
                _data.~vector();
                delete _key_to_index;
            }
        }

        /**
         * @brief 赋值运算符，绑定序列化包装器
         * @param value 序列化包装器
         * @return dict& 当前对象的引用
         * @details 清理当前资源，构造新的包装值
         */
        dict& operator=(serializable_wrapper&& value) SLOW_JSON_NOEXCEPT {
            assert_with_message(_type != serializable_wrapper::ROOT_DICT_TYPE,"根字典对象不允许此操作");
            _type = value.get_value_element_type();
            *_data_ptr = std::move(value);
            return *this;
        }

        /**
         * @brief 赋值运算符，绑定列表
         * @param value 序列化包装器列表
         * @return dict& 当前对象的引用
         */
        dict& operator=(std::vector<serializable_wrapper>&& value) SLOW_JSON_NOEXCEPT {
            assert_with_message(_type != serializable_wrapper::ROOT_DICT_TYPE,"根字典对象不允许此操作");
            _type = serializable_wrapper::LIST_TYPE;
            *_data_ptr = serializable_wrapper(std::move(value));
            return *this;
        }

        /**
         * @brief 赋值运算符，绑定字典
         * @param value 字典
         * @return dict& 当前对象的引用
         */
        dict& operator=(dict&& value) SLOW_JSON_NOEXCEPT {
            if (this != &value) {
                assert_with_message(_type != serializable_wrapper::ROOT_DICT_TYPE,"根字典对象不允许此操作");
                _type = serializable_wrapper::LIST_TYPE;
                *_data_ptr = serializable_wrapper(std::move(value));
                return *this;
            }
            return *this;
        }
        /**
         * @brief 检查是否包含指定键
         * @param key 键
         * @return bool 是否包含
         * @details 对于根字典，检查键值对；对于嵌套字典，委托给包装值
         */
        bool contains(const char* key) const SLOW_JSON_NOEXCEPT {
            if (!key) {
                return false;
            }
            if (_type == serializable_wrapper::ROOT_DICT_TYPE) {
                initialize_key_to_index();
                return _key_to_index->contains(key);
            } else if (_type == serializable_wrapper::DICT_TYPE) {
                return static_cast<dict*>(_data_ptr->value())->contains(key);
            }
            return false;
        }

        /**
         * @brief 检查字典/子字典是否为空
         * @return bool 是否为空
         * @details 和JSON的空对应，明确的数据为空(null)返回true
         * @@note 并不是字段或者数组为空的意思
         */
        bool empty() const SLOW_JSON_NOEXCEPT {
            if (_type == serializable_wrapper::FUNDAMENTAL_TYPE){
                auto&data=*(serializable_wrapper*)_data_ptr;
                slow_json::Buffer buffer;
                data.dump_fn(buffer);
                return buffer.string()=="null";
            }
            return false;
        }

        /**
         * @brief 访问字典元素
         * @param key 键
         * @return dict 对应的子字典或包装值
         * @throws 当键不存在或类型不正确时抛出异常
         */
        template<typename T, typename = std::enable_if_t<(std::is_same_v<T,std::nullptr_t> || std::is_pointer_v<T>) && !std::is_integral_v<T>>>
        dict operator[](T key)SLOW_JSON_NOEXCEPT {
            auto value_fn = [](pair& p) { return &reinterpret_cast<_pair*>(&p)->_value; };
            assert_with_message(key!=nullptr, "key为空指针");
            if (_type == serializable_wrapper::ROOT_DICT_TYPE) {
                initialize_key_to_index();
                assert_with_message(_key_to_index->contains(key), "字典中不存在该字段:'%s'", (char*)key);
                return dict{value_fn(_data[_key_to_index->at(key)])};
            } else if (_type == serializable_wrapper::DICT_TYPE) {
                return static_cast<dict*>(_data_ptr->value())->operator[](key);
            }
            assert_with_message(false, "非字典类型，无法通过键访问数据");
            return dict{nullptr};
        }

        /**
         * @brief 访问列表元素
         * @param index 索引
         * @return dict 对应的子字典或包装值
         * @throws 当索引越界或类型不正确时抛出异常
         */
        dict operator[](std::size_t index)SLOW_JSON_NOEXCEPT{
            assert_with_message(_type == serializable_wrapper::LIST_TYPE, "非列表类型，无法通过整数下标访问数据");
            auto& list_data = *static_cast<std::vector<serializable_wrapper>*>(_data_ptr->value());
            assert_with_message(index < list_data.size(), "数组越界访问:%zu >= %zu", index, list_data.size());
            return dict{&list_data[index]};
        }

        /**
         * @brief 检查是否为指定类型
         * @tparam T 目标类型
         * @return bool 是否匹配
         * @details 仅对基本类型有效，字典和列表返回false
         */
        template<typename T>
        bool as_type()const SLOW_JSON_NOEXCEPT{
            if (_type != serializable_wrapper::FUNDAMENTAL_TYPE) {
                return false;
            }
            constexpr auto type_name = std::string_view{type_name_v<T>.str};
            return type_name == _data_ptr->type_name();
        }

        /**
         * @brief 转换为指定类型
         * @tparam T 目标类型
         * @return T& 转换后的引用
         * @throws 当类型不匹配时抛出异常
         */
        template<typename T>
        T& cast()const SLOW_JSON_NOEXCEPT{
            assert_with_message(_type == serializable_wrapper::FUNDAMENTAL_TYPE, "非基础类型，无法转换");
            constexpr auto type_name = std::string_view{type_name_v<T>.str};
            assert_with_message(type_name == _data_ptr->type_name(), "类型不正确，预期为`%s`，实际为`%s`",
                                _data_ptr->type_name().data(), type_name.data());
            return *static_cast<T*>(_data_ptr->value());
        }

        /**
         * @brief 检查是否为基本类型
         * @return bool 是否为基本类型
         */
        bool is_fundamental() const noexcept{
            return _type == serializable_wrapper::FUNDAMENTAL_TYPE;
        }

        /**
         * @brief 检查是否为列表
         * @return bool 是否为列表
         */
        bool is_array() const noexcept{
            return _type == serializable_wrapper::LIST_TYPE;
        }

        /**
         * @brief 检查是否为字典
         * @return bool 是否为字典
         * @details 对嵌套字典和根字典均返回true
         */
        bool is_dict() const noexcept{
            return _type == serializable_wrapper::DICT_TYPE || _type == serializable_wrapper::ROOT_DICT_TYPE;
        }

        /**
         * @brief 获取元素数量
         * @return std::size_t 元素数量
         * @throws 当类型不正确时抛出异常
         */
        std::size_t size()const SLOW_JSON_NOEXCEPT{
            assert_with_message(_type != serializable_wrapper::FUNDAMENTAL_TYPE, "非列表或字典类型，无法通过整数下标访问数据");
            if(_type == serializable_wrapper::LIST_TYPE) {
                return static_cast<std::vector<serializable_wrapper> *>(_data_ptr->value())->size();
            }else if(_type == serializable_wrapper::DICT_TYPE) {
                return static_cast<dict *>(_data_ptr->value())->size();
            }else if(_type == serializable_wrapper::ROOT_DICT_TYPE) {
                initialize_key_to_index();
                return _data.size();
            };
            return 0;
        }

        /**
         * @brief 获取类型名称
         * @return std::string_view 类型名称
         * @details 对于根字典，返回固定字符串；其他类型委托给包装值
         */
        [[nodiscard]] std::string_view type_name() const SLOW_JSON_NOEXCEPT {
            if (_type == serializable_wrapper::ROOT_DICT_TYPE) {
                return std::string_view{type_name_v<dict>.str};
            }
            return _data_ptr->type_name();
        }

        /**
         * @brief 转换为STL无序映射
         * @return std::unordered_map<const char*, dict> 键值对映射
         * @throws 当类型不正确时抛出异常
         */
        std::unordered_map<const char*, dict> as_dict() {
            auto value_fn = [](pair& p) { return &reinterpret_cast<_pair*>(&p)->_value; };
            assert_with_message(is_dict(), "非字典类型，无法转换为 std::unordered_map");
            std::unordered_map<const char*, dict> result;
            if (_type == serializable_wrapper::ROOT_DICT_TYPE) {
                for (auto& p : _data) {
                    result.emplace(_pair_key_fn(p), dict{value_fn(p)});
                }
            } else {
                auto& d = *static_cast<dict*>(_data_ptr->value());
                return d.as_dict();
            }
            return result;
        }

        /**
         * @brief 转换为STL向量
         * @return std::vector<dict> 元素向量
         * @throws 当类型不正确时抛出异常
         */
        std::vector<dict> as_list() {
            assert_with_message(_type == serializable_wrapper::LIST_TYPE, "非列表类型，无法转换为 std::vector");
            auto& list_data = *static_cast<std::vector<serializable_wrapper>*>(_data_ptr->value());
            std::vector<dict> result;
            result.reserve(list_data.size());
            for (auto& item : list_data) {
                result.emplace_back(dict{const_cast<serializable_wrapper*>(&item)});
            }
            return result;
        }

    private:
        /**
         * @brief 内部构造函数，用于构造子节点
         * @param data 序列化包装器指针
         * @details 仅用于内部构造嵌套字典或值
         */
        explicit dict(serializable_wrapper* data) :
        _type(data ? data->get_value_element_type() : serializable_wrapper::FUNDAMENTAL_TYPE),
        _key_to_index(nullptr) {
            _data_ptr = data;
        }

        /**
         * @brief 初始化键到索引映射
         * @details 延迟初始化_key_to_index以提高性能
         */
        void initialize_key_to_index()const SLOW_JSON_NOEXCEPT {
            if (!_key_to_index && _type == serializable_wrapper::ROOT_DICT_TYPE) {
                _key_to_index = new key_to_index{};
                for (std::size_t index = 0; auto& it : _data) {
                    _key_to_index->insert(_pair_key_fn(it), index++);
                }
            }
        }

        /**
         * @brief 获取键值对的键
         * @param p 键值对
         * @return const char* 键
         */
        static const char* _pair_key_fn(const pair& p) {
            return reinterpret_cast<const _pair*>(&p)->_key;
        }

        serializable_wrapper::ElementType _type; ///< 存储类型
        mutable key_to_index* _key_to_index; ///< 键到索引映射，延迟初始化
        union {
            std::vector<pair> _data; ///< 根字典的键值对
            serializable_wrapper* _data_ptr; ///< 其他类型的包装值
        };
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

namespace slow_json {
    using details::pair;
    using details::dict;
    using list = std::vector<details::serializable_wrapper>;
}
#endif // SLOWJSON_DICT_HPP