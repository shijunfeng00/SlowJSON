//#define debug_slow_json_buffer_print

#include "dump.hpp"
#include<iostream>

using namespace slow_json::static_string_literals;


int main() {
    slow_json::Buffer buffer(1000);
    slow_json::polymorphic_dict dict{
            std::pair{"test", 123},
            std::pair{"name", "shijunfeng"},
            std::pair{"tuple", slow_json::static_dict{
                    std::pair{"haha", "wawa"},
                    std::pair{"single", "boy"}
            }}
    };
    slow_json::dumps(buffer, dict, 4);
    std::cout << buffer << std::endl;
}