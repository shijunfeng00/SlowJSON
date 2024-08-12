//#define debug_slow_json_buffer_print

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

void test_deserialization() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    assert_with_message(slow_json::concepts::supported<std::shared_ptr<int>> == 0, "相关concepts结果不正确");
    assert_with_message(slow_json::concepts::load_supported<std::shared_ptr<int>> == 0, "相关concepts结果不正确");
    assert_with_message(slow_json::concepts::dump_supported<std::shared_ptr<int>> == 1, "相关concepts结果不正确");
    std::vector<NodeTest> p;
    std::string json_str = R"([{
        "x":4,
        "y":1.2,
        "z":"strings"
    },{
        "x":41,
        "y":12.23,
        "z":"STR"
    }])";

    slow_json::loads(p, json_str);
    assert_with_message(p.front().x == 4, "反序列化结果不正确");
    assert_with_message(p.front().y == 1.2f, "反序列化结果不正确");
    assert_with_message(p.front().z == "strings", "反序列化结果不正确");
    assert_with_message(p.back().x == 41, "反序列化结果不正确");
    assert_with_message(p.back().y == 12.23f, "反序列化结果不正确");
    assert_with_message(p.back().z == "STR", "反序列化结果不正确");
}
