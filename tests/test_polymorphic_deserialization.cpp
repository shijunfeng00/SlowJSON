//
// Created by hy-20 on 2024/7/30.
//
#include "slowjson.hpp"
#include <iostream>

using namespace slow_json::static_string_literals;

struct TestPolymorphic {
    float value;

    static auto get_config() noexcept {
        return slow_json::polymorphic_dict{
                std::pair{"value"_ss, &TestPolymorphic::value}
        };
    }
};

struct NodePolymorphic {
    int xxx;
    float yyy;
    std::string zzz;
    std::deque<std::string> dq;
    TestPolymorphic test;

    static auto get_config() noexcept {
        return slow_json::polymorphic_dict{
                std::pair{"xxx"_ss, &NodePolymorphic::xxx},
                std::pair{"yyy"_ss, &NodePolymorphic::yyy},
                std::pair{"zzz"_ss, &NodePolymorphic::zzz},
                std::pair{"dq"_ss, &NodePolymorphic::dq},
                std::pair{"test"_ss, &NodePolymorphic::test}
        };
    }
};

void test_polymorphic_deserialization() {
    printf("run %s\n", __PRETTY_FUNCTION__);
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
    NodePolymorphic node;
    slow_json::loads(node, json_str);
    assert_with_message(node.xxx == 19260817, "反序列化结果不正确");
    assert_with_message(node.yyy == 2022.21f, "发序列化结果不正确");
    assert_with_message(node.zzz == "SJF", "反序列化结果不正确");
    assert_with_message(std::abs(node.test.value - 654231.0f) < 0.00001, "反序列化结果不正确");
    assert_with_message(node.dq.size() == 4, "反序列化结果不正确");
    assert_with_message(node.dq[0] == "A", "反序列化结果不正确");
    assert_with_message(node.dq[1] == "B", "反序列化结果不正确");
    assert_with_message(node.dq[2] == "C", "反序列化结果不正确");
    assert_with_message(node.dq[3] == "D", "反序列化结果不正确");
}