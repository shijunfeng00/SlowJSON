//#define debug_slow_json_buffer_print

#include "slowjson.hpp"
#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<list>

using namespace slow_json::static_string_literals;

struct Node {
    int x;
    float y;
    std::string z;

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"x"_ss, &Node::x},
                std::pair{"y"_ss, &Node::y},
                std::pair{"z"_ss, &Node::z}
        };
    }
};

int main() {
    std::cout << slow_json::concepts::supported<std::shared_ptr<int>> << std::endl;
    std::cout << slow_json::concepts::load_supported<std::shared_ptr<int>> << std::endl;
    std::cout << slow_json::concepts::dump_supported<std::shared_ptr<int>> << std::endl;

    std::vector<Node> p;
    std::string json_str = R"([{
        "x":4,
        "y":1.2,
        "z":"strings"
    },{
        "x":41,
        "y":12.23,
        "z":"STR"
    }])";
    slow_json::loads(p, json_str);
    std::cout << p.front().x << " " << p.front().y << " " << p.front().z << std::endl;
    std::cout << p.back().x << " " << p.back().y << " " << p.back().z << std::endl;
}
