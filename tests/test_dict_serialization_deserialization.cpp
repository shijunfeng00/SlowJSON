//
// Created by hyzh on 2025/3/13.
//
#include "slowjson.hpp"
using namespace slow_json;
using namespace slow_json::static_string_literals;

// 浮点数比较容差宏
#define double_EQUAL(a, b) (std::abs((a) - (b)) < 1e-6)

// 基本字典测试结构
struct DictTest7845 {
    double x;
    int y;
    std::string z;

    static auto get_config() noexcept {
        return slow_json::dict{
                {"x", &DictTest7845::x},
                {"y", &DictTest7845::y},
                {"z", &DictTest7845::z}
        };
    }
};

// 带列表和继承的测试结构
struct DictListTest7854 : public DictTest7845 {
    DictTest7845 value[3];
    std::vector<std::pair<int, double>> vec;

    static auto get_config() noexcept {
        return slow_json::inherit<DictTest7845>(slow_json::dict{
                {"value", &DictListTest7854::value},
                {"vec", &DictListTest7854::vec}
        });
    }
};

// 测试使用slow_json::dict来进行C++ Class自定义对象的序列化，反序列化，以及继承派生类的处理
void run_dict_serialization_deserialization_inherit() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    // 测试序列化 ==============================================
    Buffer buffer;
    DictListTest7854 dict_list_test;

    // 初始化测试数据
    dict_list_test.vec = {{4, 5.0f}, {7, 15.0f}};
    dict_list_test.value[0] = {1.0f, 2, "3"};
    dict_list_test.value[1] = {11.0f, 22, "33"};
    dict_list_test.value[2] = {114514.0f, 1919810, "shijunfeng@swpu"};
    dict_list_test.x = 1111.2222f;
    dict_list_test.y = 333333;
    dict_list_test.z = "i wanna be the guy";

    // 执行序列化
    slow_json::dumps(buffer, dict_list_test, 4);

    // 验证序列化结果
    assert_with_message(
            buffer.string() == R"({
    "value":[
        {
            "z":"3",
            "y":2,
            "x":1.0
        },
        {
            "z":"33",
            "y":22,
            "x":11.0
        },
        {
            "z":"shijunfeng@swpu",
            "y":1919810,
            "x":114514.0
        }
    ],
    "vec":[
        [
            4,
            5.0
        ],
        [
            7,
            15.0
        ]
    ],
    "x":1111.22216796875,
    "y":333333,
    "z":"i wanna be the guy"
})", "slow_json::dumps序列化结果不符合预期");

    // 测试反序列化 ============================================
    const std::string json_str = R"(
    {
        "value": [
            {
                "z": "3-deserialization",
                "y": 21,
                "x": 1.0
            },
            {
                "z": "33",
                "y": 222,
                "x": 11.0
            },
            {
                "z": "shijunfeng@china",
                "y": 20001026,
                "x": 114514.0
            }
        ],
        "vec": [
            [4, 5.0],
            [7, 15.0]
        ],
        "x": 1111.875415,
        "y": 7845,
        "z": "i wanna be the guy"
    })";

    DictListTest7854 dict_list_test2;
    slow_json::loads(dict_list_test2, json_str);

    // 验证基础字段
    assert_with_message(double_EQUAL(dict_list_test2.x, 1111.875415),
                        "x 字段反序列化失败");
    assert_with_message(dict_list_test2.y == 7845,
                        "y 字段反序列化失败");
    assert_with_message(dict_list_test2.z == "i wanna be the guy",
                        "z 字段反序列化失败");

    // 验证数组字段
    const auto& values = dict_list_test2.value;
    assert_with_message(values[0].x == 1.0f &&
                        values[0].y == 21 &&
                        values[0].z == "3-deserialization",
                        "value[0] 反序列化失败");

    assert_with_message(double_EQUAL(values[1].x, 11.0f) &&
                        values[1].y == 222 &&
                        values[1].z == "33",
                        "value[1] 反序列化失败");

    assert_with_message(double_EQUAL(values[2].x, 114514.0f) &&
                        values[2].y == 20001026 &&
                        values[2].z == "shijunfeng@china",
                        "value[2] 反序列化失败");

    // 验证vector字段
    const auto& vec = dict_list_test2.vec;
    assert_with_message(vec.size() == 2,
                        "vec 大小不正确");

    assert_with_message(vec[0].first == 4 &&
                        double_EQUAL(vec[0].second, 5.0f),
                        "vec[0] 反序列化失败");

    assert_with_message(vec[1].first == 7 &&
                        double_EQUAL(vec[1].second, 15.0f),
                        "vec[1] 反序列化失败");
}