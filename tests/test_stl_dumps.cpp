//#define debug_slow_json_buffer_print

#include "slowjson.hpp"
#include<iostream>
#include<unordered_map>
#include<unordered_set>
#include<vector>

void test_stl_dumps() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::Buffer buffer(100);
    long long int x = 123;
    slow_json::dumps(buffer, x);
    assert_with_message(buffer.string() == "123", "通过slow_json::dumps得到的结果不正确");

    buffer.clear();
    double y = 123.567890123456;
    slow_json::dumps(buffer, y);
    assert_with_message(buffer.string() == "123.567890123456", "通过slow_json::dumps得到的结果不正确");

    bool z = true;
    buffer.clear();
    slow_json::dumps(buffer, z);
    assert_with_message(buffer.string() == "true", "通过slow_json::dumps得到的结果不正确");
    buffer.clear();

    std::string str = "这是一个字符串";
    slow_json::dumps(buffer, str);
    assert_with_message(buffer.string() == "\"这是一个字符串\"", "通过slow_json::dumps得到的结果不正确");

    buffer.clear();
    std::vector v{"1", "2", "33", "4444"};
    slow_json::dumps(buffer, v);
    assert_with_message(buffer.string() == "[\"1\",\"2\",\"33\",\"4444\"]", "通过slow_json::dumps得到的结果不正确");

    buffer.clear();
    std::unordered_set s{1.2, 3.4, 5.6, 7.8};
    slow_json::dumps(buffer, s);
    assert_with_message(buffer.string() == "[7.8,5.6,3.4,1.2]", "通过slow_json::dumps得到的结果不正确");

    buffer.clear();
    std::tuple t{1, 2.4, "haha", std::vector{1, 2, 3}};
    slow_json::dumps(buffer, t);
    assert_with_message(buffer.string() == "[1,2.4,\"haha\",[1,2,3]]", "通过slow_json::dumps得到的结果不正确");

    buffer.clear();
    std::unordered_map<std::string, int> mp{{"sjf", 1},
                                            {"jfs", 2}};
    slow_json::dumps(buffer, mp);
    assert_with_message(buffer.string() == "{\"jfs\":2,\"sjf\":1}", "通过slow_json::dumps得到的结果不正确");

    buffer.clear();
    std::optional<int> op_i{5};
    slow_json::dumps(buffer, op_i);
    assert_with_message(buffer.string() == "5", "通过slow_json::dumps得到的结果不正确");

    buffer.clear();
    std::pair pr{5, 3};
    slow_json::dumps(buffer, pr);
    assert_with_message(buffer.string() == "[5,3]", "通过slow_json::dumps得到的结果不正确");


    buffer.clear();
    int arr[] = {9, 8, 7, 6, 5, 4, 3};
    slow_json::dumps(buffer, arr);
    assert_with_message(buffer.string() == "[9,8,7,6,5,4,3]", "通过slow_json::dumps得到的结果不正确");

    buffer.clear();
    auto pmp = std::make_shared<std::unordered_map<std::string, int>>(std::unordered_map<std::string, int>{{"A", 1},
                                                                                                           {"B", 2}});
    slow_json::dumps(buffer, pmp);
    assert_with_message(buffer.string() == "{\"B\":2,\"A\":1}", "通过slow_json::dumps得到的结果不正确");

    buffer.clear();
    slow_json::dumps(buffer, nullptr);
    assert_with_message(buffer.string() == "null", "通过slow_json::dumps得到的结果不正确");

    buffer.clear();
    slow_json::dumps(buffer, std::nullopt);
    assert_with_message(buffer.string() == "null", "通过slow_json::dumps得到的结果不正确");

    buffer.clear();
    auto object = std::tuple{std::vector{"123", "ABC"}, std::unordered_map<std::string, float>{{"XX", 1.23},
                                                                                               {"yy", 5.45}}};
    slow_json::dumps(buffer, object, 4);
    assert_with_message(buffer.string() == R"([
    [
        "123",
        "ABC"
    ],
    {
        "yy":5.4499998,
        "XX":1.23
    }
])", "通过slow_json::dumps得到的结果不正确");

}