//
// Created by hy-20 on 2024/8/5.
//
#include "slowjson.hpp"
#include <iostream>

struct Data : public slow_json::ISerializable {
    int x = 1;
    float y = 1.2;
    std::string z = "sjf";

    slow_json::dict get_config() const noexcept override {
        return slow_json::dict{
                std::pair{"x", x},
                std::pair{"y", y},
                std::pair{"z", z}
        };
    }

    void from_config(const slow_json::dict &data) noexcept override {
        x = data["x"].cast<int>();
        y = data["y"].cast<float>();
        z = data["z"].cast<std::string>();
    }
};

void test_serializable_oop() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    Data data;
    Data data2;
    data.x = 123;
    data.y = 345.678;
    data.z = "haha";
    slow_json::Buffer buffer{100};
    slow_json::dumps(buffer, data);
    assert_with_message(buffer.string() == "{\"x\":123,\"y\":345.678,\"z\":\"haha\"}",
                        "通过slow_json::dumps序列化得到的结果不正确");
    slow_json::loads(data2, buffer.string());
    assert_with_message(data2.x == 123, "通过slow_json::loads反序列化得到的结果不正确");
    assert_with_message(data2.y == 345.678f, "通过slow_json::loads反序列化得到的结果不正确");
    assert_with_message(data2.z == "haha", "通过slow_json::loads反序列化得到的结果不正确");
}