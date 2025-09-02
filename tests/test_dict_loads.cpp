//
// Created by hyzh on 2025/7/9.
//
#include "slowjson.hpp"

void test_dict_loads() {
    printf("run %s\n", __PRETTY_FUNCTION__);

    // ====== 复杂的嵌套 JSON ======
    std::string json_str = R"({"null_field":null,"int_field":19260817,"float_field":2022.21,"string_field":"SJF","bool_field":true,"object_field":{"empty_obj":{},"nested_dict":{"id":123,"tags":["alpha","beta","gamma"],"arr_of_dicts":[{"k1":"v1","flag":true},{"k2":"v2","flag":false}]}},"list_field":["A",123,null,{"deep":{"msg":"hello","nums":[1,2,3,4]}},[true,false,null]]})";

    // ====== loads ======
    slow_json::dict dict;
    slow_json::loads(dict, json_str);

    // ====== dumps ======
    slow_json::Buffer buffer;
    slow_json::dumps(buffer, dict);

    // ====== 再 loads/dumps 一次，确保闭环 ======
    slow_json::dict dict2;
    slow_json::loads(dict2, buffer.string());

    slow_json::Buffer buffer2;
    slow_json::dumps(buffer2, dict2);

    // ====== 验证字符串一致性 ======
    assert_with_message(buffer.string() == buffer2.string(),
                        "复杂嵌套 JSON 在 dumps → loads → dumps 过程中不一致");
    assert_with_message(buffer.string() == json_str,
                        "复杂嵌套 JSON 在 dumps → loads → dumps 过程中不一致");
}
