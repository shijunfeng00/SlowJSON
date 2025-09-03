//
// Created by hyzh on 2025/9/3.
//

#ifndef SLOWJSON_WRAPPER_HPP
#define SLOWJSON_WRAPPER_HPP
#include "load_from_dict_interface.hpp"
#include "key_to_index.hpp"
namespace slow_json::details {
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
                  _offset((std::size_t)(&((Class * )

        nullptr->*field_ptr))),
        _dump_fn([](
        slow_json::Buffer &buffer,
        const void *object_ptr, std::size_t
        offset) {
            const auto &object = *(Class *) object_ptr;
            Field &field = std::ref(*(Field * )((std::uintptr_t) & object + offset));
            DumpToString < Field > ::dump(buffer, field);
        }),
        _load_fn([](
        const void *object_ptr, std::size_t
        offset,
        const slow_json::dynamic_dict &dict
        ) {
            auto &object = *(Class *) object_ptr;
            Field &field = std::ref(*(Field * )((std::uintptr_t) & object + offset));
            LoadFromDict < Field > ::load(field, dict);
        }) {}

        /**
         * @brief 执行字段序列化
         * @param buffer 输出缓冲区
         * @throws 当_object_ptr为空时触发断言
         */
        void dump_fn(slow_json::Buffer &buffer) const

        SLOW_JSON_NOEXCEPT {
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
         * @brief 基础类型枚举，细化FUNDAMENTAL_TYPE，用于反序列化时记录具体类型
         */
        enum BaseType {
            NOT_FUNDAMENTAL_TYPE = 0, // 非基本类型
            NULL_TYPE = 1,
            INT64_TYPE = 2,
            UINT64_TYPE = 3,
            DOUBLE_TYPE = 4,
            BOOL_TYPE = 5,
            STRING_TYPE = 6
        };
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
            FUNDAMENTAL_TYPE = 0,
            LIST_TYPE = 1,
            DICT_TYPE = 2,
            ROOT_DICT_TYPE = 3
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
        requires(!std::is_same_v < std::decay_t < T > , serializable_wrapper >)
        constexpr serializable_wrapper(T &&value)

        SLOW_JSON_NOEXCEPT { // NOLINT
            using Raw = std::decay_t<T>;
            using U = std::conditional_t <std::is_member_pointer_v<Raw>, field_wrapper, Raw>;
            using V = std::conditional_t <std::is_member_pointer_v<Raw>, field_wrapper, T>;
            _type_name = slow_json::type_name_v<U>.str;
            assert_with_message((uintptr_t) _type_name % 64 == 0, "指针地址非 64 对齐");
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
        serializable_wrapper(const serializable_wrapper &other)

        SLOW_JSON_NOEXCEPT
                : _type_name(other._type_name),
                _dump_fn(other
        ._dump_fn),
        _move_or_delete_fn(other
        ._move_or_delete_fn) {
            set_heap_allocated(other.is_heap_allocated());
            if (is_heap_allocated()) {
                _buffer_ptr = other._buffer_ptr;
                other._buffer_ptr = nullptr;
            } else {
                other._move_or_delete_fn(&_buffer, const_cast<void *>(static_cast<const void *>(&other._buffer)), false,
                                         false);
            }
        }

        /**
         * @brief 移动赋值运算符
         * @param other 源对象
         * @return 当前对象的引用
         * @details 清理当前资源，转移源对象的资源，避免重复释放
         */
        serializable_wrapper &operator=(serializable_wrapper &&other) noexcept {
            if (this != &other) {
                // 清理当前资源（删除操作）
                if (_move_or_delete_fn) {
                    void *target = is_heap_allocated() ? _buffer_ptr : static_cast<void *>(&_buffer);
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
                void *target = is_heap_allocated() ? _buffer_ptr : static_cast<void *>(&_buffer);
                _move_or_delete_fn(nullptr, target, true, is_heap_allocated());
            }
        }

        /**
         * @brief 执行序列化
         * @param buffer 输出缓冲区
         * @details 调用存储的序列化函数，处理小对象或大对象
         */
        void dump_fn(slow_json::Buffer &buffer) const

        SLOW_JSON_NOEXCEPT {
            void *ptr = is_heap_allocated() ? _buffer_ptr : (void *) &_buffer;
            assert_with_message(ptr, "ptr地址不能为空");
            _dump_fn(buffer, ptr);
        }

        /**
         * @brief 获取值的指针
         * @return void* 指向值的指针
         */
        [[nodiscard]] const void *value() const

        SLOW_JSON_NOEXCEPT {
            return is_heap_allocated() ? _buffer_ptr : (void *) &_buffer;
        }

        /**
         * @brief 获取值的指针
         * @return void* 指向值的指针
         */
        [[nodiscard]] void *value()

        SLOW_JSON_NOEXCEPT {
            return is_heap_allocated() ? _buffer_ptr : (void *) &_buffer;
        }

        /**
         * @brief 获取被存储的原本类型名称，也即构造函数模板参数类型字符串
         * @return std::string_view 类型名称
         */
        [[nodiscard]] std::string_view type_name() const

        SLOW_JSON_NOEXCEPT {
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
        [[nodiscard]] ValueType value_type() const

        SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            return static_cast<ValueType>((ptr & VALUE_TYPE_MASK) >> 1);
        }

        /**
         * @brief 获取基础类型信息
         * @return BaseType 基础类型枚举值
         * @details 从 _type_name 指针的低6位（第4-6位）提取基础类型信息
         */
        [[nodiscard]] BaseType get_base_type() const

        SLOW_JSON_NOEXCEPT {
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
        void (*_move_or_delete_fn)(void *, void *, bool, bool) = nullptr;

        /**
         * @brief 设置值类型信息
         * @tparam U 实际存储的类型
         * @details 根据类型设置值类型标志位
         */
        template<typename U>
        void set_value_type()

        SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            // 清除原有值类型信息
            ptr &= ~VALUE_TYPE_MASK;

            // 设置新值类型信息
            if constexpr (std::is_same_v < U, std::vector < serializable_wrapper >>) {
                ptr |= (LIST_TYPE << 1);
            } else if constexpr (std::is_same_v < U, dict >) {
                ptr |= (DICT_TYPE << 1);
            } else {
                ptr |= (FUNDAMENTAL_TYPE << 1);
            }

            _type_name = reinterpret_cast<const char *>(ptr);
        }

        /**
         * @brief 设置基础类型信息
         * @param base_type 基础类型枚举值
         * @details 将基础类型信息（3位）嵌入到 _type_name 指针的低6位（第4-6位）
         */
        void set_base_type(BaseType base_type)

        SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            ptr &= ~BASE_TYPE_MASK; // 清除原有基础类型信息
            ptr |= (static_cast<uintptr_t>(base_type) << 3); // 设置新基础类型信息
            _type_name = reinterpret_cast<const char *>(ptr);
        }

        /**
         * @brief 设置堆分配标志
         * @param value 是否为堆分配
         */
        void set_heap_allocated(bool value) const

        SLOW_JSON_NOEXCEPT {
            auto ptr = reinterpret_cast<uintptr_t>(_type_name);
            ptr = value ? (ptr | HEAP_MASK) : (ptr & ~HEAP_MASK);
            _type_name = reinterpret_cast<char *>(ptr);
        }

        /**
         * @brief 获取堆分配标志
         * @return bool 是否为堆分配
         */
        [[nodiscard]] bool is_heap_allocated() const

        SLOW_JSON_NOEXCEPT {
            return reinterpret_cast<uintptr_t>(_type_name) & HEAP_MASK;
        }

        /**
         * @brief 初始化序列化、移动和析构函数
         * @tparam U 值类型
         */
        template<typename U>
        void initialize_functions()

        SLOW_JSON_NOEXCEPT {
            _dump_fn = [](slow_json::Buffer &buffer, void *object) {
                slow_json::DumpToString<U>::dump(buffer, *static_cast<U *>(object));
            };
            _move_or_delete_fn = [](void *dest, void *src, bool move_type, bool is_heap_allocated) {
                if (!move_type) {
                    // 移动操作：构造新对象到dest
                    if constexpr (std::is_trivially_copyable_v < U >) {
                        std::memcpy(dest, src, sizeof(U));
                    } else if constexpr (std::is_move_constructible_v < U >) {
                        new(dest) U(std::move(*static_cast<U *>(src)));
                    } else if constexpr (std::is_copy_constructible_v < U >) {
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
}
#endif //SLOWJSON_WRAPPER_HPP
