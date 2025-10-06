/**
 * @author shijunfeng
 * @date 2024/7/13
 * @details 实现一个用于生成JSON的字符串缓冲，避免产生大量的碎片小字符串，而是使用连续的内存去顺序分配，并可以动态扩容（模仿std::vector）
 */

#ifndef SLOWJSON_BUFFER_HPP
#define SLOWJSON_BUFFER_HPP

#include <utility>
#include <cstring>
#include <ostream>
#include <bit>
#include <vector>
#include "assert_with_message.hpp"

namespace slow_json {

    /**
     * @brief 一个用于生成JSON的字符串缓冲区，支持类似std::vector的动态大小变化
     * @details 该类的实现接口模仿std::string，基本可以平替，但是存在一些细微的差异，例如在resize和reserve的时候不会尝试去清0多余的元素
     */
    struct Buffer final {
    public:
        /**
         * 默认构造函数
         * @param capacity 初始缓冲区最大内存大小，单位为字节
         */
        explicit Buffer(std::size_t capacity = 32) :
                _capacity(capacity), _size(0), _offset(0),
                _buffer(capacity ? new char[_capacity + 1] : nullptr) {
            assert_with_message(capacity == 0 || _buffer != nullptr, "分配缓冲区失败，容量=%zu", capacity);
#ifdef debug_slow_json_buffer_print
            printf("<buffer.hpp>创建缓存:%p\n", this->_buffer);
#endif
        }

        /**
         * 析构函数
         */
        ~Buffer() {
#ifdef debug_slow_json_buffer_print
            printf("<buffer.hpp>销毁缓存:%p\n", this->_buffer);
#endif
            delete[] (this->_buffer - this->_offset);
        }

        friend std::ostream &operator<<(std::ostream &os, Buffer &buffer) {
            assert_with_message(buffer._buffer != nullptr || buffer._size == 0, "缓冲区指针为空且大小非零");
            os << buffer.c_str();
            return os;
        }
        /**
         * 根据下标访问元素
         * @param index 数组下标
         * @return
         */
        char &operator[](std::size_t index) SLOW_JSON_NOEXCEPT {
            assert_with_message(_buffer != nullptr, "缓冲区指针为空");
            assert_with_message(index < _size, "数组下标越界，index=%zu，_size=%zu", index, _size);
            return _buffer[index];
        }

        /**
         * 根据下标访问元素
         * @param index 数组下标
         * @return
         */
        const char &operator[](std::size_t index) const SLOW_JSON_NOEXCEPT {
            assert_with_message(_buffer != nullptr, "缓冲区指针为空");
            assert_with_message(index < _size, "数组下标越界，index=%zu，_size=%zu", index, _size);
            return _buffer[index];
        }

        /**
         * 在末尾插入一个字符
         * @param ch 被插入的字符
         */
        void push_back(char ch) SLOW_JSON_NOEXCEPT {
            assert_with_message(_size <= _capacity, "当前大小超出容量，_size=%zu，_capacity=%zu", _size, _capacity);
            if (_size >= _capacity) {
                // 如果插入后的大小超过最大容量，则进行扩容，容量变为当前的2倍
#ifdef debug_slow_json_buffer_print
                printf("<buffer.hpp>push_back前原本的容量:%zu\n", _capacity);
#endif
                this->reserve(std::max(1ul, _capacity << 1));
#ifdef debug_slow_json_buffer_print
                printf("<buffer.hpp>push_back后的容量:%zu\n", _capacity);
#endif
            }
            assert_with_message(_buffer != nullptr, "扩容后缓冲区指针为空");
            _buffer[_size] = ch;
            _size++;
        }

        /**
         * 删除末尾的最后一个元素
         */
        void pop_back() SLOW_JSON_NOEXCEPT {
            assert_with_message(_size > 0, "数组为空，无法删除元素");
            assert_with_message(_buffer != nullptr, "缓冲区指针为空");
            _size--;
        }

        /**
         * 获得缓冲区最后一个字符的引用
         * @return
         */
        char &back() SLOW_JSON_NOEXCEPT {
            assert_with_message(_size != 0, "数组为空，无法获取最后一个元素");
            assert_with_message(_buffer != nullptr, "缓冲区指针为空");
            return this->_buffer[_size - 1];
        }

        /**
         * 获得缓冲区最后一个字符的引用
         * @return
         */
        [[nodiscard]] const char &back() const SLOW_JSON_NOEXCEPT {
            assert_with_message(_size != 0, "数组为空，无法获取最后一个元素");
            assert_with_message(_buffer != nullptr, "缓冲区指针为空");
            return this->_buffer[_size - 1];
        }

        /**
         * 在末尾插入一个字符串
         * @param dst 被插入的字符串
         * @param length 字符串的长度
         */
        void append(const char *const dst, std::size_t length) SLOW_JSON_NOEXCEPT {
            assert_with_message(dst != nullptr, "输入字符串指针为空");
            if (_capacity == 0) {
                std::size_t new_capacity = std::max(1ul, length);
#if defined(__cpp_lib_int_pow2) && __cpp_lib_int_pow2 >= 201806L
                new_capacity = std::__ceil2(new_capacity);
#else
                if (new_capacity > 1) {
                --new_capacity;
                if (new_capacity > 0xFFFFFFFF) {
                    new_capacity |= new_capacity >> 32;
                }
                if (new_capacity > 0xFFFF) {
                    new_capacity |= new_capacity >> 16;
                }
                if (new_capacity > 0xFF) {
                    new_capacity |= new_capacity >> 8;
                }
                if (new_capacity > 0xF) {
                    new_capacity |= new_capacity >> 4;
                }
                if (new_capacity > 0x3) {
                    new_capacity |= new_capacity >> 2;
                }
                if (new_capacity > 0x1) {
                    new_capacity |= new_capacity >> 1;
                }
                new_capacity += 1;
            }
#endif
                reserve(new_capacity);
            }
            if (_size + length > _capacity) {
                std::size_t new_capacity = std::max(1ul, _capacity * 2); // 2 倍扩容
                if (new_capacity < _size + length) {
                    new_capacity = std::__ceil2(_size + length);
                }
                reserve(new_capacity);
            }
            assert_with_message(_buffer != nullptr, "扩容后缓冲区指针为空");
            memcpy(_buffer + _size, dst, length);
            _size += length;
        }

        /**
         * 在末尾插入一个字符串
         * @param dst 被插入的字符串
         */
        void append(const std::string &dst) SLOW_JSON_NOEXCEPT {
            assert_with_message(!dst.empty() || dst.c_str() != nullptr, "输入std::string无效");
            this->append(dst.c_str(), dst.size());
        }

        /**
         * 在末尾插入一个字符串
         * @param dst 被插入的字符串
         */
        void append(const std::string_view &dst) SLOW_JSON_NOEXCEPT {
            assert_with_message(!dst.empty() || dst.data() != nullptr, "输入std::string_view无效");
            this->append(dst.data(), dst.size());
        }

        /**
         * 在末尾插入一个字符串
         * @note 被插入的字符串必须包含结束符'\0'，否则将到导致不可预料的结果
         * @param dst 被插入的字符串
         */
        void append(const char *const dst) SLOW_JSON_NOEXCEPT {
            assert_with_message(dst != nullptr, "输入C字符串指针为空");
            this->append(dst, strlen(dst));
        }

        /**
         * 在末尾插入一个字符串
         * @param dst 被插入的字符串
         */
        void operator+=(const std::string &dst) SLOW_JSON_NOEXCEPT{
            this->append(dst);
        }

        /**
         * 在末尾插入一个字符串
         * @param dst 被插入的字符串
         */
        void operator+=(const std::string_view &dst) SLOW_JSON_NOEXCEPT{
            this->append(dst);
        }

        /**
         * 在末尾插入一个字符串
         * @note 被插入的字符串必须包含结束符'\0'，否则将到导致不可预料的结果
         * @param dst 被插入的字符串
         */
        void operator+=(const char *const dst) SLOW_JSON_NOEXCEPT{
            this->append(dst);
        }

        /**
         * 在末尾插入一个字符
         * @param ch 被插入的字符
         */
        void operator+=(char ch) SLOW_JSON_NOEXCEPT{
            this->append(&ch, 1);
        }

        /**
         * 获得当前缓冲区中有效字符的数量
         * @return
         */
        [[nodiscard]] std::size_t size() const SLOW_JSON_NOEXCEPT {
            return this->_size;
        }

        /**
         * 获得当前缓冲区的最大容量
         * @return
         */
        [[nodiscard]] std::size_t capacity() const SLOW_JSON_NOEXCEPT {
            return this->_capacity;
        }

        /**
         * 获得缓冲区首地址
         * @return
         */
        [[nodiscard]] char *data() SLOW_JSON_NOEXCEPT {
            return _buffer;
        }

        /**
         * @brief 改变缓冲区中有效字符数量
         * @note 注意size不能大于最大容量，当size小于当前size的时候，多余的字符不会被清空，当size大于当前size，多余的字符也不会有默认值
         *       除了size数值发生了改变，实际不会对缓冲区的数据做任何处理
         * @note 该接口不会触发数组动态扩容
         * @param size 修改之后的缓冲区有效数字长度
         */
        void resize(std::size_t size) SLOW_JSON_NOEXCEPT {
            assert_with_message(size <= _capacity, "调整大小超出容量，size=%zu，_capacity=%zu", size, _capacity);
            assert_with_message(_buffer != nullptr || size == 0, "缓冲区指针为空且大小非零");
            this->_size = size;
        }

        /**
         * @brief 尝试改变缓冲区的最大容量
         * @details 如果修改之后的缓冲区最大容量小于实际当前的最大容量，则不会起效果
         *          也就是缓冲区最大容量只能往大了改，不能往小了改，并且只会2倍扩容
         *          这样做是为了减少reserve调用次数，毕竟每次都要copy数据
         * @param target_capacity 修改之后的缓冲区最大大小
         */
        void try_reserve(std::size_t target_capacity) SLOW_JSON_NOEXCEPT {
            assert_with_message(target_capacity < (1ULL << 48), "目标容量过大，target_capacity=%zu", target_capacity);
            if (target_capacity > _capacity) {
                std::size_t new_capacity = std::max(1ul, _capacity);
                if (target_capacity > new_capacity) {
                    new_capacity = std::__ceil2(target_capacity);
                }
                reserve(new_capacity);
            }
        }

        /**
         * @brief 清空缓冲区
         * @details 其实只是把size变为0了，不会对缓冲区数据有任何修改
         * 反正都清空了，正好可以把offset也归零，容量重新上升，减少reserve调用次数
         * @see Buffer::resize
         */
        void clear() SLOW_JSON_NOEXCEPT {
            assert_with_message(_buffer != nullptr || _size == 0, "缓冲区指针为空且大小非零");
            this->resize(0);
            this->_buffer -= this->_offset;
            this->_capacity += this->_offset;
            this->_offset = 0;
        }

        /**
         * 获得缓冲区首地址
         * @return
         */
        [[nodiscard]] char *begin() SLOW_JSON_NOEXCEPT {
            return _buffer;
        }

        /**
         * 获得缓冲区尾地址（最后一个有效元素地址+1）
         * @return
         */
        [[nodiscard]] char *end() SLOW_JSON_NOEXCEPT {
            return _buffer + _size;
        }

        /**
         * 获得缓冲区首地址
         * @return
         */
        [[nodiscard]] const char *begin() const SLOW_JSON_NOEXCEPT {
            return _buffer;
        }

        /**
         * 获得缓冲区尾地址（最后一个有效元素地址+1）
         * @return
         */
        [[nodiscard]] const char *end() const SLOW_JSON_NOEXCEPT {
            return _buffer + _size;
        }

        /**
         * @brief 获得C风格的字符串
         * @details 该接口获得的字符串末尾包含一个'\0'结束符号
         * @note 该接口会修改缓冲区的数据，具体而言是将end()设置为'\0',Buffer::reserve分配内存时会保证多出一位存放'\0'
         * @note 该接口不会有数据拷贝的开销，直接返回缓冲区中数据的地址，因此注意对象声明周期问题
         * @return C风格的字符串的首地址
         */
        const char *c_str() SLOW_JSON_NOEXCEPT {
            assert_with_message(_buffer != nullptr || _size == 0, "缓冲区指针为空且大小非零");
            if (_size == 0) {
                return "\0";
            }
            _buffer[_size] = '\0';
            return _buffer;
        }

        /**
         * @brief 获得C++风格的std::string类型的字符串
         * @details 该接口会发生数据拷贝，得到的字符串是复制后的结果
         * @return std::string格式的字符串
         */
        std::string string() SLOW_JSON_NOEXCEPT {
            assert_with_message(_buffer != nullptr || _size == 0, "缓冲区指针为空且大小非零");
            return {_buffer, _size};
        }

        /**
         * @brief 删除开始的n_begin_pos个字符
         * @details 实际上，并不会真的删除数据，也不会导致数据拷贝
         *          和std::vector不同的是，这里的删除仅仅只是缓冲区的首地址加上了一个偏移量
         *          当需要调用reserve进行内存重新分配的时候，才会真正的删除数据，并将偏移量归0
         * @param n_begin_pos 需要删除的字符的数量
         */
        void erase(std::size_t n_begin_pos) SLOW_JSON_NOEXCEPT{
            assert_with_message(n_begin_pos <= _size, "删除字符数超出缓冲区大小，n_begin_pos=%zu，_size=%zu", n_begin_pos, _size);
            assert_with_message(_buffer != nullptr || _size == 0, "缓冲区指针为空且大小非零");
            _buffer = _buffer + n_begin_pos;
            _size -= n_begin_pos;
            _capacity -= n_begin_pos;
            _offset += n_begin_pos;
#ifdef debug_slow_json_buffer_print
            printf("<buffer.hpp> erase删除前%zu个元素的数据，首地址变为%p，剩余元素数量为:%zu\n", n_begin_pos, this->_buffer, this->_size);
#endif
        }

        /**
         * @brief 从缓冲区末尾分配一块未初始化的内存（默认1字节对齐）
         * @param size 需要分配的字节数
         * @return 指向分配内存起始位置的指针
         */
        void* allocate(std::size_t size) SLOW_JSON_NOEXCEPT{
            return allocate(size, 1); // 默认1字节对齐
        }

        /**
         * @brief 从缓冲区末尾分配一块对齐的未初始化内存
         * @param size 需要分配的字节数
         * @param alignment 内存对齐要求（必须是2的幂）
         * @return 指向分配内存起始位置的指针
         * @note 对齐值必须是2的幂（如1,2,4,8,16,...）
         */
        void* allocate(std::size_t size, std::size_t alignment) SLOW_JSON_NOEXCEPT {
            assert_with_message((alignment & (alignment - 1)) == 0, "对齐值必须是2的幂, alignment=%zu", alignment);
            char* ptr = _buffer + _size;
            uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
            uintptr_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1);
            std::size_t padding = aligned_addr - addr;
            std::size_t total_needed = padding + size;
            if (_size + total_needed > _capacity) {
                std::size_t new_capacity = std::max(1ul, _capacity * 2); // 2倍扩容
                if (new_capacity < _size + total_needed) {
                    new_capacity = std::__ceil2(_size + total_needed);
                }
                reserve(new_capacity);
                ptr = _buffer + _size;
                addr = reinterpret_cast<uintptr_t>(ptr);
                aligned_addr = (addr + alignment - 1) & ~(alignment - 1);
                padding = aligned_addr - addr;
                total_needed = padding + size;
            }
            _size += total_needed;
            return reinterpret_cast<void*>(aligned_addr);
        }

    private:
        /**
         * @brief 重新分配一段内存空间，并将数据拷贝过去，然后删除上次分配的内存空间，实现动态扩容
         * @details 这个函数实际上并没有考虑容量变小的情况（虽然应该也是正常的）
         * @param capacity 修改之后的缓冲区最大容量
         */
        void reserve(std::size_t capacity) SLOW_JSON_NOEXCEPT{
            assert_with_message(_capacity < capacity, "数组容量变小，_capacity=%zu，capacity=%zu", _capacity, capacity);
            assert_with_message(capacity < (1ULL << 48), "新容量过大，capacity=%zu", capacity);
            auto new_buffer = new char[capacity + 1]; // 多一位出来是用来放结束符号'\0'的
            assert_with_message(new_buffer != nullptr, "分配新缓冲区失败，容量=%zu", capacity);
            assert_with_message(_buffer,"_buffer为空");
            memcpy(new_buffer, _buffer, _size);
#ifdef debug_slow_json_buffer_print
            printf("<buffer.hpp>数组扩容,%zu->%zu 删除原来的数据:%p 创建新数据:%p\n", _capacity, capacity, _buffer, new_buffer);
#endif
            delete[] (this->_buffer - _offset);
            this->_buffer = new_buffer;
            this->_capacity = capacity;
            this->_offset = 0;
        }

        std::size_t _capacity; ///< 缓冲区当前最大容量
        std::size_t _size;     ///< 缓冲区当前使用量
        std::size_t _offset;   ///< 偏移量，用来处理erase
        char *_buffer;         ///< 缓冲区数据首地址
    };

    /**
    * @brief 用于反序列化或者slow_json::dict::copy_key中的一个缓存
    * @details目的是为字典的多个key分配连续的内存，避免频繁的new/delete，进而提升性能
    */
    template<std::size_t init_size = 64>
    class KeyBuffer final {
        static_assert(init_size > 0, "KeyBuffer<init_size>: init_size 必须大于 0");

    public:
        /**
         * @brief 构造函数
         * @details
         * - 初始化首页（位于栈内存），容量为 @tparam init_size 字节；
         * - 为堆页列表预留少量空间（避免早期扩容的 vector 搬家开销）。
         */
        KeyBuffer() noexcept
            : first_base_(first_storage_)
            , first_cap_(init_size)
            , first_used_(0) {
            heap_.reserve(10);
        }

        /**
         * @brief 析构函数
         * @details 释放所有堆分配的 chunk；首页在栈上，无需释放。
         */
        ~KeyBuffer() {
            for (auto* c : heap_) {
                destroy_chunk_(c);
            }
        }

        /**
         * @brief 从缓冲区末尾分配一段对齐的未初始化内存
         * @param size 需要分配的字节数（必须 > 0）
         * @param alignment 内存对齐要求（必须是 2 的幂，默认 1）
         * @return 指向分配内存起始位置的指针
         *
         * @details
         * - 若当前页（首页或最后一页）剩余空间不足（含对齐填充），将分配一个新的堆页；
         * - 新页容量为 @c max(当前页容量×2, std::__ceil2(size + alignment - 1))；
         * - 分配成功后仅推进“已用”指针，不会移动任何旧数据。
         *
         * @note 该接口不会抛出内存回收操作，内存由 @ref ~KeyBuffer 统一释放。
         */
        void* allocate(std::size_t size, std::size_t alignment = 1) {
            assert_with_message(size > 0, "KeyBuffer::allocate: size 必须 > 0");
            assert_with_message((alignment & (alignment - 1)) == 0, "KeyBuffer::allocate: alignment 必须为 2 的幂");

            while(true) {
                char*       base;
                std::size_t cap;
                std::size_t used;

                if (heap_.empty()) {
                    base = first_base_;
                    cap  = first_cap_;
                    used = first_used_;
                } else {
                    base = heap_.back()->data;
                    cap  = heap_.back()->cap;
                    used = heap_.back()->used;
                }

                const std::uintptr_t addr    = reinterpret_cast<std::uintptr_t>(base + used);
                const std::size_t    mask    = alignment - 1;
                const std::size_t    padding = (alignment == 1) ? 0u : ((alignment - (addr & mask)) & mask);

                if (used + padding + size <= cap) {
                    // 当前页足够：就地分配
                    if (heap_.empty()) {
                        first_used_ = used + padding + size;
                        return base + used + padding;
                    } else {
                        heap_.back()->used = used + padding + size;
                        return base + used + padding;
                    }
                }

                // 当前页不足：创建新页，容量按几何扩容或向上取 2 的幂（覆盖单次大请求）
                const std::size_t last_cap = cap; // 对首页即为 first_cap_
                const std::size_t worst    = size + (alignment > 1 ? (alignment - 1) : 0);
                std::size_t new_cap        = std::max(last_cap * 2, std::__ceil2(std::max<std::size_t>(1, worst)));
                heap_.push_back(create_chunk_(new_cap));
                // 进入下一轮循环，必可分配
            }
        }

    private:
        /**
         * @brief 内部页结构：页头 + 紧随其后的数据区
         * @details
         * 物理布局： | cap | used | data[cap] |
         */
        struct Chunk {
            std::size_t cap;     ///< 数据区总容量（字节）
            std::size_t used;    ///< 已用（字节）
            char*       data;    ///< 数据区首地址（= this + sizeof(Chunk)）
        };

        /**
         * @brief 分配一个堆页（包含页头与数据区）
         * @param cap 数据区容量（字节）
         * @return 新页指针
         */
        static Chunk* create_chunk_(std::size_t cap) {
            const std::size_t total = sizeof(Chunk) + cap;
            void* mem = ::operator new(total);
            auto* c   = new (mem) Chunk{cap, 0, nullptr};
            c->data   = reinterpret_cast<char*>(c) + sizeof(Chunk);
            return c;
        }

        /**
         * @brief 释放一个堆页
         * @param c 待释放页指针
         */ 
        static void destroy_chunk_(Chunk* c) noexcept {
            ::operator delete(c);
        }

        // —— 首页（栈）描述 —— //
        alignas(16) char first_storage_[init_size]; ///< 首页物理存储 （栈上）
        char*            first_base_;               ///< = first_storage_
        std::size_t      first_cap_;                ///< = init_size
        std::size_t      first_used_;               ///< 首页已用字节

        // —— 后续页（堆） —— //
        std::vector<Chunk*> heap_;                  ///< 所有堆页（扩容新增，不搬家）

        // 禁止拷贝/赋值/移动（避免首页地址变化导致外部指针失效）
        KeyBuffer(const KeyBuffer&)            = delete;
        KeyBuffer& operator=(const KeyBuffer&) = delete;
        KeyBuffer(KeyBuffer&&)                 = delete;
        KeyBuffer& operator=(KeyBuffer&&)      = delete;
    };
}

#endif // SLOWJSON_BUFFER_HPP