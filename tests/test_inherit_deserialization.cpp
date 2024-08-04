//
// Created by hy-20 on 2024/7/30.
//

#include "slowjson.hpp"
#include <iostream>
#include <deque>

using namespace slow_json::static_string_literals;

struct Test {
    float value = 123.456;

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"value"_ss, &Test::value}
        };
    }
};

struct Node {
    int xxx = 1;
    float yyy = 1.2345;
    std::string zzz = "shijunfeng";
    Test test;
    std::deque<std::string> dq{"a", "b", "c", "d"};

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

int main() {
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
    std::cout << p.xxx << std::endl;
    std::cout << p.yyy << std::endl;
    std::cout << p.zzz << std::endl;
    std::cout << p.test.value << std::endl;
    std::cout << p.dq.size() << std::endl;
    std::cout << p.hahaha << std::endl;
}