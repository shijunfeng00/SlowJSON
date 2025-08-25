//
// Created by hyzh on 2025/8/25.
//
#include "slowjson.hpp"

inline std::string generate_test_json() {
    return R"({
        "name": "John Doe",
        "age": 30,
        "scores": [90, 85.5, 95, null],
        "details": {
            "location": "Sichuan",
            "active": true,
            "tags": ["student", "active"],
            "metadata": {
                "id": 123456,
                "rating": 4.5
            }
        },
        "empty": {}
    })";
}


inline auto function_using_extract(){
    auto json_str=generate_test_json();
    slow_json::dict dict;
    slow_json::loads(dict,json_str);
    dict=slow_json::details::DictHandler::parse_json_to_dict(json_str);
    auto details=dict["details"].extract();
    slow_json::Buffer buffer{1024};
    slow_json::dumps(buffer,dict);
    assert_with_message(buffer.string()==R"({"name":"John Doe","age":30,"scores":[90,85.5,95,null],"details":null,"empty":{}})","extract功能不正确");
    return details;
}
void test_dict_extract(){
    printf("run %s\n", __PRETTY_FUNCTION__);
    auto details=function_using_extract();
    slow_json::Buffer buffer;
    slow_json::dumps(buffer,details,4);
    assert_with_message(buffer.string()==R"({
    "location":"Sichuan",
    "active":true,
    "tags":[
        "student",
        "active"
    ],
    "metadata":{
        "id":123456,
        "rating":4.5
    }
})","extract功能不正确");
}