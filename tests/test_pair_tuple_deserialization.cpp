//
// Created by hy-20 on 2024/8/20.
//

#include "slowjson.hpp"
#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<list>

using namespace slow_json::static_string_literals;

struct NodeTest {
    int x;
    float y;
    std::string z;

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"x"_ss, &NodeTest::x},
                std::pair{"y"_ss, &NodeTest::y},
                std::pair{"z"_ss, &NodeTest::z}
        };
    }
};

void test_pair_tuple_deserialization() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    std::string json_str = R"([{
        "x":[4,5.4],
        "y":1.2,
        "z":"strings"
    },{
        "x":41,
        "y":12.23,
        "z":"STR",
        "tp":[1,2.3,"test"]
    }])";
    slow_json::dict dict=slow_json::dict::from_string(json_str);
    auto pr = dict[0]["x"].cast<std::pair<int, float>>();
    assert_with_message(pr.first == 4, "JSON反序列化结果错误");
    assert_with_message(pr.second == 5.4f, "JSON反序列化结果错误");
    auto tp = dict[1]["tp"].cast<std::tuple<int, float, std::string>>();
    assert_with_message(std::get<0>(tp) == 1, "JSON反序列化结果错误");
    assert_with_message(std::get<1>(tp) == 2.3f, "JSON反序列化结果错误");
    assert_with_message(std::get<2>(tp) == "test", "JSON反序列化结果错误");
}
