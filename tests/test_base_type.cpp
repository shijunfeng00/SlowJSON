//
// Created by hyzh on 2025/8/29.
//
#include "slowjson.hpp"
void test_base_type() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    std::string json_str = R"({
        "sjf": null,
        "xxx": 19260817,
        "yyy": 2022.21,
        "zzz": "SJF",
        "test": {"value": -654231},
        "dq": [
            "A",
            "B",
            false,
            "C",
            "D"
        ]
    })";

    slow_json::dict dict;
    slow_json::loads(dict, json_str);

    // 验证 BaseType
    assert_with_message(dict["sjf"].get_base_type() == slow_json::details::serializable_wrapper::NULL_TYPE,
                        "sjf 的 BaseType 预期为 NULL_TYPE，实际为 %s",
                        slow_json::details::enum2string(dict["sjf"].get_base_type()).data());
    assert_with_message(dict["xxx"].get_base_type() == slow_json::details::serializable_wrapper::UINT64_TYPE,
                        "xxx 的 BaseType 预期为 UINT64_TYPE，实际为 %s",
                        slow_json::details::enum2string(dict["xxx"].get_base_type()).data());
    assert_with_message(dict["yyy"].get_base_type() == slow_json::details::serializable_wrapper::DOUBLE_TYPE,
                        "yyy 的 BaseType 预期为 DOUBLE_TYPE，实际为 %s",
                        slow_json::details::enum2string(dict["yyy"].get_base_type()).data());
    assert_with_message(dict["zzz"].get_base_type() == slow_json::details::serializable_wrapper::STRING_TYPE,
                        "zzz 的 BaseType 预期为 STRING_TYPE，实际为 %s",
                        slow_json::details::enum2string(dict["zzz"].get_base_type()).data());
    assert_with_message(dict["test"]["value"].get_base_type() == slow_json::details::serializable_wrapper::INT64_TYPE,
                        "test.value 的 BaseType 预期为 INT64_TYPE，实际为 %s",
                        slow_json::details::enum2string(dict["test"]["value"].get_base_type()).data());
    assert_with_message(dict["dq"].get_base_type() == slow_json::details::serializable_wrapper::NOT_FUNDAMENTAL_TYPE,
                        "dq 的 BaseType 预期为 NOT_FUNDAMENTAL_TYPE，实际为 %s",
                        slow_json::details::enum2string(dict["dq"].get_base_type()).data());
    assert_with_message(dict["dq"][2].get_base_type() == slow_json::details::serializable_wrapper::BOOL_TYPE,
                        "dq[2] 的 BaseType 预期为 BOOL_TYPE，实际为 %s",
                        slow_json::details::enum2string(dict["dq"][2].get_base_type()).data());

}