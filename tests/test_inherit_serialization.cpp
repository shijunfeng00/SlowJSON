
#include "slowjson.hpp"
#include <iostream>
#include <deque>
#include <fstream>

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
    slow_json::Buffer buffer{1000};
    slow_json::dumps(buffer, p, 4);
    std::cout << buffer << std::endl;
    slow_json::static_dict d1{std::pair{"a", "b"}};
    slow_json::static_dict d2{std::pair{"c", "d"}};
    slow_json::merge(d1, slow_json::static_dict{std::pair{"a", "b"}});
}