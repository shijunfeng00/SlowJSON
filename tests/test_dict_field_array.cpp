#include "slowjson.hpp"
#include <vector>
#include <optional>
#include <iostream>

using TrackId = int;
using TimeStamp = uint64_t;
using String = std::string;

/**
 * 一个关键点的定义
 */
struct KeyPoint {
    float x;
    float y;

    static auto get_config() noexcept {
        using namespace slow_json::static_string_literals;
        return slow_json::dict{
                {"y"_ss, &KeyPoint::x},
                {"x"_ss, &KeyPoint::y}
        };
    }
};

/**
 * 带有关键点信息的多目标检测结果
 */
struct MultiTargetDetectKeyPointResult {
    KeyPoint key_point[6] = {{1, 1},
                             {2, 2},
                             {3, 3},
                             {4, 4}};    ///<关键点
    static auto get_config() noexcept {
        using namespace slow_json::static_string_literals;
        return slow_json::dict{
                {"key_point"_ss, &MultiTargetDetectKeyPointResult::key_point}
        };
    }
};

void test_dict_field_array() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::Buffer buffer;
    MultiTargetDetectKeyPointResult fuck;
    slow_json::dumps(buffer, fuck);

    assert_with_message(buffer.string() ==
                        R"({"key_point":[{"y":1.0,"x":1.0},{"y":2.0,"x":2.0},{"y":3.0,"x":3.0},{"y":4.0,"x":4.0},{"y":0.0,"x":0.0},{"y":0.0,"x":0.0}]})",
                        "通过slow_json::dumps序列化得到额结果不正确");

    std::string json_str = R"({"key_point":[{"x":11,"y":11},{"x":12,"y":12},{"x":13,"y":13},{"x":14,"y":14},{"x":15,"y":15},{"x":16,"y":16}]})";

    MultiTargetDetectKeyPointResult shit;
    slow_json::loads(shit, json_str);
    char output[200] = {0};
    for (auto &k: shit.key_point) {
        sprintf(output + strlen(output), "(%d,%d) ", (int) k.x, (int) k.y);
    }
    assert_with_message(std::string{output} == "(11,11) (12,12) (13,13) (14,14) (15,15) (16,16) ",
                        "通过slow_json::loads反序列化得到的结果不正确");
}