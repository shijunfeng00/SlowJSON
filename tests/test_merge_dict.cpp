//
// Created by hy-20 on 2024/8/9.
//
#include "slowjson.hpp"
#include "iostream"

using namespace slow_json::static_string_literals;

void test_merge_dict() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::static_dict json{
            std::pair{"a"_ss, 5}
    };
    slow_json::static_dict json2{
            std::pair{"b"_ss, 7.2}
    };
    slow_json::static_dict json3 = slow_json::details::merge(json, json2);
    slow_json::Buffer buffer{1000};
    slow_json::dumps(buffer, json3);

    assert_with_message(buffer.string() == R"({"a":5,"b":7.2})", "通过slow_json::dumps序列化结果错误");


    {
        slow_json::static_dict d1{std::pair{"a", 1}};
        slow_json::static_dict d2{std::pair{"b", 1}};
        slow_json::static_dict d3{std::pair{"c", 1}};
        auto d4 = slow_json::details::merge(d1, d2, d3);
        buffer.clear();
        slow_json::dumps(buffer, d4);
        assert_with_message(buffer.string() == R"({"a":1,"b":1,"c":1})", "通过slow_json::dumps序列化结果不正确");
    }

    {
        slow_json::dict d1{{"a",1},{"b",2}};
        slow_json::dict d2{{"c",3},{"d",4}};
        slow_json::dict d3{{"e",5},{"f",6}};
        auto d4=slow_json::details::merge(std::move(d1),std::move(d2),std::move(d3));
        buffer.clear();
        slow_json::dumps(buffer, d4);
        assert_with_message(buffer.string() == R"({"a":1,"b":2,"c":3,"d":4,"e":5,"f":6})", "通过slow_json::dumps序列化结果不正确");
    }

    {
        buffer.clear();
        auto d4=slow_json::details::merge(
                slow_json::dict{{"a",1},{"b",2}},
                slow_json::dict{{"c",3},{"d",4}},
                slow_json::dict{{"e",5},{"f",6}});
        buffer.clear();
        slow_json::dumps(buffer, d4);
        assert_with_message(buffer.string() == R"({"a":1,"b":2,"c":3,"d":4,"e":5,"f":6})", "通过slow_json::dumps序列化结果不正确");
    }

}