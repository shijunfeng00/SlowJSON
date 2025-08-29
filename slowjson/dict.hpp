
/**
 * @file dict.hpp
 * @brief 实现动态字典结构及相关序列化功能
 * @author shijunfeng
 * @date 2025/3/4
 *
 * ======================== 核心组件说明 ========================
 * 本文件实现了动态字典结构，支持JSON格式的序列化/反序列化操作。
 * 主要组件：
 * 1. field_wrapper - 封装类成员指针，提供运行时动态访问
 * 2. serializable_wrapper - 通用值包装器，支持多种存储类型
 * 3. key_to_index - 键到索引的映射结构
 * 4. dict - 核心字典结构，支持嵌套字典和列表
 * 5. pair - 键值对基础单元
 *
 * ======================== 设计亮点 ========================
 * 1. 内存优化：
 *   - StaticString<>::str要求至少8对齐以保证可以存储元数据
 *   - serializable_wrapper:
 *     * 小对象(≤32B)使用内部缓冲区(SOO)
 *     * 大对象使用堆分配
 *     * 类型名称指针低6位存储元数据(值类型2位+堆分配标志1位+基础类型3位)
 *
 *   - dict:
 *     * _key_to_index指针低4位存储（key_to_index强制要求16对齐）:
 *       - 值类型信息(2位)
 *       - 键复制标志(1位)
 *       - 堆分配标志(1位)
 *     * 将这些元数据嵌入到指针地址中，大幅减少结构体大小，提高缓存局部性
 *
 * 2. 高效序列化：
 *   - 通过函数指针实现多态行为（不基于虚函数）
 *   - 延迟初始化键索引映射(key_to_index)
 *
 * 3. 灵活存储：
 *   - 支持基础类型、列表、嵌套字典、大量STL容器及容器适配器
 *
 * ======================== 使用示例 ========================
 * 1. 创建字典：
 *    dict d = {
 *        {"name", "John"},
 *        {"age", 30},
 *        {"classes",{"Chinese","English","Math"}},
 *        {"scores", std::vector{90, 85, 95}},
 *        {"details",{
 *            {"location","Sichuan"},
 *            {"device","Mobile Phone"}
 *        }},
 *        {"others",nullptr}
 *    };
 *
 * 2. 访问元素：
 *    auto name = d["name"].cast<std::string>();
 *    auto device = d["details"]["device"].cast<const char*>();
 *
 * 3. 类型检查：
 *    if (d["age"].is_fundamental()) {
 *        // 处理基础类型
 *    }
 *  4. 遍历
 *    for(auto&[k,v]:d["details"].as_dict()){
 *        std::cout<<k<<" ";
 *        slow_json::visit(v,[](const auto&&value){std::cout<<v<<std::endl;});
 *    }
 *
 */

#ifndef SLOWJSON_DICT_HPP
#define SLOWJSON_DICT_HPP
#include <vector>
#include <map>
#include <unordered_map>
#include "load_from_dict_interface.hpp"
#include "key_to_index.hpp"

namespace slow_json::details {
    struct dict;
    struct serializable_wrapper;

    /**
     * @brief 基础类型枚举，用于反序列化时记录具体类型
     */
    enum BaseType {
        NOT_FUNDAMENTAL_TYPE = 0, // 非基本类型
        NULL_TYPE = 1,            // null
        INT64_TYPE = 2,           // int64_t
        UINT64_TYPE = 3,          // uint64_t
        DOUBLE_TYPE = 4,          // double
        BOOL_TYPE = 5,            // bool
        STRING_TYPE = 6           // std::string
    };

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
        enum ValueType {
            FUNDAMENTAL_TYPE = 0,  // 原始C++对象 00
            LIST_TYPE = 1,         // 列表 01
            DICT_TYPE = 2,         // 嵌套字典 10
            ROOT_DICT_TYPE = 3     // 根字典 11
        };

    public:
        friend struct DumpToString<details::serializable_wrapper>;
        friend struct LoadFromDict<details::serializable_wrapper>;
        friend struct DictHandler;
        friend struct dict;
        /**
         * @brief 构造函数，绑定任意可序列化值（移动构造）
         * @tparam T 值类型，需满足可序列化要求且非成员指针
         * @param value 要包装的值
         * @details 小对象（<=32字节）存储在内部缓冲区，大对象分配在堆上，BaseType 初始化为 NOT_FUNDAMENTAL_TYPE
         */
        template<typename T>
        requires(!std::is_same_v<std::decay_t<T>,serializable_wrapper>)
        constexpr serializable_wrapper(T&& value) SLOW_JSON_NOEXCEPT { // NOLINT
            using Raw = std::decay_t<T>;
            using U   = std::conditional_t<std::is_member_pointer_v<Raw>, field_wrapper, Raw>;
            using V   = std::conditional_t<std::is_member_pointer_v<Raw>,field_wrapper,T>;
            _type_name = slow_json::type_name_v<U>.str;
            assert_with_message((uintptr_t)_type_name % 64 == 0, "指针地址非 64 对齐");
            set_value_type<U>();
            set_base_type(NOT_FUNDAMENTAL_TYPE);
            if constexpr (sizeof(U) <= buffer_size && alignof(U) <= alignof(std::max_align_t)) {
                new(&_buffer) U(std::forward<V>(value));
                set_heap_allocated(false);
            } else {
                _buffer_ptr = new U(std::forward<V>(value));
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
                  _move_or_delete_fn(other._move_or_delete_fn)
        {
            set_heap_allocated(other.is_heap_allocated());
            if (is_heap_allocated()) {
                _buffer_ptr = other._buffer_ptr;
                other._buffer_ptr = nullptr;
            } else {
                other._move_or_delete_fn(&_buffer, const_cast<void*>(static_cast<const void*>(&other._buffer)), false, false);
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
                // 清理当前资源（删除操作）
                if (_move_or_delete_fn) {
                    void* target = is_heap_allocated() ? _buffer_ptr : static_cast<void*>(&_buffer);
                    _move_or_delete_fn(nullptr, target, true, is_heap_allocated());
                }

                // 转移资源
                _type_name = other._type_name;
                _dump_fn = other._dump_fn;
                _move_or_delete_fn = other._move_or_delete_fn;
                set_heap_allocated(other.is_heap_allocated());
                if (is_heap_allocated()) {
                    _buffer_ptr = other._buffer_ptr;
                    other._buffer_ptr = nullptr;
                } else {
                    other._move_or_delete_fn(&_buffer, &other._buffer, false, false);
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
                  _move_or_delete_fn(other._move_or_delete_fn) {
            set_heap_allocated(other.is_heap_allocated());
            if (is_heap_allocated()) {
                _buffer_ptr = other._buffer_ptr;
                other._buffer_ptr = nullptr;
            } else {
                other._move_or_delete_fn(&_buffer, &other._buffer, false, false);
            }
        }

        /**
         * @brief 析构函数
         * @details 根据堆分配标志释放堆内存或调用小对象的析构函数
         */
        ~serializable_wrapper() {
            if (_move_or_delete_fn) {
                void* target = is_heap_allocated() ? _buffer_ptr : static_cast<void*>(&_buffer);
                _move_or_delete_fn(nullptr, target, true, is_heap_allocated());
            }
        }

        /**
         * @brief 执行序列化
         * @param buffer 输出缓冲区
         * @details 调用存储的序列化函数，处理小对象或大对象
         */
        void dump_fn(slow_json::Buffer &buffer) const SLOW_JSON_NOEXCEPT {
            void *ptr = is_heap_allocated() ? _buffer_ptr : (void *)&_buffer;
            assert_with_message(ptr,"ptr地址不能为空");
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
            ptr &= CLEAR_MASK;  // 清除低6位
            assert_with_message(ptr % 8 == 0, "指针地址非8对齐");
            return std::string_view{reinterpret_cast<const char *>(ptr)};
        }

        /**
         * @brief 获取存储的值类型信息
         * @return ValueType 值类型枚举值
         * @details 返回基本类型、列表、字典或根字典的类型
         */
        [[nodiscard]] ValueType value_type() const SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            return static_cast<ValueType>((ptr & VALUE_TYPE_MASK) >> 1);
        }

        /**
         * @brief 获取基础类型信息
         * @return BaseType 基础类型枚举值
         * @details 从 _type_name 指针的低6位（第4-6位）提取基础类型信息
         */
        [[nodiscard]] BaseType get_base_type() const SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            return static_cast<BaseType>((ptr & BASE_TYPE_MASK) >> 3);
        }

    private:
        static constexpr std::size_t buffer_size = 32; ///< 小对象缓冲区大小
        static constexpr uintptr_t HEAP_MASK = 0x1;    // 最低位 - 堆分配标志
        static constexpr uintptr_t VALUE_TYPE_MASK = 0x6; // 第2-3位 - 值类型信息 (0b0110)
        static constexpr uintptr_t BASE_TYPE_MASK = 0x38; // 第4-6位 - 基础类型信息 (0b111000)
        static constexpr uintptr_t CLEAR_MASK = ~0x3F; // 清除低6位的掩码

        union {
            mutable void *_buffer_ptr; ///< 大对象指针，指向堆内存
            alignas(16) char _buffer[buffer_size]; ///< 小对象缓冲区
        };
        mutable const char *_type_name; ///< 类型名称，内存地址64对齐，末尾6位存储值类型、堆分配信息和基础类型
        void (*_dump_fn)(slow_json::Buffer &, void *) = nullptr; ///< 序列化函数指针
        void (*_move_or_delete_fn)(void*, void*, bool, bool) = nullptr;

        /**
         * @brief 设置值类型信息
         * @tparam U 实际存储的类型
         * @details 根据类型设置值类型标志位
         */
        template<typename U>
        void set_value_type() SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            // 清除原有值类型信息
            ptr &= ~VALUE_TYPE_MASK;

            // 设置新值类型信息
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
         * @brief 设置基础类型信息
         * @param base_type 基础类型枚举值
         * @details 将基础类型信息（3位）嵌入到 _type_name 指针的低6位（第4-6位）
         */
        void set_base_type(BaseType base_type) SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            ptr &= ~BASE_TYPE_MASK; // 清除原有基础类型信息
            ptr |= (static_cast<uintptr_t>(base_type) << 3); // 设置新基础类型信息
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
            _move_or_delete_fn = [](void *dest, void *src, bool move_type, bool is_heap_allocated) {
                if (!move_type) {
                    // 移动操作：构造新对象到dest
                    if constexpr (std::is_trivially_copyable_v<U>) {
                        std::memcpy(dest, src, sizeof(U));
                    } else if constexpr (std::is_move_constructible_v<U>) {
                        new(dest) U(std::move(*static_cast<U *>(src)));
                    } else if constexpr (std::is_copy_constructible_v<U>) {
                        new(dest) U(*static_cast<U *>(src));
                    } else {
                        assert_with_message(false, "类型既不可复制也不可移动: %s", slow_json::type_name_v<U>.str);
                    }
                } else {
                    // 删除操作：根据堆分配标志销毁对象
                    if (is_heap_allocated) {
                        delete static_cast<U *>(src);
                    } else {
                        static_cast<U *>(src)->~U();
                    }
                }
            };
        }
    };

    /**
      * @brief 动态字典结构，用于存储键值对或包装值
      * @details 提供键值对的存储、序列化和反序列化功能，支持动态数据操作。
      *          对于子字典对象，不会进行数据拷贝，也不应析构任何数据。
      *          使用赋值操作（如 dict["list"]={1,2,3}）会自动释放原有数据指针，操作安全。
      *          优化设计：将 `_type`（2 位）、`_copied`（1 位）和 `_is_heap_allocated`（1 位）嵌入到 `_key_to_index` 指针的低 4 位中，
      *          利用指针 16 字节对齐的特性（低 4 位为 0），减少内存占用。
      *          此优化将结构体大小减少，移除 bool _is_heap_allocated，提高缓存局部性。
      *          位运算开销较小，相比于缓存未命中，额外新增的开销是可以接受的。
      */
    struct dict {
        friend struct pair;
        friend struct LoadFromDict<dict>;
        friend struct DumpToString<dict>;
        friend struct DictHandler;
        friend dict merge(dict&&, dict&&);

        // 与 pair 内存布局一致，解决 pair 和 dict 交叉依赖问题
        struct _pair {
            const char *_key; ///< 键，字符串形式
            serializable_wrapper _value; ///< 值，序列化包装器形式
        };

        static constexpr uintptr_t VALUE_TYPE_MASK = 0x6;        ///< 中间两位 - 值类型信息 (0b0110)
        static constexpr uintptr_t COPIED_MASK = 0x1;      ///< 最低位 - 复制标志 (0b0001)
        static constexpr uintptr_t HEAP_ALLOCATED_MASK = 0x8; ///< 第 4 位 - 堆分配标志 (0b1000)
        static constexpr uintptr_t CLEAR_MASK = ~0xF;      ///< 清除低 4 位的掩码

        /**
         * @brief 构造函数，使用初始化列表构造根字典
         * @param data 初始化列表，包含键值对
         * @details 使用移动语义将键值对列表存储到字典中，设置类型为 ROOT_DICT_TYPE
         */
        dict(std::initializer_list<pair>&& data) :
                _key_to_index(nullptr) {
            set_value_type(serializable_wrapper::ROOT_DICT_TYPE);
            set_copied(false);
            set_heap_allocated(false);
            new (&_data) std::vector<pair>(data);
        }

        /**
         * @brief 构造函数，使用可变参数模板构造根字典
         * @tparam K 键类型
         * @tparam V 值类型
         * @param data 键值对参数包
         * @details 将参数包中的键值对转换为 pair 对象并存储，设置类型为 ROOT_DICT_TYPE
         */
        template<typename... K, typename... V>
        requires ((concepts::static_string<K> && ...) ||
                  (concepts::string<K> && ...))
        constexpr dict(std::pair<K, V>&&... data)   // NOLINT(google-explicit-constructor)
                : _key_to_index(nullptr) {
            set_value_type(serializable_wrapper::ROOT_DICT_TYPE);
            set_copied(false);
            set_heap_allocated(false);
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
         * @details 移动源字典的数据到新对象，转移 _key_to_index 指针和元数据
         */
        dict(dict&& other) noexcept :
                _key_to_index(nullptr) {
            auto other_type = other.value_type();
            set_value_type(other_type);
            set_copied(other.get_copied());
            set_heap_allocated(other.is_heap_allocated());
            if (other_type == serializable_wrapper::ROOT_DICT_TYPE) {
                new (&_data) std::vector<pair>(std::move(other._data));
                set_key_to_index(other.get_key_to_index());
                other.set_key_to_index(nullptr);
            } else {
                _data_ptr = other._data_ptr;
                set_key_to_index(other.get_key_to_index());
                other._data_ptr = nullptr;
                other.set_key_to_index(nullptr);
            }
            other.set_heap_allocated(false);
        }

        /**
         * @brief 析构函数
         * @details 释放键值对数据或包装值的资源，清理 _key_to_index
         */
        ~dict() {
            auto type = value_type();
            if (type == serializable_wrapper::ROOT_DICT_TYPE || type == serializable_wrapper::DICT_TYPE) {
                if (get_copied()) {
                    for (auto &it : _data) {
                        delete[] reinterpret_cast<_pair*>(&it)->_key;
                        reinterpret_cast<_pair*>(&it)->_key = nullptr;
                    }
                }
            }
            if (type == serializable_wrapper::ROOT_DICT_TYPE) {
                _data.~vector();
                delete get_key_to_index();
            } else if (is_heap_allocated()) {
                delete _data_ptr;
                delete get_key_to_index();
            }
        }

        /**
         * @brief 赋值运算符，绑定序列化包装器
         * @param value 序列化包装器
         * @return dict& 当前对象的引用
         * @details 清理当前资源，构造新的包装值，更新类型
         */
        dict& operator=(serializable_wrapper&& value) SLOW_JSON_NOEXCEPT {
            auto current_type = value_type();
            if (current_type == serializable_wrapper::ROOT_DICT_TYPE) {
                // 当前为根字典，清理 _data 和 _key_to_index
                _data.~vector();
                delete get_key_to_index();
                set_key_to_index(nullptr);
                // 分配新的堆内存
                _data_ptr = new serializable_wrapper{std::move(value)};
                set_heap_allocated(true);
                set_value_type(serializable_wrapper::FUNDAMENTAL_TYPE);
                set_copied(false);
            } else {
                // 非根字典，直接修改 *_data_ptr
                *_data_ptr = std::move(value);
                set_value_type(serializable_wrapper::FUNDAMENTAL_TYPE);
                set_copied(false);
            }
            return *this;
        }

        /**
         * @brief 赋值运算符，绑定列表
         * @param value 序列化包装器列表
         * @return dict& 当前对象的引用
         * @details 清理当前资源，构造新的列表，更新类型
         */
        dict& operator=(std::vector<serializable_wrapper>&& value) SLOW_JSON_NOEXCEPT {
            auto current_type = value_type();
            if (current_type == serializable_wrapper::ROOT_DICT_TYPE) {
                // 当前为根字典，清理 _data 和 _key_to_index
                _data.~vector();
                delete get_key_to_index();
                set_key_to_index(nullptr);
                // 分配新的堆内存
                _data_ptr = new serializable_wrapper{std::move(value)};
                set_heap_allocated(true);
                set_value_type(serializable_wrapper::LIST_TYPE);
                set_copied(false);
            } else {
                // 非根字典，直接修改 *_data_ptr
                *_data_ptr = serializable_wrapper(std::move(value));
                set_value_type(serializable_wrapper::LIST_TYPE);
                set_copied(false);
            }
            return *this;
        }

        /**
         * @brief 赋值运算符，绑定字典
         * @param value 字典
         * @return dict& 当前对象的引用
         * @details 清理当前资源，移动字典数据，更新类型
         */
        dict& operator=(dict&& other) SLOW_JSON_NOEXCEPT {
            if (this != &other) {
                if (value_type() == serializable_wrapper::ROOT_DICT_TYPE) {
                    // 自己是根字典，情况复杂
                    if (other.value_type() == serializable_wrapper::ROOT_DICT_TYPE) {
                        // 对面也是根字典，执行移动构造
                        if (get_copied()) {
                            for (auto &it : _data) {
                                delete[] reinterpret_cast<_pair*>(&it)->_key;
                                reinterpret_cast<_pair*>(&it)->_key = nullptr;
                            }
                        }
                        delete get_key_to_index();
                        set_key_to_index(other.get_key_to_index());
                        _data = std::move(other._data);
                        other.set_key_to_index(nullptr);
                        if (other.get_copied()) {
                            other.set_copied(false);
                            set_copied(true);
                        } else {
                            set_copied(false); // 接管所有权，避免 double free
                        }
                        set_value_type(serializable_wrapper::ROOT_DICT_TYPE);
                        set_heap_allocated(false);
                    } else {
                        // 对面非根字典，析构 _data 换 _data_ptr
                        _data.~vector();
                        delete get_key_to_index();
                        set_key_to_index(nullptr);
                        set_copied(false);
                        set_value_type(other.value_type());
                        _data_ptr = new serializable_wrapper(std::move(other));
                        set_heap_allocated(true);
                    }
                } else {
                    // 自己非根字典
                    auto type = other.value_type();
                    set_value_type(type == serializable_wrapper::ROOT_DICT_TYPE ? serializable_wrapper::DICT_TYPE : type);
                    set_key_to_index(other.get_key_to_index());
                    other.set_key_to_index(nullptr);
                    if (other.get_copied()) {
                        other.set_copied(false);
                        set_copied(true);
                    } else {
                        set_copied(false); // 接管所有权，避免 double free
                    }
                    *_data_ptr = serializable_wrapper(std::move(other));
                    set_heap_allocated(is_heap_allocated() || other.is_heap_allocated());
                }
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
            auto type = value_type();
            if (type == serializable_wrapper::ROOT_DICT_TYPE) {
                initialize_key_to_index();
                return get_key_to_index()->contains(key);
            } else if (type == serializable_wrapper::DICT_TYPE) {
                return static_cast<dict*>(_data_ptr->value())->contains(key);
            }
            return false;
        }

        /**
         * @brief 检查字典/子字典是否为空
         * @return bool 是否为空
         * @details 和 JSON 的空对应，明确的数据为空 (null) 返回 true
         */
        bool is_null() const SLOW_JSON_NOEXCEPT {
            if (value_type() == serializable_wrapper::FUNDAMENTAL_TYPE) {
                auto& data = *(serializable_wrapper*)_data_ptr;
                slow_json::Buffer buffer;
                data.dump_fn(buffer);
                return buffer.string() == "null";
            }
            return false;
        }

        /**
         * @brief 访问字典元素
         * @param key 键
         * @return dict 对应的子字典或包装值
         * @throws 当键不存在或类型不正确时抛出异常
         */
        template<typename T, typename = std::enable_if_t<(std::is_same_v<T, std::nullptr_t> || std::is_pointer_v<T>) && !std::is_integral_v<T>>>
        dict operator[](T key) SLOW_JSON_NOEXCEPT {
            auto value_fn = [](pair& p) { return &reinterpret_cast<_pair*>(&p)->_value; };
            assert_with_message(key != nullptr, "key 为空指针");
            auto type = value_type();
            if (type == serializable_wrapper::ROOT_DICT_TYPE) {
                initialize_key_to_index();
                assert_with_message(get_key_to_index()->contains(key), "字典中不存在该字段:'%s'", (char*)key);
                return dict{value_fn(_data[get_key_to_index()->at(key)])};
            } else if (type == serializable_wrapper::DICT_TYPE) {
                return static_cast<dict*>(_data_ptr->value())->operator[](key);
            }
            assert_with_message(false, "非字典类型，无法通过键访问数据");
            return dict{nullptr};
        }

        /**
         * @brief 访问字典元素
         * @param key 键
         * @return dict 对应的子字典或包装值
         * @throws 当键不存在或类型不正确时抛出异常
         */
        template<typename T, typename = std::enable_if_t<(std::is_same_v<T, std::nullptr_t> || std::is_pointer_v<T>) && !std::is_integral_v<T>>>
        dict operator[](T key)const SLOW_JSON_NOEXCEPT {
            auto value_fn = [](const pair& p) { return &reinterpret_cast<const _pair*>(&p)->_value; };
            assert_with_message(key != nullptr, "key 为空指针");
            auto type = value_type();
            if (type == serializable_wrapper::ROOT_DICT_TYPE) {
                initialize_key_to_index();
                assert_with_message(get_key_to_index()->contains(key), "字典中不存在该字段:'%s'", (char*)key);
                return dict{value_fn(_data[get_key_to_index()->at(key)])};
            } else if (type == serializable_wrapper::DICT_TYPE) {
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
        dict operator[](std::size_t index) const SLOW_JSON_NOEXCEPT {
            assert_with_message(value_type() == serializable_wrapper::LIST_TYPE, "非列表类型，无法通过整数下标访问数据");
            auto& list_data = *static_cast<std::vector<serializable_wrapper>*>(_data_ptr->value());
            assert_with_message(index < list_data.size(), "数组越界访问:%zu >= %zu", index, list_data.size());
            return dict{const_cast<serializable_wrapper*>(&list_data[index])};
        }

        /**
         * @brief 检查是否为指定类型
         * @tparam T 目标类型
         * @return bool 是否匹配
         * @details 仅对基本类型有效，字典和列表返回 false
         */
        template<typename T>
        bool as_type() const SLOW_JSON_NOEXCEPT {
            if (value_type() != serializable_wrapper::FUNDAMENTAL_TYPE) {
                return false;
            }
            return _data_ptr->type_name().data()==type_name_v<T>.str;
        }

        /**
         * @brief 转换为指定类型
         * @tparam T 目标类型
         * @return T& 转换后的引用
         * @throws 当类型不匹配时抛出异常
         */
        template<typename T>
        T& cast() const SLOW_JSON_NOEXCEPT {
            assert_with_message(value_type() == serializable_wrapper::FUNDAMENTAL_TYPE, "非基础类型，无法转换");
            constexpr auto type_name = std::string_view{type_name_v<T>.str};
            assert_with_message(type_name == _data_ptr->type_name(), "类型不正确，预期为`%s`，实际为`%s`",
                                _data_ptr->type_name().data(), type_name.data());
            return *static_cast<T*>(_data_ptr->value());
        }

        /**
         * @brief 检查是否为基本类型
         * @return bool 是否为基本类型
         */
        bool is_fundamental() const noexcept {
            return value_type() == serializable_wrapper::FUNDAMENTAL_TYPE;
        }

        /**
         * @brief 检查是否为列表
         * @return bool 是否为列表
         */
        bool is_array() const noexcept {
            return value_type() == serializable_wrapper::LIST_TYPE;
        }

        /**
         * @brief 检查是否为字典
         * @return bool 是否为字典
         * @details 对嵌套字典和根字典均返回 true
         */
        bool is_dict() const noexcept {
            auto type = value_type();
            return type == serializable_wrapper::DICT_TYPE || type == serializable_wrapper::ROOT_DICT_TYPE;
        }

        /**
         * @brief 获取元素数量
         * @return std::size_t 元素数量
         * @throws 当类型不正确时抛出异常
         */
        std::size_t size() const SLOW_JSON_NOEXCEPT {
            auto type = value_type();
            assert_with_message(type != serializable_wrapper::FUNDAMENTAL_TYPE, "非列表或字典类型，无法通过整数下标访问数据");
            if (type == serializable_wrapper::LIST_TYPE) {
                return static_cast<std::vector<serializable_wrapper> *>(_data_ptr->value())->size();
            } else if (type == serializable_wrapper::DICT_TYPE) {
                return static_cast<dict *>(_data_ptr->value())->size();
            } else if (type == serializable_wrapper::ROOT_DICT_TYPE) {
                initialize_key_to_index();
                return _data.size();
            }
            return 0;
        }

        /**
         * @brief 获取类型名称
         * @return std::string_view 类型名称
         * @details 对于根字典，返回固定字符串；其他类型委托给包装值
         */
        [[nodiscard]] std::string_view type_name() const SLOW_JSON_NOEXCEPT {
            if (value_type() == serializable_wrapper::ROOT_DICT_TYPE) {
                return std::string_view{type_name_v<dict>.str};
            }
            return _data_ptr->type_name();
        }

        /**
         * @brief 转换为 STL 无序映射
         * @return std::unordered_map<const char*, dict> 键值对映射
         * @throws 当类型不正确时抛出异常
         */
        std::unordered_map<const char*, dict> as_dict() {
            auto value_fn = [](pair& p) { return &reinterpret_cast<_pair*>(&p)->_value; };
            assert_with_message(is_dict(), "非字典类型，无法转换为 std::unordered_map");
            std::unordered_map<const char*, dict> result;
            auto type = value_type();
            if (type == serializable_wrapper::ROOT_DICT_TYPE) {
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
         * @brief 转换为 STL 向量
         * @return std::vector<dict> 元素向量
         * @throws 当类型不正确时抛出异常
         */
        std::vector<dict> as_list() {
            assert_with_message(value_type() == serializable_wrapper::LIST_TYPE, "非列表类型，无法转换为 std::vector");
            auto& list_data = *static_cast<std::vector<serializable_wrapper>*>(_data_ptr->value());
            std::vector<dict> result;
            result.reserve(list_data.size());
            for (auto& item : list_data) {
                result.emplace_back(dict{const_cast<serializable_wrapper*>(&item)});
            }
            return result;
        }

        /**
         * @brief 复制键字符串
         * @details 为每个键值对的键创建副本，设置复制标志
         */
        void copy_key() {
            assert_with_message(!get_copied(), "重复调用 发生内存泄漏");
            assert_with_message(value_type() == serializable_wrapper::ROOT_DICT_TYPE, "不正确的类型");
            for (auto& it : _data) {
                const char* key = _pair_key_fn(it);
                auto size = strlen(key) + 1;
                auto key_cp = new char[size];
                memcpy(key_cp, key, size);
                reinterpret_cast<_pair*>(&it)->_key = key_cp;
            }
            set_copied(true);
        }

        /**
         * @brief 提取当前 dict 对象的值并转移所有权
         * @return dict 包含提取值的字典
         * @throws std::runtime_error 当类型不正确或无法提取时抛出异常
         * @details 从当前 dict 中提取值（serializable_wrapper 或 pair::_value），转移所有权到返回的 dict，
         *          原值置为 null（serializable_wrapper{nullptr}）。支持 ROOT_DICT_TYPE、DICT_TYPE、LIST_TYPE 和 FUNDAMENTAL_TYPE。
         *          对于 ROOT_DICT_TYPE，需通过调用上下文（如 operator[]）提供键。
         */
        dict extract() {
            auto type = value_type();
            if (type == serializable_wrapper::ROOT_DICT_TYPE) {
                throw std::runtime_error("根字典类型无法直接提取，需通过 operator[] 指定键");
            }

            if (type == serializable_wrapper::DICT_TYPE || type == serializable_wrapper::FUNDAMENTAL_TYPE || type == serializable_wrapper::LIST_TYPE) {
                assert_with_message(_data_ptr != nullptr, "数据指针为空，无法提取");
                // 创建新 dict，转移 _data_ptr 的 serializable_wrapper
                dict result{std::move(*_data_ptr)};
                // 原 _data_ptr 置为 null
                *_data_ptr = serializable_wrapper{nullptr};
                // 确保新 dict 的 _data_ptr 是堆分配的
                result.set_heap_allocated(true);
                return result;
            }

            throw std::runtime_error("未知类型，无法提取");
        }



        /**
         * @brief 获取基础类型信息
         * @return BaseType 基础类型枚举值
         * @details 如果非 FUNDAMENTAL_TYPE，返回 NOT_FUNDAMENTAL_TYPE；否则委托给 _data_ptr
         */
        [[nodiscard]] BaseType get_base_type() const SLOW_JSON_NOEXCEPT {
            if (value_type() != serializable_wrapper::FUNDAMENTAL_TYPE) {
                return NOT_FUNDAMENTAL_TYPE;
            }
            return _data_ptr->get_base_type();
        }

    protected:
        /**
         * @brief 内部构造函数，用于构造子节点
         * @param data 序列化包装器指针
         * @details 仅用于内部构造嵌套字典或值，设置类型
         */
        explicit dict(serializable_wrapper* data) :
                _key_to_index(nullptr) {
            set_value_type(data ? data->value_type() : serializable_wrapper::FUNDAMENTAL_TYPE);
            set_copied(false);
            set_heap_allocated(false);
            _data_ptr = data;
        }

        explicit dict(std::vector<serializable_wrapper>&&data):
                _key_to_index{nullptr} {
            set_value_type(serializable_wrapper::LIST_TYPE);
            set_copied(false);
            set_heap_allocated(true);
            _data_ptr=new serializable_wrapper{std::move(data)};
        }

        explicit dict(serializable_wrapper&& data) :
                _key_to_index(nullptr) {
            set_value_type(serializable_wrapper::FUNDAMENTAL_TYPE);
            set_copied(false);
            set_heap_allocated(true);
            _data_ptr = new serializable_wrapper{std::move(data)};
        }
        /**
         * @brief 设置基础类型信息
         * @param base_type 基础类型枚举值
         * @details 仅当类型为 FUNDAMENTAL_TYPE 时设置 _data_ptr 的 BaseType，否则忽略
         */
        void set_base_type(BaseType base_type) SLOW_JSON_NOEXCEPT {
            if (value_type() == serializable_wrapper::FUNDAMENTAL_TYPE) {
                _data_ptr->set_base_type(base_type);
            }
        }
        /**
         * @brief 初始化键到索引映射
         * @details 延迟初始化 _key_to_index 以提高性能
         */
        void initialize_key_to_index() const SLOW_JSON_NOEXCEPT {
            if (!get_key_to_index() && value_type() == serializable_wrapper::ROOT_DICT_TYPE) {
                set_key_to_index(new key_to_index(_data));
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

        /**
         * @brief 获取存储的值类型信息
         * @return serializable_wrapper::ValueType 值类型枚举值
         * @details 从 _key_to_index 指针的低 4 位中提取类型信息（第 2-3 位）
         */
        [[nodiscard]] serializable_wrapper::ValueType value_type() const SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_key_to_index);
            return static_cast<serializable_wrapper::ValueType>((ptr & VALUE_TYPE_MASK) >> 1);
        }

        /**
         * @brief 设置值类型信息
         * @param type 值类型枚举值
         * @details 将值类型信息（2 位）嵌入到 _key_to_index 指针的低 4 位（第 2-3 位）
         */
        void set_value_type(serializable_wrapper::ValueType type) SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_key_to_index);
            ptr &= ~VALUE_TYPE_MASK; // 清除原有值类型信息
            ptr |= (static_cast<uintptr_t>(type) << 1); // 设置新值类型信息
            _key_to_index = reinterpret_cast<key_to_index*>(ptr);
        }

        /**
         * @brief 获取复制标志
         * @return bool 是否已复制键
         * @details 从 _key_to_index 指针的低 4 位中提取复制标志（最低位）
         */
        [[nodiscard]] bool get_copied() const SLOW_JSON_NOEXCEPT {
            return reinterpret_cast<uintptr_t>(_key_to_index) & COPIED_MASK;
        }

        /**
         * @brief 设置复制标志
         * @param value 是否已复制键
         * @details 将复制标志（1 位）嵌入到 _key_to_index 指针的低 4 位（最低位）
         */
        void set_copied(bool value) SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_key_to_index);
            ptr = value ? (ptr | COPIED_MASK) : (ptr & ~COPIED_MASK);
            _key_to_index = reinterpret_cast<key_to_index*>(ptr);
        }

        /**
         * @brief 获取堆分配标志
         * @details operator=中可能会存在堆分配_data_ptr的情况
         * @note 和serializable_wrapper::is_heap_allocated语义是不同的
         * @return bool 是否为堆分配
         * @details 从 _key_to_index 指针的低 4 位中提取堆分配标志（第 4 位）
         */
        [[nodiscard]] bool is_heap_allocated() const SLOW_JSON_NOEXCEPT {
            return reinterpret_cast<uintptr_t>(_key_to_index) & HEAP_ALLOCATED_MASK;
        }

        /**
         * @brief 设置堆分配标志
         * @details operator=中可能会存在堆分配_data_ptr的情况
         * @note 和serializable_wrapper::is_heap_allocated语义是不同的
         * @param value 是否为堆分配
         * @details 将堆分配标志（1 位）嵌入到 _key_to_index 指针的低 4 位（第 4 位）
         */
        void set_heap_allocated(bool value) SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_key_to_index);
            ptr = value ? (ptr | HEAP_ALLOCATED_MASK) : (ptr & ~HEAP_ALLOCATED_MASK);
            _key_to_index = reinterpret_cast<key_to_index*>(ptr);
        }

        /**
         * @brief 获取键到索引映射指针
         * @return key_to_index* 指向键到索引映射的指针
         * @details 清除 _key_to_index 指针低 4 位的元数据，返回实际指针
         */
        [[nodiscard]] key_to_index* get_key_to_index() const SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_key_to_index);
            ptr &= CLEAR_MASK; // 清除低 4 位
            return reinterpret_cast<key_to_index*>(ptr);
        }

        /**
         * @brief 设置键到索引映射指针
         * @param ptr 键到索引映射指针
         * @details 保留低 4 位的元数据，将新指针嵌入
         */
        void set_key_to_index(key_to_index* ptr) const SLOW_JSON_NOEXCEPT {
            assert_with_message((uintptr_t)ptr % 16 == 0 || ptr == nullptr, "新指针地址非 16 对齐");
            auto old_metadata = reinterpret_cast<uintptr_t>(_key_to_index) & ~CLEAR_MASK;
            auto new_ptr = reinterpret_cast<uintptr_t>(ptr);
            _key_to_index = reinterpret_cast<key_to_index*>(new_ptr | old_metadata);
        }

        mutable key_to_index* _key_to_index; ///< 键到索引映射，延迟初始化，低 4 位存储值类型、复制标志和堆分配标志
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

        pair(const pair&p):_key{p._key},_value{p._value} {
            p._key = nullptr;
        }
        pair(pair&&p)noexcept:_key{p._key},_value{std::move(p._value)} {
            p._key = nullptr;
        }

    private:
        mutable const char *_key; ///< 键，字符串形式
        mutable serializable_wrapper _value; ///< 值，序列化包装器形式
    };
}

namespace slow_json {
    using details::pair;
    using details::dict;
    using list = std::vector<details::serializable_wrapper>;
}

#endif //SLOWJSON_DICT_HPP