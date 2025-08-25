//
// Created by hyzh on 2025/7/9.
//
#include "slowjson.hpp"
#include<iostream>

void test_dict_loads(){
    printf("run %s\n", __PRETTY_FUNCTION__);
    std::string json_str = R"({
    "sjf":null,
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

    slow_json::dict dict;
    slow_json::loads(dict,json_str);
    slow_json::Buffer buffer;
    slow_json::dumps(buffer,dict,4);

    assert_with_message(buffer.string()==R"({
    "sjf":null,
    "xxx":19260817,
    "yyy":2022.21,
    "zzz":"SJF",
    "test":{
        "value":654231
    },
    "dq":[
        "A",
        "B",
        "C",
        "D"
    ]
})","slow_json::dict通过slow_json::loads反序列化结果不正确");
    dict["dq"][0]={1,1,4,5,1,4};
    dict["dq"][1]=nullptr;
    dict["test"]={
            {"key1","value1"},
            {"key2","value2"}
    };
    buffer.clear();

    slow_json::dumps(buffer,dict,4);

    std::cout<<"wocao:"<<buffer<<std::endl;

    assert_with_message(buffer.string()==R"({
    "sjf":null,
    "xxx":19260817,
    "yyy":2022.21,
    "zzz":"SJF",
    "test":{
        "key1":"value1",
        "key2":"value2"
    },
    "dq":[
        [
            1,
            1,
            4,
            5,
            1,
            4
        ],
        null,
        "C",
        "D"
    ]
})","slow_json::dict通过slow_json::loads反序列化结果不正确");
}