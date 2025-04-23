#include "ostream.hpp"
#include "slowjson.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

void test_dict_visit() {
    printf("run %s\n", __PRETTY_FUNCTION__);

    // 创建复杂的测试字典
    slow_json::dict dict{
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

    // 测试基本键值访问和类型转换
    assert_with_message(dict.is_dict(), "预期为字典类型");
    assert_with_message(std::strcmp(dict["name"].cast<std::string>().c_str(), "str3") == 0, "name 键值应为 str3");
    assert_with_message(dict["value"].cast<int>() == 123, "value 键值应为 123");
    assert_with_message(dict["enabled"].cast<bool>(), "enabled 键值应为 true");

    // 测试类型名称
    assert_with_message(dict["name"].type_name() == "std::__cxx11::basic_string<char>", "name 类型应为 std::string");
    assert_with_message(dict["value"].type_name() == "int", "value 类型应为 int");
    assert_with_message(dict["enabled"].type_name() == "bool", "enabled 类型应为 bool");

    // 测试列表访问
    assert_with_message(dict["list"].is_array(), "list 键应为数组");
    assert_with_message(dict["list"].size() == 7, "list 长度应为 7");
    assert_with_message(dict["list"][0].cast<int>() == 1, "list[0] 应为 1");
    assert_with_message(std::strcmp(dict["list"][1].cast<const char*>(), "str1") == 0, "list[1] 应为 str1");
    assert_with_message(std::abs(dict["list"][3].cast<float>() - 3.4f) < 1e-6, "list[3] 应为 3.4");

    // 测试列表中的复杂类型
    assert_with_message(dict["list"][4].cast<std::vector<int>>()[1] == 22, "list[4][1] 应为 22");
    assert_with_message(dict["list"][4].type_name() == "std::vector<int, std::allocator<int> >", "list[4] 类型应为 std::vector<int>");

    // 测试嵌套字典
    assert_with_message(dict["list"][5].is_dict(), "list[5] 应为字典");
    assert_with_message(dict["list"][5]["a"].cast<int>() == 1234, "list[5].a 应为 1234");
    assert_with_message(dict["list"][5]["b"].cast<int>() == 456, "list[5].b 应为 456");
    assert_with_message(std::get<0>(dict["list"][5]["c"].cast<std::tuple<int, const char*>>()) == 2, "list[5].c[0] 应为 2");
    assert_with_message(std::strcmp(std::get<1>(dict["list"][5]["c"].cast<std::tuple<int, const char*>>()), "2") == 0, "list[5].c[1] 应为 2");
    assert_with_message(dict["list"][5]["c"].type_name() == "std::tuple<int, const char*>", "list[5].c 类型应为 std::tuple<int, const char*>");

    // 测试嵌套列表
    assert_with_message(dict["list"][5]["list2"].is_array(), "list[5].list2 应为数组");
    assert_with_message(dict["list"][5]["list2"].size() == 3, "list[5].list2 长度应为 3");
    assert_with_message(dict["list"][5]["list2"][2].type_name() == "double", "list[5].list2[2] 类型应为 double");
    assert_with_message(std::abs(dict["list"][5]["list2"][2].cast<double>() - 3.456789) < 1e-6, "list[5].list2[2] 应为 3.456789");

    // 测试顶级嵌套字典
    assert_with_message(dict["nested_dict"].is_dict(), "nested_dict 应为字典");
    assert_with_message(dict["nested_dict"]["x"].cast<int>() == 1, "nested_dict.x 应为 1");
    assert_with_message(std::strcmp(dict["nested_dict"]["y"].cast<const char*>(), "2.3asd") == 0, "nested_dict.y 应为 2.3asd");
    assert_with_message(std::abs(dict["nested_dict"]["z"].cast<float>() - 2.345f) < 1e-6, "nested_dict.z 应为 2.345");
    assert_with_message(dict["nested_dict"]["z"].type_name() == "float", "nested_dict.z 类型应为 float");

    // 测试顶级列表
    assert_with_message(dict["list"][6].is_array(), "list[6] 应为数组");
    assert_with_message(dict["list"][6].size() == 3, "list[6] 长度应为 3");
    assert_with_message(dict["list"][6][2].type_name() == "double", "list[6][2] 类型应为 double");
    assert_with_message(std::abs(dict["list"][6][2].cast<double>() - 3.456789) < 1e-6, "list[6][2] 应为 3.456789");

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