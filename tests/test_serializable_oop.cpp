//
// Created by hy-20 on 2024/8/5.
//
#include "slowjson.hpp"
#include <iostream>

struct Data : public slow_json::ISerializable {
    int x = 1;
    float y = 1.2;
    std::string z = "sjf";

    slow_json::polymorphic_dict get_config() const noexcept override {
        return slow_json::polymorphic_dict{
                std::pair{"x", x},
                std::pair{"y", y},
                std::pair{"z", z}
        };
    }

    void from_config(const slow_json::dynamic_dict &data) noexcept override {
        x = data["x"].cast<int>();
        y = data["y"].cast<float>();
        z = data["z"].cast<std::string>();
    }
};

int main() {
    Data data;
    Data data2;
    data.x = 123;
    data.y = 345.678;
    data.z = "haha";
    slow_json::Buffer buffer{100};
    slow_json::dumps(buffer, data);
    std::cout << buffer << std::endl;
    slow_json::loads(data2, buffer.string());
    std::cout << data2.x << std::endl;
    std::cout << data2.y << std::endl;
    std::cout << data2.z << std::endl;
}