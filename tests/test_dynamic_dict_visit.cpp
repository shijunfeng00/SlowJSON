//
//#include "dict.hpp"
//#include "ostream.hpp"
//#include "load.hpp"
//#include <stdexcept>
//#include <cmath>
//
//using namespace slow_json;
//
//void test_dynamic_dict_visit() {
//    printf("run %s\n", __PRETTY_FUNCTION__);
//
//    // 创建复杂的测试 JSON 字符串
//    std::string json_data = R"({
//        "x": {"xx": 20001026, "yy": "wori"},
//        "v": [1, 1, 4, 5, 1, 4],
//        "list": [110, 1, 2, 3, "4", 5],
//        "y": "2.3asd",
//        "z": 2.345,
//        "dd": {
//            "d1": 1,
//            "d2": {"123": 2.12, "45": 6},
//            "d3": null,
//            "d4": null
//        }
//    })";
//
//    // 从 JSON 字符串创建 dynamic_dict
//    slow_json::dict dict;
//    slow_json::loads(dict,json_data);
//
//    // 测试基本键值访问和类型转换
//    assert_with_message(dict.contains("x"), "应包含 x 键");
//    assert_with_message(dict["x"].is_dict(), "x 键应为字典");
//    assert_with_message(dict["x"].contains("xx"), "x 应包含 xx 键");
//    assert_with_message(dict["x"]["xx"].as_type<int>(), "x.xx 应为 int 类型");
//    assert_with_message(dict["x"]["xx"].cast<int>() == 20001026, "x.xx 应为 20001026");
//    assert_with_message(dict["x"]["yy"].as_type<std::string_view>(), "x.yy 应为 std::string_view 类型");
//    assert_with_message(dict["x"]["yy"].cast<std::string_view>() == "wori", "x.yy 应为 wori");
//
//    assert_with_message(dict.contains("v"), "应包含 v 键");
//    assert_with_message(dict["v"].is_array(), "v 键应为数组");
//    assert_with_message(dict["v"].size() == 6, "v 长度应为 6");
//    assert_with_message(dict["v"][0].as_type<int>(), "v[0] 应为 int 类型");
//    assert_with_message(dict["v"][0].cast<int>() == 1, "v[0] 应为 1");
//    assert_with_message(dict["v"][1].cast<int>() == 1, "v[1] 应为 1");
//    assert_with_message(dict["v"][2].cast<int>() == 4, "v[2] 应为 4");
//    assert_with_message(dict["v"][3].cast<int>() == 5, "v[3] 应为 5");
//    assert_with_message(dict["v"][4].cast<int>() == 1, "v[4] 应为 1");
//    assert_with_message(dict["v"][5].cast<int>() == 4, "v[5] 应为 4");
//
//    assert_with_message(dict.contains("y"), "应包含 y 键");
//    assert_with_message(dict["y"].as_type<std::string_view>(), "y 键应为 std::string_view 类型");
//    assert_with_message(dict["y"].cast<std::string_view>() == "2.3asd", "y 键值应为 2.3asd");
//
//    assert_with_message(dict.contains("z"), "应包含 z 键");
//    assert_with_message(dict["z"].as_type<float>(), "z 键应为 float 类型");
//    assert_with_message(std::abs(dict["z"].cast<float>() - 2.345f) < 1e-6, "z 键值应为 2.345");
//
//    // 测试空数据
//    assert_with_message(dict["dd"]["d3"].is_null(), "dd.d3 应为空");
//    assert_with_message(dict["dd"]["d4"].is_null(), "dd.d4 应为空");
//
//    // 测试列表访问
//    assert_with_message(dict.contains("list"), "应包含 list 键");
//    assert_with_message(dict["list"].is_array(), "list 键应为数组");
//    assert_with_message(!dict["list"].as_type<int>(), "list 键不应为 int 类型");
//    assert_with_message(dict["list"].size() == 6, "list 长度应为 6");
//    assert_with_message(dict["list"][0].as_type<int>(), "list[0] 应为 int 类型");
//    assert_with_message(dict["list"][0].cast<int>() == 110, "list[0] 应为 110");
//    assert_with_message(dict["list"][4].as_type<std::string_view>(), "list[4] 应为 std::string_view 类型");
//    assert_with_message(dict["list"][4].cast<std::string_view>() == "4", "list[4] 应为 4");
//
//    // 测试嵌套字典
//    assert_with_message(dict.contains("dd"), "应包含 dd 键");
//    assert_with_message(dict["dd"].is_dict(), "dd 键应为字典");
//    assert_with_message(dict["dd"].contains("d1"), "dd 应包含 d1 键");
//    assert_with_message(dict["dd"]["d1"].as_type<int>(), "dd.d1 应为 int 类型");
//    assert_with_message(dict["dd"]["d1"].cast<int>() == 1, "dd.d1 应为 1");
//    assert_with_message(dict["dd"]["d2"].is_dict(), "dd.d2 应为字典");
//
//    assert_with_message(dict["dd"]["d2"].contains("123"), "dd.d2 应包含 123 键");
//    assert_with_message(dict["dd"]["d2"]["123"].as_type<double>(), "dd.d2.123 应为 double 类型");
//    assert_with_message(std::abs(dict["dd"]["d2"]["123"].cast<double>() - 2.12) < 1e-6, "dd.d2.123 应为 2.12");
//
//    // 测试类型名称（dynamic_dict 不直接提供 type_name 方法，跳过此测试或使用 RapidJSON 的类型）
//    assert_with_message(dict["x"].is_dict(), "x 类型应为 dict");
//    assert_with_message(dict["list"].is_array(), "list 类型应为 list");
//    assert_with_message(dict["y"].as_type<std::string_view>(), "y 类型应为 std::string_view");
//    assert_with_message(dict["z"].as_type<float>(), "z 类型应为 float");
//
//    // 测试键是否存在
//    assert_with_message(dict.contains("list"), "应包含 list 键");
//    assert_with_message(!dict.contains("non_existent"), "不应包含 non_existent 键");
//
//    // 测试空字典
//    slow_json::dict empty_dict;
//    assert_with_message(empty_dict.is_null(), "空字典应为空");
//    assert_with_message(!empty_dict.contains("any"), "空字典不应包含任何键");
//
//    // 测试访问不存在的键
//    try {
//        dynamic_dict dict(R"({"key": 42})");
//        dict["non_existent"];
//        assert_with_message(false, "访问不存在的键应抛出 std::runtime_error");
//    } catch (const std::runtime_error& e) {
//        //printf("Caught expected exception for non-existent key: %s\n", e.what());
//    } catch (...) {
//        assert_with_message(false, "访问不存在的键应抛出 std::runtime_error 而非其他异常");
//    }
//
//    // 测试数组越界访问
//    try {
//        dynamic_dict dict(R"({"list": [1, 2, 3]})");
//        dict["list"][3];
//        assert_with_message(false, "数组越界访问应抛出 std::runtime_error");
//    } catch (const std::runtime_error& e) {
//        //printf("Caught expected exception for array out-of-bounds: %s\n", e.what());
//    } catch (...) {
//        assert_with_message(false, "数组越界访问应抛出 std::runtime_error 而非其他异常");
//    }
//
//    // 测试类型转换错误
//    try {
//        dynamic_dict dict(R"({"key": "string"})");
//        dict["key"].cast<int>();
//        assert_with_message(false, "错误类型转换应抛出 std::runtime_error");
//    } catch (const std::runtime_error& e) {
//        //printf("Caught expected exception for type mismatch: %s\n", e.what());
//    } catch (...) {
//        assert_with_message(false, "错误类型转换应抛出 std::runtime_error 而非其他异常");
//    }
//
//    // 测试非字典类型的键访问
//    try {
//        dict["z"]["invalid_key"];
//        assert_with_message(false, "非字典类型的键访问应抛出 std::runtime_error");
//    } catch (const std::runtime_error& e) {
//        //printf("Caught expected exception for non-dict key access: %s\n", e.what());
//    } catch (...) {
//        assert_with_message(false, "非字典类型的键访问应抛出 std::runtime_error 而非其他异常");
//    }
//
//    // 测试非数组类型的索引访问
//    try {
//        dict["z"][0];
//        assert_with_message(false, "非数组类型的索引访问应抛出 std::runtime_error");
//    } catch (const std::runtime_error& e) {
//        //printf("Caught expected exception for non-array index access: %s\n", e.what());
//    } catch (...) {
//        assert_with_message(false, "非数组类型的索引访问应抛出 std::runtime_error 而非其他异常");
//    }
//}
