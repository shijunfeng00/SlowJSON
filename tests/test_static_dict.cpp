//#define debug_slow_json_buffer_print

#include "dump.hpp"
#include<iostream>

using namespace slow_json::static_string_literals;


int main() {
    slow_json::Buffer buffer(1000);
    constexpr slow_json::static_dict dict{
            std::pair{"test"_ss, 123},
            std::pair{"name"_ss, "shijunfeng"},
            std::pair{"tuple"_ss, slow_json::static_dict{
                    std::pair{"haha"_ss, "wawa"},
                    std::pair{"single"_ss, "boy"}
            }}
    };
    constexpr std::tuple tuple{
            std::pair{"test"_ss, 123},
            std::pair{"name"_ss, "shijunfeng"},
            std::pair{"tuple"_ss, slow_json::static_dict{
                    std::pair{"haha"_ss, "wawa"},
                    std::pair{"single"_ss, "boy"}
            }}
    };
    slow_json::dumps(buffer, dict, 4);
    std::cout << buffer << std::endl;
    buffer.clear();

    slow_json::dumps(buffer, tuple, 4);
    std::cout << buffer << std::endl;


    constexpr auto value = dict["name"_ss];
    std::cout << value << std::endl;
}