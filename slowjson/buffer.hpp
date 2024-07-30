//
// Created by hy-20 on 2024/7/13.
//

#ifndef SLOWJSON_BUFFER_HPP
#define SLOWJSON_BUFFER_HPP

#include<utility>
#include <cstring>
#include <ostream>
#include"assert_with_message.hpp"

namespace slow_json {


    struct Buffer {
    public:
        explicit Buffer(std::size_t capacity = 0) : _capacity(capacity), _size(0), _offset(0),
                                                    _buffer(capacity ? new char[_capacity] : nullptr) {
#ifdef debug_slow_json_buffer_print
            printf("创建缓存:%p\n", this->_buffer);
#endif
        }

        char &operator[](std::size_t index) noexcept {
            assert_with_message(index < _size, "发生数组越界行为,index=%zu _size=%zu", index, _size);
            return _buffer[index];
        }

        const char &operator[](std::size_t index) const noexcept {
            assert_with_message(index < _size, "发生数组越界行为,index=%zu,_size=%zu", index, _size);
            return _buffer[index];
        }

        void push_back(char ch) noexcept {
            if (_size >= _capacity) {
#ifdef debug_slow_json_buffer_print
                printf("原本的容量:%zu",_capacity);
#endif
                this->reserve(std::max(1ul, _capacity << 1));
            }
            _buffer[_size] = ch;
            _size++;
        }

        char &back() noexcept {
            assert_with_message(_size != 0, "数组为空，无法获取最后一个元素的地址");
            return this->_buffer[_size - 1];
        }

        [[nodiscard]] const char &back() const noexcept {
            assert_with_message(_size != 0, "数组为空，无法获取最后一个元素的地址");
            return this->_buffer[_size - 1];
        }

        void append(const char *const dst, std::size_t length) noexcept {
            if (_capacity == 0) {
                this->reserve(length);
            }
            if (_size + length >= _capacity) {
                std::size_t new_capacity = _capacity;
                while (new_capacity < _size + length)new_capacity <<= 1;
                if (new_capacity != _capacity) {
                    this->reserve(new_capacity);
                }
            }
            memcpy(_buffer + _size, dst, length);
            _size += length;
        }

        void append(const std::string &dst) noexcept {
            this->append(dst.c_str(), dst.size());
        }

        void append(const std::string_view &dst) noexcept {
            this->append(dst.data(), dst.size());
        }

        void append(const char *const dst) {
            this->append(dst, strlen(dst));
        }

        void operator+=(const std::string &dst) {
            this->append(dst);
        }

        void operator+=(const std::string_view &dst) {
            this->append(dst);
        }

        void operator+=(const char *const dst) {
            this->append(dst);
        }

        void operator+=(char ch) {
            this->append(&ch, 1);
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return this->_size;
        }

        [[nodiscard]] std::size_t capacity() const noexcept {
            return this->_capacity;
        }

        [[nodiscard]] char *data() noexcept {
            return _buffer;
        }

        void resize(std::size_t size) noexcept {
            assert_with_message(size <= _capacity, "数组越界,size=%zu,_capacity=%zu", size, _capacity);
            this->_size = size;
        }

        void try_reserve(std::size_t target_capacity) {
            if (target_capacity > _capacity) {
                this->reserve(target_capacity);
            }
        }

        void clear() noexcept {
            this->resize(0);
        }

        [[nodiscard]] char *begin() noexcept {
            return _buffer;
        }

        [[nodiscard]] char *end() noexcept {
            return _buffer + _size;
        }

        [[nodiscard]] const char *begin() const noexcept {
            return _buffer;
        }

        [[nodiscard]] const char *end() const noexcept {
            return _buffer + _size;
        }

        const char *c_str() noexcept {
            if (_size == 0) {
                return "\0";
            }
            _buffer[_size] = '\0';
            return _buffer;
        }

        std::string string() noexcept {
            return {_buffer, _size};
        }

        void erase(std::size_t n_begin_pos) {
            _buffer = _buffer + n_begin_pos;
            _size -= n_begin_pos;
            _capacity -= n_begin_pos;
            _offset += n_begin_pos;
        }

        ~Buffer() {
#ifdef debug_slow_json_buffer_print
            printf("销毁缓存:%p\n",this->_buffer);
#endif
            delete[] (this->_buffer - this->_offset);
        }

        friend std::ostream &operator<<(std::ostream &os, Buffer &buffer) {
            os << buffer.c_str();
            return os;
        }

    private:

        void reserve(std::size_t capacity) {
            auto new_buffer = new char[capacity + 1]; //多一位出来是用来放结束符号'\0'的
            memcpy(new_buffer, _buffer, _capacity);
#ifdef debug_slow_json_buffer_print
            printf("数组扩容,%zu->%zu 删除原来的数据:%p 创建新数据:%p\n",_capacity,capacity,_buffer,new_buffer);
#endif
            assert_with_message(_capacity < capacity, "数组容量变小:%zu->%zu", _capacity, capacity);
            delete[] (this->_buffer - _offset);
            this->_buffer = new_buffer;
            this->_capacity = capacity;
            this->_offset = 0;
        }

        std::size_t _capacity;
        std::size_t _size;
        std::size_t _offset;
        char *_buffer;

    };
}
#endif //SLOWJSON_BUFFER_HPP
