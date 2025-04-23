//#define debug_slow_json_buffer_print

#include "slowjson.hpp"
#include<iostream>

using namespace slow_json::static_string_literals;


void test_dict_dumps() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::Buffer buffer(1000);
    slow_json::dict dict{
            std::pair{"test", 123},
            std::pair{"name", "shijunfeng"},
            std::pair{"tuple", slow_json::static_dict{
                    std::pair{"haha", "wawa"},
                    std::pair{"single", "boy"}
            }}
    };
    slow_json::dumps(buffer, dict, 4);
    assert_with_message(buffer.string() == R"({
    "name":"shijunfeng",
    "tuple":{
        "haha":"wawa",
        "single":"boy"
    },
    "test":123
})"|| buffer.string() == R"({
    "tuple":{
        "haha":"wawa",
        "single":"boy"
    },
    "name":"shijunfeng",
    "test":123
})", "采用slow_json::dumps序列化得到的结果不正确");
    slow_json::dict dict2{
            {"name", std::string{"str3"}},
            {"value", 123},
            {"enabled", true},
            {"list", {
                             1,
                             "str1",
                             std::string{"str2"},
                             3.4f,
                             std::vector{11, 22, 33, 44},
                             slow_json::dict{
                                     {"a", 1234},
                                     {"b", 456},
                                     {"c", std::tuple<int, const char*>{2, "2"}},
                                     {"list2", {1, "2", 3.456789}}
                             },
                             slow_json::list{1, 2, 3.456789}
                     }},
            {"nested_dict", {
                             {"x", 1},
                             {"y", "2.3asd"},
                             {"z", 2.345f}
                     }}
    };
    buffer.clear();
    slow_json::dumps(buffer,dict2,4);
    assert_with_message(buffer.string()==R"({
    "nested_dict":{
        "y":"2.3asd",
        "z":2.345,
        "x":1
    },
    "list":[
        1,
        "str1",
        "str2",
        3.4,
        [
            11,
            22,
            33,
            44
        ],
        {
            "list2":[
                1,
                "2",
                3.456789
            ],
            "c":[
                2,
                "2"
            ],
            "b":456,
            "a":1234
        },
        [
            1,
            2,
            3.456789
        ]
    ],
    "value":123,
    "enabled":true,
    "name":"str3"
})","slow_json::dumps序列化结恶果不正确")
}