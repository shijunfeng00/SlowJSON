//
// Created by hy-20 on 2024/7/18.
//
// #define debug_slow_json_buffer_print

#include "buffer.hpp"
#include <iostream>
#include<sstream>
using namespace slow_json;
void test_buffer() {
    printf("run %s\n", __PRETTY_FUNCTION__);

    // 测试默认构造和空缓冲区
    {
        Buffer buffer(0);
        assert_with_message(buffer.size() == 0, "空缓冲区大小应为 0");
        assert_with_message(buffer.capacity() == 0, "空缓冲区容量应为 0");
        assert_with_message(buffer.c_str() == std::string("\0"), "空缓冲区 c_str 应返回空字符串");
        assert_with_message(buffer.string().empty(), "空缓冲区 string 应返回空 std::string");
    }

    // 测试构造和基本字符操作
    {
        Buffer buffer(32);
        assert_with_message(buffer.size() == 0, "初始缓冲区大小应为 0");
        assert_with_message(buffer.capacity() == 32, "初始缓冲区容量应为 32");

        buffer.push_back('A');
        assert_with_message(buffer.size() == 1, "push_back 后大小应为 1");
        assert_with_message(buffer[0] == 'A', "buffer[0] 应为 'A'");
        assert_with_message(buffer.back() == 'A', "back() 应返回 'A'");

        buffer.push_back('B');
        assert_with_message(buffer.size() == 2, "push_back 后大小应为 2");
        assert_with_message(buffer[1] == 'B', "buffer[1] 应为 'B'");
        assert_with_message(buffer.back() == 'B', "back() 应返回 'B'");

        buffer.pop_back();
        assert_with_message(buffer.size() == 1, "pop_back 后大小应为 1");
        assert_with_message(buffer.back() == 'A', "pop_back 后 back() 应返回 'A'");

        buffer.append("CDE", 3);
        assert_with_message(buffer.size() == 4, "append 后大小应为 4");
        assert_with_message(buffer.string() == "ACDE", "append 后内容应为 'ACDE'");
        assert_with_message(std::strcmp(buffer.c_str(), "ACDE") == 0, "c_str 应返回 'ACDE'");
    }

    // 测试动态扩容
    {
        Buffer buffer(4);
        buffer.append("12345", 5); // 触发扩容
        assert_with_message(buffer.size() == 5, "append 后大小应为 5");
        assert_with_message(buffer.capacity() >= 5, "扩容后容量应大于或等于 5");
        assert_with_message(buffer.string() == "12345", "append 后内容应为 '12345'");

        buffer.try_reserve(100);
        assert_with_message(buffer.capacity() >= 100, "try_reserve 后容量应大于或等于 100");
        assert_with_message(buffer.size() == 5, "try_reserve 不应改变大小");
        assert_with_message(buffer.string() == "12345", "try_reserve 后内容不变");
    }

    // 测试字符串追加
    {
        Buffer buffer(32);
        buffer += std::string("Hello");
        buffer += std::string_view(" ");
        buffer += "World";
        assert_with_message(buffer.size() == 11, "追加字符串后大小应为 11");
        assert_with_message(buffer.string() == "Hello World", "追加字符串后内容应为 'Hello World'");

        buffer += '!';
        assert_with_message(buffer.size() == 12, "追加字符后大小应为 12");
        assert_with_message(buffer.string() == "Hello World!", "追加字符后内容应为 'Hello World!'");
    }

    // 测试擦除
    {
        Buffer buffer(32);
        buffer.append("123456789", 9);
        buffer.erase(3);
        assert_with_message(buffer.size() == 6, "erase 3 个字符后大小应为 6");
        assert_with_message(buffer.capacity() == 29, "erase 后容量应减小 3");
        assert_with_message(buffer.string() == "456789", "erase 后内容应为 '456789'");
    }

    // 测试内存分配和对齐
    {
        Buffer buffer(32);
        void* ptr1 = buffer.allocate(8, 8); // 8 字节对齐
        assert_with_message(reinterpret_cast<uintptr_t>(ptr1) % 8 == 0, "分配的内存应为 8 字节对齐");
        assert_with_message(buffer.size() >= 8, "分配 8 字节后大小应增加");

        buffer.clear();
        assert_with_message(buffer.size() == 0, "clear 后大小应为 0");
        assert_with_message(buffer.capacity() == 32, "clear 后容量应恢复");

        void* ptr2 = buffer.allocate(10, 16); // 16 字节对齐
        assert_with_message(reinterpret_cast<uintptr_t>(ptr2) % 16 == 0, "分配的内存应为 16 字节对齐");
        assert_with_message(buffer.size() >= 10, "分配 10 字节后大小应增加");
    }

    // 测试 resize
    {
        Buffer buffer(32);
        buffer.append("12345", 5);
        buffer.resize(3);
        assert_with_message(buffer.size() == 3, "resize 后大小应为 3");
        assert_with_message(buffer.string() == "123", "resize 后内容应为 '123'");

        buffer.resize(5); // 扩展大小，但不初始化
        assert_with_message(buffer.size() == 5, "resize 后大小应为 5");
    }

    // 测试异常情况
    {
        Buffer buffer(32);
        try {
            buffer[0];
            assert_with_message(false, "访问空缓冲区应抛出 std::runtime_error");
        } catch (const std::runtime_error& e) {
            // printf("Caught expected exception for empty buffer access: %s\n", e.what());
        } catch(...){

        }

        try {
            buffer.pop_back();
            assert_with_message(false, "pop_back 空缓冲区应抛出 std::runtime_error");
        } catch (const std::runtime_error& e) {
            // printf("Caught expected exception for pop_back on empty buffer: %s\n", e.what());
        }

        try {
            buffer.back();
            assert_with_message(false, "back 空缓冲区应抛出 std::runtime_error");
        } catch (const std::runtime_error& e) {
            // printf("Caught expected exception for back on empty buffer: %s\n", e.what());
        }

        try {
            buffer.resize(33);
            assert_with_message(false, "resize 超出容量应抛出 std::runtime_error");
        } catch (const std::runtime_error& e) {
            // printf("Caught expected exception for resize beyond capacity: %s\n", e.what());
        }

        try {
            buffer.erase(1);
            assert_with_message(false, "erase 空缓冲区应抛出 std::runtime_error");
        } catch (const std::runtime_error& e) {
            // printf("Caught expected exception for erase on empty buffer: %s\n", e.what());
        }
    }

    // 测试 ostream 输出
    {
        Buffer buffer(32);
        buffer.append("Test", 4);
        std::ostringstream oss;
        oss << buffer;
        assert_with_message(oss.str() == "Test", "ostream 输出应为 'Test'");
    }
}

