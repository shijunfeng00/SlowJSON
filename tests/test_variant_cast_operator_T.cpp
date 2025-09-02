//
// Created by hyzh on 2025/9/2.
//
#include "slowjson.hpp"

using slow_json::dict;

void test_variant_cast_operator_T() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::Buffer buffer{4096};

    // ====== 花括号初始化（dict套dict，dict套list[dict]）======
    dict d{
            {"test", 19260817},
            {"name", "{shijunfeng}"},
            {"tuple", {
                             {"haha", "wawa"},
                             {"single", "boy"},
                             {"nested_list", {
                                                     dict{
                                                             {"id", 1},
                                                             {"msg", "hello"}
                                                     },
                                                     dict{
                                                             {"id", 2},
                                                             {"msg", "world"}
                                                     }
                                             }}
                     }}
    };

    // ====== 序列化 ======
    slow_json::dumps(buffer, d, 4);
    auto json_str = buffer.string();

    // 验证序列化结果
    assert_with_message(json_str == R"({
    "test":19260817,
    "name":"{shijunfeng}",
    "tuple":{
        "haha":"wawa",
        "single":"boy",
        "nested_list":[
            {
                "id":1,
                "msg":"hello"
            },
            {
                "id":2,
                "msg":"world"
            }
        ]
    }
})", "花括号初始化 + dumps 序列化结果不正确");

    // ====== 反序列化到新 dict ======
    dict d2;
    slow_json::loads(d2, json_str);


    slow_json::cout.set_indent(4);
    slow_json::Buffer b;
    slow_json::dumps(b,d2,4);


    // ====== 测试 cast/operator T ======

    // 基础类型
    assert_with_message(d2["test"].cast<int>() == 19260817, "cast<int> 错误");
    assert_with_message(d2["test"].cast<uint64_t>() == 19260817ULL, "cast<uint64_t> 错误");
    assert_with_message(d2["test"].cast<int64_t>() == 19260817LL, "cast<int64_t> 错误");
    assert_with_message(d2["test"].cast<double>() == 19260817.0, "cast<double> 错误");

    int x = d2["test"]; // operator T()
    assert_with_message(x == 19260817, "operator int() 错误");

    // 字符串
    assert_with_message(d2["name"].cast<std::string>() == "{shijunfeng}", "cast<string> 错误");
    assert_with_message(d2["name"].cast<std::string_view>() == "{shijunfeng}", "cast<string_view> 错误");
    assert_with_message(std::string(d2["name"].cast<const char*>()) == "{shijunfeng}", "cast<const char*> 错误");

    // 子 dict
    auto tuple = d2["tuple"];
    assert_with_message(tuple.is_dict(), "tuple 应为 dict");
    assert_with_message(tuple["haha"].cast<std::string>() == "wawa", "tuple.haha 错误");

    // list[dict]
    auto nested_list = tuple["nested_list"].as_list();
    assert_with_message(nested_list.size() == 2, "nested_list 大小错误");
    assert_with_message(nested_list[0]["id"].cast<int>() == 1, "nested_list[0].id 错误");
    assert_with_message(nested_list[1]["msg"].cast<std::string>() == "world", "nested_list[1].msg 错误");

    // 异常转换
    bool caught = false;
    try { d2["test"].cast<std::string>(); }
    catch (...) { caught = true; }
    assert_with_message(caught, "int -> string 未抛异常");

    caught = false;
    try { d2["name"].cast<int>(); }
    catch (...) { caught = true; }
    assert_with_message(caught, "string -> int 未抛异常");
}
