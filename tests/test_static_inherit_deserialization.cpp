//
// Created by hy-20 on 2024/7/30.
//

#include "slowjson.hpp"
#include <iostream>
#include <deque>

using namespace slow_json::static_string_literals;

struct Test {
    float value;

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"value"_ss, &Test::value}
        };
    }
};

struct Node {
    int xxx;
    float yyy;
    std::string zzz;
    Test test;
    std::deque<std::string> dq;

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"xxx"_ss, &Node::xxx},
                std::pair{"yyy"_ss, &Node::yyy},
                std::pair{"zzz"_ss, &Node::zzz},
                std::pair{"test"_ss, &Node::test},
                std::pair{"dq"_ss, &Node::dq}
        };
    }
};

struct Node2 : public Node {
    int hahaha = 2333;

    static constexpr auto get_config() noexcept {
        return slow_json::inherit<Node>(slow_json::static_dict{
                std::pair{"hahaha"_ss, &Node2::hahaha}
        });
    }
};

void test_static_inherit_deserialization() {
    printf("run %s\n", __PRETTY_FUNCTION__);

    Node2 p;
    std::string json_str = R"(
{
    "xxx":1,
    "yyy":1.2345,
    "zzz":"shijunfeng",
    "test":{
        "value":123.456
    },
    "dq":[
        "a",
        "b",
        "c",
        "d"
    ],
    "hahaha":2333
}
)";
    slow_json::loads(p, json_str);
    assert_with_message(p.xxx == 1, "反序列化结果不正确");
    assert_with_message(p.yyy == 1.2345f, "反序列化结果不正确");
    assert_with_message(p.zzz == "shijunfeng", "反序列化结果不正确");
    assert_with_message(p.test.value == 123.456f, "反序列化结果不正确");
    assert_with_message(p.dq.size() == 4, "反序列化结果不正确");
    assert_with_message(p.hahaha == 2333, "反序列化结果不正确");
}