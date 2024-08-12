//
// Created by hy-20 on 2024/7/29.
//
#include "slowjson.hpp"
#include<iostream>

using namespace slow_json::static_string_literals;

struct BaseNode {
    int x = 1;
    std::vector<float> y = {1.2, 3.4};
    std::string z = "STR";

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"x"_ss, &BaseNode::x},
                std::pair{"y"_ss, &BaseNode::y},
                std::pair{"z"_ss, &BaseNode::z}
        };
    }
};

struct LinkedList {
    BaseNode nodes[3];

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"nodes"_ss, &LinkedList::nodes}
        };
    }
};

void test_serialization_nested() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::Buffer buffer(1000);
    LinkedList node_list;
    node_list.nodes[2].z = "change";
    //序列化node_list
    slow_json::dumps(buffer, node_list);
    //反序列化到node_list2上去
    LinkedList node_list2;
    slow_json::loads(node_list2, buffer.string());
    buffer.clear();
    //然后再次序列化node_list2，查看结果是否正确
    slow_json::dumps(buffer, node_list2);
    assert_with_message(buffer.string() ==
                        "{\"nodes\":[{\"x\":1,\"y\":[1.2,3.4],\"z\":\"STR\"},{\"x\":1,\"y\":[1.2,3.4],\"z\":\"STR\"},{\"x\":1,\"y\":[1.2,3.4],\"z\":\"change\"}]}",
                        "通过slow_json::dumps得到的结果不正确");
}