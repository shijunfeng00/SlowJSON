//
// Created by hy-20 on 2024/7/30.
//
#include "slowjson.hpp"
#include <iostream>

using namespace slow_json::static_string_literals;

struct Test {
    float value;

    static auto get_config() noexcept {
        return slow_json::polymorphic_dict{
                std::pair{"value"_ss, &Test::value}
        };
    }
};

struct Node {
    int xxx;
    float yyy;
    std::string zzz;
    std::deque<std::string> dq;
    Test test;

    static auto get_config() noexcept {
        return slow_json::polymorphic_dict{
                std::pair{"xxx"_ss, &Node::xxx},
                std::pair{"yyy"_ss, &Node::yyy},
                std::pair{"zzz"_ss, &Node::zzz},
                std::pair{"dq"_ss, &Node::dq},
                std::pair{"test"_ss, &Node::test}
        };
    }
};

int main() {
    std::string json_str = R"({
    "xxx":19260817,
    "yyy":2022.21,
    "zzz":"SJF",
    "test":{"value":654231},
    "dq":[
        "A",
        "B",
        "C",
        "D"
    ]
})";
    Node node;
    slow_json::loads(node, json_str);
    std::cout << node.xxx << std::endl;
    std::cout << node.yyy << std::endl;
    std::cout << node.zzz << std::endl;
    std::cout << node.test.value << std::endl;
    for (auto &it: node.dq) {
        std::cout << it << " ";
    }
    std::cout << std::endl;
}