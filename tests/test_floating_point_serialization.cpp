//
// Created by hy-20 on 2024/8/12.
//
//#define debug_slow_json_buffer_print

#include "slowjson.hpp"
#include<iostream>

void test_floating_point_serialization() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::Buffer buffer{1000};
    float x = 123.45;
    slow_json::dumps(buffer, x);
    assert_with_message(buffer.string() == "123.4499", "通过slow_json::dumps序列化浮点数结果错误");
    buffer.clear();

    x = 12345;
    slow_json::dumps(buffer, x);
    assert_with_message(buffer.string() == "12345", "通过slow_json::dumps序列化浮点数结果错误");
    buffer.clear();

    x = 0.12345;
    slow_json::dumps(buffer, x);
    assert_with_message(buffer.string() == "0.12345", "通过slow_json::dumps序列化浮点数结果错误");
    buffer.clear();

    double y = 0.12345;
    slow_json::dumps(buffer, y);
    assert_with_message(buffer.string() == "0.12345", "通过slow_json::dumps序列化浮点数结果错误");
    buffer.clear();

    y = 123.45;
    slow_json::dumps(buffer, y);
    assert_with_message(buffer.string() == "123.45", "通过slow_json::dumps序列化浮点数结果错误");
    buffer.clear();

    y = 123.4567890123456789;
    slow_json::dumps(buffer, y);
    assert_with_message(buffer.string() == "123.45678901234568", "通过slow_json::dumps序列化浮点数结果错误");
    buffer.clear();

    y = 1234567890123456789.0;
    slow_json::dumps(buffer, y);
    assert_with_message(buffer.string() == "12345678901234568", "通过slow_json::dumps序列化浮点数结果错误");
    buffer.clear();

    y = 0.0;
    slow_json::dumps(buffer, y);
    slow_json::dumps(buffer, x);
    slow_json::dumps(buffer, y);
    slow_json::dumps(buffer, y);
    assert_with_message(buffer.string() == "00.1234500", "通过slow_json::dumps序列化浮点数结果错误");
    buffer.clear();

}