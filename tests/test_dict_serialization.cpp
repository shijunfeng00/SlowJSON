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

    static slow_json::dict get_config() noexcept;
};

slow_json::dict NodeP::get_config() noexcept {
    return slow_json::dict{
            std::pair{"dq"_ss, &NodeP::dq},
            std::pair{"zzz"_ss, &NodeP::zzz},
            std::pair{"yyy"_ss, &NodeP::yyy},
            std::pair{"xxx"_ss, &NodeP::xxx}
    };
}


void test_dict_serialization() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    NodeP p;
    slow_json::Buffer buffer;
    slow_json::dumps(buffer, p, 4);
    assert_with_message(buffer.string() == R"({
    "dq":[
        "a",
        "b",
        "c",
        "d"
    ],
    "zzz":"shijunfeng",
    "yyy":1.2345,
    "xxx":1
})", "通过slow_json::dumps序列化得到的结果不正确");
}