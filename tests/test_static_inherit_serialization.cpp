
#include "slowjson.hpp"
#include <iostream>
#include <deque>
#include <fstream>

using namespace slow_json::static_string_literals;

struct Test2024 {
    float value = 123.456;

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"value"_ss, &Test2024::value}
        };
    }
};

struct Node {
    int xxx = 1;
    float yyy = 1.2345;
    std::string zzz = "shijunfeng";
    Test2024 test;
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

void test_static_inherit_serialization() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    Node2 p;
    slow_json::Buffer buffer{1000};
    slow_json::dumps(buffer, p);
    assert_with_message(
            buffer.string() == R"({"xxx":1,"yyy":1.2345,"zzz":"","test":{"value":123.456},"dq":[],"hahaha":2333})",
            "通过slow_json::dumps序列化得到的结果不正确");
}