//
// Created by hy-20 on 2024/8/2.
//
#include "slowjson.hpp"
#include "iostream"

enum Color {
    RED,
    GREEN,
    BLUE,
    BLACK
};

int main() {
    auto p = slow_json::string2enum<Color>("BLACK");
    std::cout << slow_json::type_name_v<decltype(p)> << std::endl;
    std::cout << p << std::endl;
    Color color = RED;
    std::cout << slow_json::enum2string(color) << std::endl;
    std::cout << "--------------\n";

    slow_json::Buffer buffer;
    slow_json::dumps(buffer, RED, 4);
    std::cout << buffer << std::endl;

    Color color2;
    slow_json::loads(color2, "\"BLUE\""); //注意，这里是个字符串
    std::cout << (color2 == BLUE);

}
