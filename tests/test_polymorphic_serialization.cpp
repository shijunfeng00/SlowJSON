//
// Created by hy-20 on 2024/7/30.
//
#include "slowjson.hpp"
#include <iostream>
#include <deque>

using namespace slow_json::static_string_literals;

struct Node {
    int xxx = 1;
    float yyy = 1.2345;
    std::string zzz = "shijunfeng";
    std::deque<std::string> dq{"a", "b", "c", "d"};

    static slow_json::polymorphic_dict get_config() noexcept;
};

slow_json::polymorphic_dict Node::get_config() noexcept {
    return slow_json::polymorphic_dict{
            std::pair{"xxx"_ss, &Node::xxx},
            std::pair{"yyy"_ss, &Node::yyy},
            std::pair{"zzz"_ss, &Node::zzz},
            std::pair{"dq"_ss, &Node::dq}
    };
}


int main() {
    Node p;
    slow_json::Buffer buffer;
    slow_json::dumps(buffer, p, 4);
    std::cout << buffer << std::endl;
}