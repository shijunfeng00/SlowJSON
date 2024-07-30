//
// Created by hy-20 on 2024/7/29.
//
#include "slowjson.hpp"
#include<iostream>

using namespace slow_json::static_string_literals;

struct Node {
    int x = 1;
    std::vector<float> y = {1.2, 3.4};
    std::string z = "STR";

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"x"_ss, &Node::x},
                std::pair{"y"_ss, &Node::y},
                std::pair{"z"_ss, &Node::z}
        };
    }
};

struct NodeList {
    Node nodes[3];

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"nodes"_ss, &NodeList::nodes}
        };
    }
};

int main() {
    slow_json::Buffer buffer(1000);
    NodeList node_list;
    slow_json::dumps(buffer, node_list);
    std::cout << buffer << std::endl;

//    Node node;
//    slow_json::Buffer buffer(1000);
//    slow_json::dumps(buffer,node);
//    std::cout<<buffer<<std::endl;
}