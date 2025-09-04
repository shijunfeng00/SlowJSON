//#define debug_slow_json_buffer_print

#include "slowjson.hpp"
#include<iostream>
#include<unordered_set>
#include<unordered_map>

using namespace slow_json::static_string_literals;

void test_stl_loads() {
    printf("run %s\n", __PRETTY_FUNCTION__);

    {
        slow_json::dict dict;
        slow_json::loads(dict,"\"null\"");
        auto result = dict.cast<std::optional<std::string>>();
        assert_with_message(result == "null", "通过slow_json::loads反序列结果不正确");
    }
    {
        slow_json::dict dict=slow_json::details::DictHandler::parse_json_to_dict("null");
        auto result = dict.cast<std::optional<std::string>>();
        assert_with_message(result == std::nullopt, "通过slow_json::loads反序列结果不正确");
    }
    {
        slow_json::dict dict=slow_json::details::DictHandler::parse_json_to_dict("[1,2,3,4,5,6,7]");
        int fuck[7];
        dict.fit(fuck);
        assert_with_message(std::size(fuck) == 7, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[0] == 1, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[1] == 2, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[2] == 3, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[3] == 4, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[4] == 5, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[5] == 6, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[6] == 7, "通过slow_json::loads反序列结果不正确");
    }
    {
        slow_json::dict dict=slow_json::dict::from_string("[1,2,3,4,5,6,7]");
        std::deque<int> fuck;
        dict.fit(fuck);
        assert_with_message(fuck.size() == 7, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[0] == 1, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[1] == 2, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[2] == 3, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[3] == 4, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[4] == 5, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[5] == 6, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck[6] == 7, "通过slow_json::loads反序列结果不正确");
    }
    {
        slow_json::dict dict=slow_json::dict::from_string("[1,2,3,4,5,6,7]");
        std::unordered_set<int> fuck;
        dict.fit(fuck);
        assert_with_message(fuck.size() == 7, "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck.contains(1), "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck.contains(2), "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck.contains(3), "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck.contains(4), "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck.contains(5), "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck.contains(6), "通过slow_json::loads反序列结果不正确");
        assert_with_message(fuck.contains(7), "通过slow_json::loads反序列结果不正确");
    }
    {
        std::string json_str = R"({
        "x":[4],
        "y":[1],
        "z":[2,3,4,5,6]
    })";
        std::unordered_map<std::string, std::vector<int>> fuck;
        slow_json::loads(fuck, json_str);
        slow_json::Buffer buffer{1000};
        slow_json::dumps(buffer, fuck);
        assert_with_message(buffer.string() == "{\"x\":[4],\"y\":[1],\"z\":[2,3,4,5,6]}",
                            "通过slow_json::dumps序列化并通过slow_json::loads反序列号i化之后结果不对");
    }
}
