//
// Created by hy-20 on 2024/8/9.
//
#include "slowjson.hpp"
#include "iostream"

using namespace slow_json::static_string_literals;

void test_function() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::dict dict{
            std::pair{"function",[](){return "test";}}
    };

    slow_json::dict dict2{
            std::pair{"function",[](){return "test2";}}
    };

    slow_json::Buffer buffer;
    slow_json::dumps(buffer,dict);
    assert_with_message(buffer.string()==R"({"function":"test"})","通过slow_json::dumps序列化得到的结果不正确");
    buffer.clear();
    slow_json::dumps(buffer,dict2);
    assert_with_message(buffer.string()==R"({"function":"test2"})","通过slow_json::dumps序列化得到的结果不正确");
}