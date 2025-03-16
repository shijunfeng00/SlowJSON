//
// Created by hyzh on 2025/3/14.
//
#include "slowjson.hpp"
using namespace slow_json::static_string_literals;
void test(){
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::dict d1{
            std::pair{"aa",3},
            std::pair{"bb",4},
    };
    slow_json::dict d2{
            std::pair{"x"_ss,3},
            std::pair{"y"_ss,4},
    };

    slow_json::dict d3{
            {"x"_ss,3},
            {"y"_ss,4},
    };

    slow_json::Buffer buffer;
    slow_json::dumps(buffer,d1);
    assert_with_message(buffer==R"({"bb":4,"aa":3})","slow_json::dumps序列化结果不正确");
    buffer.clear();

    slow_json::dumps(buffer,d2);
    assert_with_message(buffer==R"({"y":4,"x":3})","slow_json::dumps序列化结果不正确");
    buffer.clear();

    slow_json::dumps(buffer,d3);
    assert_with_message(buffer==R"({"y":4,"x":3})","slow_json::dumps序列化结果不正确");
    buffer.clear();

}