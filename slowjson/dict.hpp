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
            constexpr field_wrapper(Field Class::*field_ptr) // NOLINT(google-explicit-constructor)
                    : _object_ptr(nullptr),
                      _offset((std::size_t) (&((Class * )

            nullptr->*field_ptr))),
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
            const void *_object_ptr; ///< 绑定的对象指针
            std::size_t _offset; ///< 成员变量相对于对象地址的偏移量
            void (*_dump_fn)(slow_json::Buffer &, const void *, std::size_t); ///< 序列化函数指针
            void (*_load_fn)(const void *, std::size_t, const slow_json::dynamic_dict &); ///< 反序列化函数指针
        };

        /**
         * @brief 可序列化值包装器
         * @details 封装任意可序列化值，提供统一的dump接口，优化小对象存储以减少堆分配
         */
        struct serializable_wrapper {
        public:
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
            constexpr serializable_wrapper(T &&value) { // NOLINT(google-explicit-constructor)
                using U = std::decay_t<T>;
                constexpr std::size_t align_size = alignof(U);
                _type_name = slow_json::type_name_v<U>.str;
                assert_with_message(((uintptr_t) _type_name) % 4 == 0 , "指针地址非4对齐");
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
                if (is_heap_allocated() && _buffer_ptr) {
                    _deleter(_buffer_ptr, true);
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
            [[nodiscard]] void *value() {
                return is_heap_allocated() ? _buffer_ptr : (void *) &_buffer;
            }

            /**
             * @brief 获取值的类型名称
             * @return std::string_view 类型名称
             */
            [[nodiscard]] std::string_view type_name() const noexcept {
                uintptr_t ptr = reinterpret_cast<uintptr_t>(_type_name);
                ptr &= ~static_cast<uintptr_t>(1);
                assert_with_message(ptr % 4 == 0, "指针地址非4对齐");
                return {reinterpret_cast<const char *>(ptr)};
            }

        private:
            static constexpr std::size_t buffer_size = 24; ///< 小对象缓冲区大小
            union {
                mutable void *_buffer_ptr; ///< 大对象指针，指向堆内存
                alignas(8) char _buffer[buffer_size]; ///< 小对象缓冲区
            };
            mutable const char *_type_name; ///< 类型名称，带堆分配标志
            void (*_dump_fn)(slow_json::Buffer &, void *); ///< 序列化函数指针
            void (*_move_fn)(void *, void *); ///< 移动/复制函数指针
            void (*_deleter)(void *, bool); ///< 析构函数指针

            /**
             * @brief 初始化序列化、移动和析构函数
             * @tparam U 值类型
             */
            template<typename U>
            void initialize_functions() {
                _dump_fn = [](slow_json::Buffer &buffer, void *object) {
                    DumpToString<U>::dump(buffer, *static_cast<U *>(object));
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
         * @brief JSON键值对结构
         * @details 支持多种值类型：基本类型、类成员指针、列表、嵌套字典
         */
        struct pair {
        public:
            using value_t = std::variant<
                    helper::serializable_wrapper,
                    helper::field_wrapper,
                    std::vector<helper::serializable_wrapper>,
                    std::initializer_list<helper::serializable_wrapper>,
                    std::vector<pair>,
                    std::initializer_list<pair>>;

            /**
             * @brief 构造基本类型键值对
             * @param key JSON键
             * @param value 基本类型值
             */
            constexpr pair(const char *key, const helper::serializable_wrapper &value)
                    : _key(key), _value(value) {}

            /**
             * @brief 构造基本类型键值对（静态字符串键）
             * @tparam Key static_string类型
             * @param key JSON键
             * @param value 基本类型值
             */
            template<concepts::static_string Key>
            constexpr pair(const Key &key, const helper::serializable_wrapper &value)
                    : _key(Key::with_end()), _value(value) {}

            /**
             * @brief 构造列表类型键值对
             * @param key JSON键
             * @param value 列表值
             */
            constexpr pair(const char *key, const std::vector<helper::serializable_wrapper> &value)
                    : _key(key), _value(value) {}

            /**
             * @brief 构造列表类型键值对（静态字符串键）
             * @tparam Key static_string类型
             * @param key JSON键
             * @param value 列表值
             */
            template<concepts::static_string Key>
            constexpr pair(const Key &key, const std::vector<helper::serializable_wrapper> &value)
                    : _key(Key::with_end()), _value(value) {}

            /**
             * @brief 构造嵌套字典键值对
             * @param key JSON键
             * @param value 嵌套字典值
             */
            constexpr pair(const char *key, const std::vector<pair> &value)
                    : _key(key), _value(value) {}

            /**
             * @brief 构造嵌套字典键值对（静态字符串键）
             * @tparam Key static_string类型
             * @param key JSON键
             * @param value 嵌套字典值
             */
            template<concepts::static_string Key>
            constexpr pair(const Key &key, const std::vector<pair> &value)
                    : _key(Key::with_end()), _value(value) {}

            /**
             * @brief 构造类成员指针键值对
             * @param key JSON键
             * @param value 类成员指针
             */
            constexpr pair(const char *key, const helper::field_wrapper &value)
                    : _key(key), _value(value) {}

            /**
             * @brief 构造类成员指针键值对（静态字符串键）
             * @tparam Key static_string类型
             * @param key JSON键
             * @param value 类成员指针
             */
            template<concepts::static_string Key>
            constexpr pair(const Key &key, const helper::field_wrapper &value)
                    : _key(Key::with_end()), _value(value) {}

            /**
             * @brief 获取键值
             * @return const char* 键字符串
             */
            [[nodiscard]] const char *key() const noexcept { return _key; }

            /**
             * @brief 获取值引用（常量）
             * @return const value_t& 值variant
             */
            [[nodiscard]] const value_t &value() const noexcept { return _value; }

            /**
             * @brief 获取值引用（可变）
             * @return value_t& 值variant
             */
            [[nodiscard]] value_t &value() noexcept { return _value; }

            /**
             * @brief 类型安全的取值方法（常量）
             * @tparam T 期望类型
             * @return const void* 类型指针或nullptr
             */
            template<typename T>
            [[nodiscard]] const void *get_if() const noexcept {
                return std::get_if<T>(&_value);
            }

            /**
             * @brief 类型安全的取值方法（可变）
             * @tparam T 期望类型
             * @return void* 类型指针或nullptr
             */
            template<typename T>
            [[nodiscard]] void *get_if() noexcept {
                return std::get_if<T>(&_value);
            }

        private:
            const char *_key; ///< JSON键字符串
            value_t _value; ///< 值variant
        };

    }


    struct list:public std::vector<helper::serializable_wrapper>{
      using super_t=std::vector<helper::serializable_wrapper>;
      list(std::initializer_list<helper::serializable_wrapper>&&object):super_t(std::move(object)){}
      list(const list&)=delete;
      list(list&&others)=default;
    };

/**
 * @brief 动态字典类
 * @details 支持嵌套字典、列表、混合类型值存储和类成员指针绑定
 * @note 性能略低于static_dict，适合动态构建场景
 */
    struct dict {
        // Friend declarations
        friend struct LoadFromDict<dict>;
        friend struct DumpToString<dict>;

    public:
        enum class value_type {
            UNKNOWN,    ///< 未知类型
            ELEMENT,    ///< 基本类型
            LIST,       ///< 列表
            DICT,       ///< 嵌套字典
            ROOT_DICT   ///< 根字典
        };

        /**
         * @brief 默认构造函数，创建空的根字典
         */
        dict() : _type(value_type::ROOT_DICT) {
            new(&_buffer) map_type{};
            reinterpret_cast<map_type *>(&_buffer)->reserve(16);
        }

        /**
         * @brief 从键值对构造
         * @tparam K 键类型
         * @tparam V 值类型
         * @param args 键值对
         */
        template<typename... K, typename... V> requires((concepts::string<K> && !concepts::static_string<K>) && ...)

        dict(const std::pair<K, V> &... args)
                : dict(std::vector<helper::pair>{helper::pair{args.first, args.second}...}) {}

        /**
         * @brief 从静态字符串键值对构造
         * @tparam K 键类型
         * @tparam V 值类型
         * @param args 键值对
         */
        template<typename... K, typename... V> requires(concepts::static_string<K> && ...)

        dict(const std::pair<K, V> &... args)
                : dict(std::vector<helper::pair>{helper::pair{K::with_end(), args.second}...}) {}

        /**
         * @brief 从pair vector构造
         * @param pairs 键值对vector
         */
        dict(const std::vector<helper::pair> &pairs)
                : dict(pairs.data(), pairs.data() + pairs.size()) {}

        /**
         * @brief 从初始化列表构造
         * @param pairs 键值对初始化列表
         */
        dict(const std::initializer_list<helper::pair> &pairs)
                : dict(pairs.begin(), pairs.end()) {}

        dict(const dict &) = delete;

        dict(dict&& other) noexcept : _type(other._type) {
            if (_type == value_type::ROOT_DICT) {
                // 移动 map_type 到 _buffer
                new(&_buffer) map_type(std::move(*reinterpret_cast<map_type*>(&other._buffer)));
                // 销毁 other 的 map_type，避免 double free
                reinterpret_cast<map_type*>(&other._buffer)->~map_type();
            } else {
                // 移动指针类型，转移所有权
                _object_ptr = other._object_ptr;
                other._object_ptr = nullptr;
            }
            // 将源对象的 _type 设置为 UNKNOWN，表示已移动
            other._type = value_type::UNKNOWN;
        }

        /**
         * @brief 析构函数
         * @details 仅当_type为ROOT_DICT时释放资源
         */
        ~dict() {
            if (_type == value_type::ROOT_DICT) {
                reinterpret_cast<map_type *>(_buffer)->~map_type();
            }
        }

        /**
         * @brief 获取对象类型
         * @return value_type 当前类型
         */
        [[nodiscard]] value_type type() const noexcept { return _type; }

        /**
         * @brief 检查是否为数组
         * @return bool 是否为LIST
         */
        [[nodiscard]] bool is_array() const noexcept { return _type == value_type::LIST; }

        /**
         * @brief 检查是否为字典
         * @return bool 是否为DICT或ROOT_DICT
         */
        [[nodiscard]] bool is_dict() const noexcept {
            return _type == value_type::DICT || _type == value_type::ROOT_DICT;
        }

        /**
         * @brief 检查是否为基本类型
         * @return bool 是否为ELEMENT
         */
        [[nodiscard]] bool is_element() const noexcept { return _type == value_type::ELEMENT; }

        std::string_view type_name()const noexcept{
            assert_with_message(this->is_element(),"非基础元素对象");
            return static_cast<helper::serializable_wrapper*>(this->_object_ptr)->type_name();
        }

        /**
         * @brief 检查是否包含键
         * @param key 键
         * @return bool 是否包含
         */
        [[nodiscard]] bool contains(const char *key) const {
            assert_with_message(is_dict(), "非字典类型无法按键访问");
            if (_type == value_type::ROOT_DICT) {
                return reinterpret_cast<const map_type *>(&_buffer)->contains(key);
            }
            return static_cast<dict *>(_object_ptr)->contains(key);
        }

        /**
         * @brief 获取数组长度
         * @return std::size_t 长度
         */
        [[nodiscard]] std::size_t size() const {
            assert_with_message(is_array(), "非数组类型无法获取长度");
            return static_cast<std::vector<helper::serializable_wrapper> *>(_object_ptr)->size();
        }

        /**
         * @brief 获取字典所有键
         * @return std::vector<const char*> 键列表
         */
        [[nodiscard]] std::vector<const char *> keys() const {
            assert_with_message(is_dict(), "非字典类型无法获取键");
            std::vector<const char *> result;
            if (_type == value_type::ROOT_DICT) {
                for (const auto &it: *reinterpret_cast<const map_type *>(&_buffer)) {
                    result.emplace_back(it.first);
                }
            } else {
                result = static_cast<dict *>(_object_ptr)->keys();
            }
            return result;
        }

        /**
         * @brief 访问字典元素
         * @tparam T 键类型（指针）
         * @param key 键
         * @return dict 新dict对象
         */
        template<typename T, typename = std::enable_if_t<std::is_pointer_v<T> && !std::is_integral_v<T>>>
        dict operator[](T key) {
            assert_with_message(is_dict(), "非字典类型无法按键访问");
            map_type *map = get_map();
            assert_with_message(map->contains(key), "键不存在: %s", key);
            auto &var = map->at(key);
            if (auto *val = std::get_if<helper::serializable_wrapper>(&var)) {
                return dict{val, value_type::ELEMENT};
            } else if (auto *val = std::get_if<std::vector<helper::serializable_wrapper>>(&var)) {
                return dict{val, value_type::LIST};
            } else if (auto *val = std::get_if<std::unique_ptr<dict>>(&var)) {
                return dict{val->get(), value_type::DICT};
            }
            assert_with_message(false, "未知variant类型");
            return dict{nullptr, value_type::UNKNOWN};
        }

        /**
         * @brief 访问数组元素
         * @param index 索引
         * @return dict 新dict对象
         */
        dict operator[](std::size_t index) {
            assert_with_message(is_array(), "非数组类型无法按索引访问");
            auto&vec = *static_cast<std::vector<helper::serializable_wrapper> *>(_object_ptr);
            assert_with_message(index < vec.size(), "数组越界: 下标%d, 长度%d", index, vec.size());
            auto&object=vec[index];
            // 可能存在多重嵌套
            if(object.type_name()=="slow_json::dict"){
                return dict{static_cast<dict*>(object.value())->object(),value_type::DICT};
            }
            if(object.type_name()=="slow_json::list"){
                return dict{static_cast<std::vector<helper::serializable_wrapper>*>(object.value()),value_type::LIST};
            }
            return dict{&object, value_type::ELEMENT};
        }

        /**
         * @brief 转换为指定类型
         * @tparam T 目标类型
         * @return T& 转换后的引用
         */
        template<typename T>
        T &cast() {
            assert_with_message(is_element(), "非基本类型无法转换");
            auto correct_type_name = static_cast<helper::serializable_wrapper *>(_object_ptr)->type_name();
            constexpr std::string_view type_name = slow_json::type_name_v<T>.str;
            assert_with_message(correct_type_name == type_name, "转换类型不正确，需要%s,实际为%s",
                                correct_type_name.data(), type_name.data());
            return *static_cast<T *>(static_cast<helper::serializable_wrapper *>(_object_ptr)->value());
        }

    private:
        union {
            alignas(8) char _buffer[56]; ///< 根字典缓冲区
            void *_object_ptr; ///< 非根字典指针
        };
        value_type _type; ///< 数据类型

        using map_type = std::unordered_map<const char *, std::variant<
                helper::serializable_wrapper,
                helper::field_wrapper,
                std::vector<helper::serializable_wrapper>,
                std::unique_ptr<dict>>>;

        /**
         * @brief 获取对象指针
         * @return void* 对象指针
         */
        [[nodiscard]] void *object() const noexcept {
            assert_with_message(_type!=value_type::UNKNOWN,"类型异常");
            return (_type == value_type::ROOT_DICT) ? (void *) &_buffer : _object_ptr;
        }

        /**
         * @brief 获取map指针
         * @return map_type* map指针
         */
        [[nodiscard]] map_type *get_map() {
            return (_type == value_type::ROOT_DICT)
                   ? reinterpret_cast<map_type *>(&_buffer)
                   : reinterpret_cast<map_type *>(static_cast<dict *>(_object_ptr)->object());
        }

        /**
         * @brief 范围构造函数
         * @param begin pair数组起始指针
         * @param end pair数组结束指针
         */
        dict(const helper::pair *begin, const helper::pair *end) : _type(value_type::ROOT_DICT) {
            new(&_buffer) map_type{};
            reinterpret_cast<map_type *>(&_buffer)->reserve(end - begin);
            auto *map = reinterpret_cast<map_type *>(&_buffer);
            for (; begin != end; ++begin) {
                const auto &p = *begin;
                const void *value_ptr = nullptr;
                using type1 = helper::serializable_wrapper;
                using type2 = std::vector<helper::serializable_wrapper>;
                using type3 = std::vector<helper::pair>;
                using type4 = std::initializer_list<helper::pair>;
                using type5 = helper::field_wrapper;
                if ((value_ptr = p.get_if<type1>())) {
                    map->emplace(p.key(), std::forward<type1>(*(type1 *) value_ptr));
                } else if ((value_ptr = p.get_if<type2>())) {
                    map->emplace(p.key(), std::forward<type2>(*(type2 *) value_ptr));
                } else if ((value_ptr = p.get_if<type3>())) {
                    map->emplace(p.key(), std::make_unique<dict>(*(type3 *) value_ptr));
                } else if ((value_ptr = p.get_if<type4>())) {
                    map->emplace(p.key(), std::make_unique<dict>(*(type4 *) value_ptr));
                } else if ((value_ptr = p.get_if<type5>())) {
                    map->emplace(p.key(), *(type5 *) value_ptr);
                } else {
                    assert_with_message(false, "序列化失败: 未知值类型");
                }
            }
        }

        /**
         * @brief 私有构造函数，用于非根dict
         * @param object 数据指针
         * @param type 数据类型
         */
        dict(void *object, value_type type) : _object_ptr(object), _type(type) {
            assert_with_message(type != value_type::ROOT_DICT, "类型不能为ROOT_DICT");
        }
    };

} // namespace slow_json

#endif // SLOWJSON_DICT_HPP