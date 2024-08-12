//#define debug_slow_json_buffer_print

#include "slowjson.hpp"
#include<iostream>

using namespace slow_json::static_string_literals;


void test_static_dict_dumps() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::Buffer buffer(1000);
    constexpr slow_json::static_dict dict{
            std::pair{"test"_ss, 123},
            std::pair{"name"_ss, "shijunfeng"},
            std::pair{"tuple"_ss, slow_json::static_dict{
                    std::pair{"haha"_ss, "wawa"},
                    std::pair{"single"_ss, "boy"}
            }}
    };
    constexpr std::tuple tuple{
            std::pair{"test"_ss, 123},
            std::pair{"name"_ss, "shijunfeng"},
            std::pair{"tuple"_ss, slow_json::static_dict{
                    std::pair{"haha"_ss, "wawa"},
                    std::pair{"single"_ss, "boy"}
            }}
    };
    slow_json::dumps(buffer, dict, 4);
    assert_with_message(buffer.string() == R"({
    "test":123,
    "name":"shijunfeng",
    "tuple":{
        "haha":"wawa",
        "single":"boy"
    }
})", "通过slow_json::dumps序列化结果不正确");

    buffer.clear();

    slow_json::dumps(buffer, tuple, 4);
    assert_with_message(buffer.string() == R"([
    [
        "test",
        123
    ],
    [
        "name",
        "shijunfeng"
    ],
    [
        "tuple",
        {
            "haha":"wawa",
            "single":"boy"
        }
    ]
])", "通过slow_json::dumps序列化结果不正确");

    constexpr auto value = dict["name"_ss];
    assert_with_message(value == "shijunfeng", "通过slow_json::dumps序列化结果不正确");
}