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
}