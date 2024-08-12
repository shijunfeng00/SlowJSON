//
// Created by hy-20 on 2024/7/30.
//
#include "slowjson.hpp"
#include <iostream>
#include <deque>

using namespace slow_json::static_string_literals;

struct NodeP {
    int xxx = 1;
    float yyy = 1.2345;
    std::string zzz = "shijunfeng";
    std::deque<std::string> dq{"a", "b", "c", "d"};

    static slow_json::polymorphic_dict get_config() noexcept;
};

slow_json::polymorphic_dict NodeP::get_config() noexcept {
    return slow_json::polymorphic_dict{
            std::pair{"xxx"_ss, &NodeP::xxx},
            std::pair{"yyy"_ss, &NodeP::yyy},
            std::pair{"zzz"_ss, &NodeP::zzz},
            std::pair{"dq"_ss, &NodeP::dq}
    };
}


void test_polymorphic_serialization() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    NodeP p;
    slow_json::Buffer buffer;
    slow_json::dumps(buffer, p, 4);
    assert_with_message(buffer.string() == R"({
    "xxx":1,
    "yyy":1.2345,
    "zzz":"shijunfeng",
    "dq":[
        "a",
        "b",
        "c",
        "d"
    ]
})", "通过slow_json::dumps序列化得到的结果不正确");
}