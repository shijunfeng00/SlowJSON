//
// Created by hy-20 on 2024/8/9.
//
#include "slowjson.hpp"
#include "iostream"

using namespace slow_json::static_string_literals;

void test_merge_dict() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::static_dict json{
            std::pair{"a"_ss, 5}
    };
    slow_json::static_dict json2{
            std::pair{"b"_ss, 7.2}
    };
    slow_json::static_dict json3 = slow_json::merge(json, json2);
    slow_json::Buffer buffer{1000};
    slow_json::dumps(buffer, json3);

    assert_with_message(buffer.string() == R"({"a":5,"b":7.2})", "通过slow_json::dumps序列化结果错误");

    slow_json::polymorphic_dict json4{
            std::pair{"a"_ss, 5}
    };
    slow_json::polymorphic_dict json5{
            std::pair{"b"_ss, 7.2}
    };
    slow_json::polymorphic_dict json6 = slow_json::merge(json4, json5);
    buffer.clear();
    slow_json::dumps(buffer, json6);
    assert_with_message(buffer.string() == R"({"a":5,"b":7.2})", "通过slow_json::dumps序列化结果错误");


}