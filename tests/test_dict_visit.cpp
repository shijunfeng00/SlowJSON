//
// Created by hyzh on 2025/4/23.
//
#include "ostream.hpp"
#include "slowjson.hpp"
void test_dict_visit() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    // 创建测试字典
    slow_json::dict dict{
            {"name", "SlowJSON"},
            {"version", 3.3f},
            {"enabled", true},
            {"list", {1, 2, 3, 4, 5, 6}},
            {"nested_dict", {
                {"a", 1},
                {"b", "2.3asd"},
                {"c", 2.345f},
                {"list2", {1, "2", 3.456789}}
            }}
    };

    // 测试基本键值访问和类型转换
    assert_with_message(dict.is_dict(), "预期为字典类型");
    assert_with_message(std::strcmp(dict["name"].cast<const char*>(), "SlowJSON") == 0, "name 键值应为 SlowJSON");
    assert_with_message(std::abs(dict["version"].cast<float>() - 3.3f) < 1e-6, "version 键值应为 3.3");
    assert_with_message(dict["enabled"].cast<bool>() == true, "enabled Swap enabled 键值应为 true");

    // 测试列表访问
    assert_with_message(dict["list"].is_array(), "list 键应为数组");
    assert_with_message(dict["list"].size() == 6, "list 长度应为 6");
    assert_with_message(dict["list"][2].cast<int>() == 3, "list[2] 应为 3");

    // 测试嵌套字典
    assert_with_message(dict["nested_dict"].is_dict(), "nested_dict 应为字典");
    assert_with_message(dict["nested_dict"]["a"].cast<int>() == 1, "nested_dict.a 应为 1");
    assert_with_message(std::strcmp(dict["nested_dict"]["b"].cast<const char*>(), "2.3asd") == 0, "nested_dict.b 应为 2.3asd");
    assert_with_message(std::abs(dict["nested_dict"]["c"].cast<float>() - 2.345f) < 1e-6, "nested_dict.c 应为 2.345");

    // 测试嵌套列表
    assert_with_message(dict["nested_dict"]["list2"].is_array(), "nested_dict.list2 应为数组");
    assert_with_message(dict["nested_dict"]["list2"].size() == 3, "nested_dict.list2 长度应为 3");
    assert_with_message(dict["nested_dict"]["list2"][0].cast<int>() == 1, "nested_dict.list2[0] 应为 1");
    assert_with_message(std::strcmp(dict["nested_dict"]["list2"][1].cast<const char*>(), "2") == 0, "nested_dict.list2[1] 应为 2");
    assert_with_message(std::abs(dict["nested_dict"]["list2"][2].cast<double>() - 3.456789) < 1e-6, "nested_dict.list2[2] 应为 3.456789");

    // 测试键是否存在
    assert_with_message(dict.contains("name"), "应包含 name 键");
    assert_with_message(!dict.contains("non_existent"), "不应包含 non_existent 键");

    // 测试空字典
    slow_json::dict empty_dict;
    assert_with_message(empty_dict.is_dict(), "空字典应为字典类型");
    assert_with_message(empty_dict.keys().empty(), "空字典应无键");
    assert_with_message(!empty_dict.contains("any"), "空字典不应包含任何键");

    // 测试访问不存在的键
    try {
        slow_json::dict dict{{"key", 42}};
        dict["non_existent"];
        assert(false && "访问不存在的键应抛出异常");

    } catch (const std::exception&) {
        // 预期行为
    }

    // 测试数组越界访问
    try {
        slow_json::dict dict{{"list", {1, 2, 3}}};
        dict["list"][3];
        assert(false && "数组越界访问应抛出异常");
    } catch (const std::exception&) {
        // 预期行为
    }

    // 测试类型转换错误
    try {
        slow_json::dict dict{{"key", "string"}};
        dict["key"].cast<int>();
        assert(false && "错误类型转换应抛出异常");
    } catch (const std::exception&) {
        // 预期行为
    }
}