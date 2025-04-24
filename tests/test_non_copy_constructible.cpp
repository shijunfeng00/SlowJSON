//
// Created by hy-20 on 2024/8/14.
//
//
// Created by hy-20 on 2024/7/30.
//
#include "slowjson.hpp"
#include <iostream>
#include <deque>

using namespace slow_json::static_string_literals;

struct NodeNonCopyConstructible {
    int xxx = 1;
    float yyy = 1.2345;
    std::string zzz = "shijunfeng";
    std::deque<std::string> dq{"a", "b", "c", "d"};

    /**
     * 禁用复制构造，表明没有发生额外的数据copy
     * @param node
     */
    NodeNonCopyConstructible(const NodeNonCopyConstructible &node) = delete;

    NodeNonCopyConstructible() {}

    static slow_json::dict get_config() noexcept;
};

slow_json::dict NodeNonCopyConstructible::get_config() noexcept {
    return slow_json::dict{
            std::pair{"xxx"_ss, &NodeNonCopyConstructible::xxx},
            std::pair{"yyy"_ss, &NodeNonCopyConstructible::yyy},
            std::pair{"zzz"_ss, &NodeNonCopyConstructible::zzz},
            std::pair{"dq"_ss, &NodeNonCopyConstructible::dq}
    };
}

void test_non_copy_constructible() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    NodeNonCopyConstructible p;
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
})"||buffer.string()==R"({
    "zzz":"shijunfeng",
    "yyy":1.2345,
    "dq":[
        "a",
        "b",
        "c",
        "d"
    ],
    "xxx":1
})", "slow_json::dumps序列化得到的结果不正确");
    slow_json::static_dict dict{
            std::pair{"object", std::ref(p)} //支持std::ref，这样也不会产生额外的数据拷贝
    };
    buffer.clear();
    slow_json::dumps(buffer, dict);
    assert_with_message(
            buffer.string() == R"({"object":{"dq":["a","b","c","d"],"zzz":"shijunfeng","yyy":1.2345,"xxx":1}})"||
            buffer.string()==R"({"object":{"zzz":"shijunfeng","yyy":1.2345,"dq":["a","b","c","d"],"xxx":1}})",
            "slow_json::dumps序列化得到的结果不正确");
}
