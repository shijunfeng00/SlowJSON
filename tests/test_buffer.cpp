//
// Created by hy-20 on 2024/7/18.
//
// #define debug_slow_json_buffer_print

#include "buffer.hpp"
#include <iostream>

void test_buffer() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::Buffer buf{2};

    for (int i = 0; i < 9; i++) {
        buf.push_back('1' + i);
    }

    std::string q = "123";

    std::string p = "caonina_caonima_caonima_caonima";

    buf.append("fuck fuck fuck", 14);

    buf.append(p.c_str(), p.size());

    buf.resize(20);

    assert_with_message("123456789fuck fuck f" == buf.string(), "最终buffer的结果不对");
}

