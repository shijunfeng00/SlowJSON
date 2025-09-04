//
// Created by hyzh on 2025/7/6.
//
#include "ostream.hpp"
#include "slowjson.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>

using namespace slow_json;

void test_dict_modify() {
    printf("run %s\n", __PRETTY_FUNCTION__);

    // 创建复杂的测试字典
    dict dict{
            {"x", {{"xx", 20001026}, {"yy", "wori"}}},
            {"list", {110, 1, 2, 3, "4", 5}},
            {"y", "2.3asd"},
            {"z", 2.345f},
            {"dd", {{"d1", 1}, {"d2", {{"123", 2.12},{"456",34}}}}}
    };

    // 测试修改基本类型
    dict["y"] = "new_string";
    assert_with_message(std::strcmp(dict["y"].cast<const char*>(), "new_string") == 0, "y 键值应为 new_string");

    dict["z"] = 5.678f;
    assert_with_message(std::abs(dict["z"].cast<float>() - 5.678f) < 1e-6, "z 键值应为 5.678");

    // 测试修改列表
    dict["list"] = {1, 2, "3", 4};
    assert_with_message(dict["list"].size() == 4, "list 长度应为 4");
    assert_with_message(dict["list"][2].cast<const char*>() == std::string("3"), "list[2] 应为 3");
    assert_with_message(dict["list"][3].cast<int>() == 4, "list[3] 应为 4");

    // 测试修改嵌套字典
    dict["x"] = {{"new_xx", 123}, {"new_yy", "test"}};
    slow_json::cout<<"dict:"<<dict<<std::endl;
    assert_with_message(dict["x"].contains("new_xx"), "x 应包含 new_xx 键");
    assert_with_message(dict["x"]["new_xx"].cast<int>() == 123, "x.new_xx 应为 123");
    assert_with_message(std::strcmp(dict["x"]["new_yy"].cast<const char*>(), "test") == 0, "x.new_yy 应为 test");
    assert_with_message(!dict["x"].contains("xx"), "x 不应包含 xx 键");

    // 测试修改嵌套字典中的元素
    dict["dd"]["d1"] = 999;
    assert_with_message(dict["dd"]["d1"].cast<int>() == 999, "dd.d1 应为 999");

    // 测试修改嵌套字典为新字典
    dict["dd"] = {{"test1", 1.234f}, {"test2", 2}};
    assert_with_message(dict["dd"].contains("test1"), "dd 应包含 test1 键");
    assert_with_message(std::abs(dict["dd"]["test1"].cast<float>() - 1.234f) < 1e-6, "dd.test1 应为 1.234");
    assert_with_message(dict["dd"]["test2"].cast<int>() == 2, "dd.test2 应为 2");

    // 测试修改嵌套列表中的元素
    dict["list"][0] = 999;
    assert_with_message(dict["list"][0].cast<int>() == 999, "list[0] 应为 999");

    // 测试修改不存在的键
    try {
        slow_json::dict dict{{"key", 42}};
        dict["non_existent"] = 100;
        assert_with_message(false, "修改不存在的键应抛出 std::runtime_error");
    } catch (const std::runtime_error& e) {
        //printf("Caught expected exception for non-existent key: %s\n", e.what());
    } catch (...) {
        assert_with_message(false, "修改不存在的键应抛出 std::runtime_error 而非其他异常");
    }

    // 测试数组越界修改
    try {
        slow_json::dict dict{{"list", {1, 2, 3}}};
        dict["list"][3] = 100;
        assert_with_message(false, "数组越界修改应抛出 std::runtime_error");
    } catch (const std::runtime_error& e) {
        //printf("Caught expected exception for array out-of-bounds: %s\n", e.what());
    } catch (...) {
        assert_with_message(false, "数组越界修改应抛出 std::runtime_error 而非其他异常");
    }

    // 测试空字典修改
    slow_json::dict empty_dict;
    try {
        empty_dict["key"] = 42;
        assert_with_message(false, "空字典修改应抛出 std::runtime_error");
    } catch (const std::runtime_error& e) {
        //printf("Caught expected exception for empty dict modification: %s\n", e.what());
    } catch (...) {
        assert_with_message(false, "空字典修改应抛出 std::runtime_error 而非其他异常");
    }

    // 测试 nullptr 键修改
    try {
        dict[nullptr] = 42;
        assert_with_message(false, "nullptr 键修改应抛出 std::runtime_error");
    } catch (const std::runtime_error& e) {
        //printf("Caught expected exception for nullptr key: %s\n", e.what());
    } catch (...) {
        assert_with_message(false, "nullptr 键修改应抛出 std::runtime_error 而非其他异常");
    }

    // 测试非字典类型的键修改
    try {
        dict["z"]["invalid_key"] = 42;
        assert_with_message(false, "非字典类型的键修改应抛出 std::runtime_error");
    } catch (const std::runtime_error& e) {
        //printf("Caught expected exception for non-dict key modification: %s\n", e.what());
    } catch (...) {
        assert_with_message(false, "非字典类型的键修改应抛出 std::runtime_error 而非其他异常");
    }

    // 测试非数组类型的索引修改
    try {
        dict["z"][0] = 42;
        assert_with_message(false, "非数组类型的索引修改应抛出 std::runtime_error");
    } catch (const std::runtime_error& e) {
        //printf("Caught expected exception for non-array index modification: %s\n", e.what());
    } catch (...) {
        assert_with_message(false, "非数组类型的索引修改应抛出 std::runtime_error 而非其他异常");
    }

    // 测试根字典赋值 - 基本类型
    slow_json::dict root_dict{{"key1", 42}, {"key2", "test"}};
    root_dict = 100;
    assert_with_message(root_dict.is_fundamental(), "根字典赋值后应为基本类型");
    assert_with_message(root_dict.cast<int>() == 100, "根字典赋值后应为 100");
//
    // 测试根字典赋值 - 列表
    root_dict = {1, 2, 3, 4};
    assert_with_message(root_dict.is_array(), "根字典赋值后应为列表类型");
    assert_with_message(root_dict.size() == 4, "根字典赋值后列表长度应为 4");
    assert_with_message(root_dict[0].cast<int>() == 1, "根字典赋值后列表[0] 应为 1");
    assert_with_message(root_dict[3].cast<int>() == 4, "根字典赋值后列表[3] 应为 4");

    // 测试根字典赋值 - 字典
    slow_json::dict another_dict{{"new_key", 999}, {"another_key", "value"}};
    root_dict = std::move(another_dict);
    assert_with_message(root_dict.is_dict(), "根字典赋值后应为字典类型");
    assert_with_message(root_dict.contains("new_key"), "根字典赋值后应包含 new_key 键");
    assert_with_message(root_dict["new_key"].cast<int>() == 999, "根字典赋值后 new_key 应为 999");
    assert_with_message(std::strcmp(root_dict["another_key"].cast<const char*>(), "value") == 0, "根字典赋值后 another_key 应为 value");

    // 测试根字典到根字典的赋值
    slow_json::dict dict1{{"a", 1}, {"b", "test"}};
    slow_json::dict dict2{{"c", 3}, {"d", "value"}};
    dict1.copy_key();
    dict1 = std::move(dict2);
    assert_with_message(dict1.is_dict(), "根字典赋值后应为字典类型");
    assert_with_message(dict1.contains("c"), "根字典赋值后应包含 c 键");
    assert_with_message(dict1["c"].cast<int>() == 3, "根字典赋值后 c 应为 3");
    assert_with_message(std::strcmp(dict1["d"].cast<const char*>(), "value") == 0, "根字典赋值后 d 应为 value");
    assert_with_message(!dict1.contains("a"), "根字典赋值后不应包含 a 键");

    // 测试根字典到非根字典的赋值
    slow_json::dict nested_dict{
            {"nested", {
                               {"inner", 42},
                               {"inner4",45}
                       }},
            {"nested2",4}
    };
    slow_json::dict root_dict2{{"key", 123}};
    nested_dict["nested"] = std::move(root_dict2);
    assert_with_message(nested_dict["nested"].is_dict(), "嵌套字典赋值为根字典后应为字典类型");
    assert_with_message(nested_dict["nested"].contains("key"), "嵌套字典赋值后应包含 key 键");
    assert_with_message(nested_dict["nested"]["key"].cast<int>() == 123, "嵌套字典赋值后 key 应为 123");

    // 测试非根字典到根字典的赋值
    slow_json::dict root_dict3{
            std::pair{"outer",
                      slow_json::list{{"inner", 456}}
            }
    };
    slow_json::dict nested_dict2{{"inner_key", 789}};
    root_dict3 = std::move(nested_dict2);
    assert_with_message(root_dict3.is_dict(), "根字典赋值为非根字典后应为字典类型");
    assert_with_message(root_dict3.contains("inner_key"), "根字典赋值后应包含 inner_key 键");
    assert_with_message(root_dict3["inner_key"].cast<int>() == 789, "根字典赋值后 inner_key 应为 789");
}