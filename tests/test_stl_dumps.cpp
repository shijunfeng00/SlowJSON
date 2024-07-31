//#define debug_slow_json_buffer_print

#include "slowjson.hpp"
#include<iostream>
#include<unordered_map>
#include<unordered_set>
#include<vector>

int main() {

    slow_json::Buffer buffer(100);
    long long int x = 123;
    slow_json::dumps(buffer, x);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    double y = 123.567890123456;
    slow_json::dumps(buffer, y);
    std::cout << buffer << std::endl;
    bool z = true;

    buffer.resize(0);
    slow_json::dumps(buffer, z);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    std::string str = "这是一个字符串";
    slow_json::dumps(buffer, str);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    std::vector v{"1", "2", "33", "4444"};
    slow_json::dumps(buffer, v);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    std::unordered_set s{1.2, 3.4, 5.6, 7.8};
    slow_json::dumps(buffer, s);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    std::tuple t{1, 2.4, "haha", std::vector{1, 2, 3}};
    slow_json::dumps(buffer, t);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    std::unordered_map<std::string, int> mp{{"sjf", 1},
                                            {"jfs", 2}};
    slow_json::dumps(buffer, mp);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    std::optional<int> op_i{5};
    slow_json::dumps(buffer, op_i);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    std::pair pr{5, 3};
    slow_json::dumps(buffer, pr);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    int arr[] = {9, 8, 7, 6, 5, 4, 3};
    slow_json::dumps(buffer, arr);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    auto pmp = std::make_shared<std::unordered_map<std::string, int>>(std::unordered_map<std::string, int>{{"A", 1},
                                                                                                           {"B", 2}});
    slow_json::dumps(buffer, pmp);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    slow_json::dumps(buffer, nullptr);
    std::cout << buffer << std::endl;

    buffer.resize(0);
    slow_json::dumps(buffer, std::nullopt);
    std::cout << buffer << std::endl;

    auto object = std::tuple{std::vector{"123", "ABC"}, std::unordered_map<std::string, float>{{"XX", 1.23},
                                                                                               {"yy", 5.45}}};
    slow_json::dumps(buffer, object, 4);
    std::cout << buffer << std::endl;
}