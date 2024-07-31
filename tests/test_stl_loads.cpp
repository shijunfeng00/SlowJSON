//#define debug_slow_json_buffer_print

#include "slowjson.hpp"
#include<iostream>
#include<unordered_set>
#include<unordered_map>

using namespace slow_json::static_string_literals;

int main() {
    {
        slow_json::dynamic_dict dict("\"null\"");
        auto result = dict.cast<std::optional<std::string>>();
        if (result) {
            std::cout << result.value() << std::endl;
        } else {
            std::cout << "空数据" << std::endl;
        }
    }
    {
        slow_json::dynamic_dict dict("[1,2,3,4,5,6,7]");
        int fuck[7];
        //std::vector<int>fuck;
        dict.fit(fuck);
        for (auto &it: fuck) {
            std::cout << it << " ";
        }
        std::cout << std::endl;
    }
    {
        slow_json::dynamic_dict dict("[1,2,3,4,5,6,7]");
        std::deque<int> fuck;
        dict.fit(fuck);
        for (auto &it: fuck) {
            std::cout << it << " ";
        }
        std::cout << std::endl;
    }
    {
        slow_json::dynamic_dict dict("[1,2,3,4,5,6,7]");
        std::unordered_set<int> fuck;
        dict.fit(fuck);
        for (auto &it: fuck) {
            std::cout << it << " ";
        }
        std::cout << std::endl;
    }
    {
        std::string json_str = R"({
        "x":[4],
        "y":[1],
        "z":[2,3,4,5,6]
    })";
        std::unordered_map<std::string, std::vector<int>> fuck;
        slow_json::loads(fuck, json_str);
        for (auto &it: fuck) {
            std::cout << it.first << " " << it.second.back() << " " << it.second.size() << std::endl;
        }
        std::cout << std::endl;
    }
}
