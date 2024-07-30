//
// Created by hy-20 on 2024/7/18.
//
#include "buffer.hpp"
#include <iostream>

int main() {

    slow_json::Buffer buf{2};

    for (int i = 0; i < 9; i++) {
        buf.push_back('1' + i);
    }

    std::string q = "123";

    std::string p = "caonina_caonima_caonima_caonima";

    buf.append("shijunfeng_huruiting_huruiting_shijunfeng", 41);

    buf.append(p.c_str(), p.size());

    buf.resize(10);

    std::cout << "{" << buf << "}" << std::endl;
    std::cout << buf.string() << std::endl;
}