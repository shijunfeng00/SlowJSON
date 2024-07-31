#include "slowjson.hpp"
#include <vector>
#include <optional>
#include <opencv2/opencv.hpp>

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
        return slow_json::polymorphic_dict{
                std::pair{"x"_ss, &KeyPoint::x},
                std::pair{"y"_ss, &KeyPoint::y}
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
        return slow_json::polymorphic_dict{
                std::pair{"key_point"_ss, &MultiTargetDetectKeyPointResult::key_point}
        };
    }
};

int main() {
    slow_json::Buffer buffer;
    MultiTargetDetectKeyPointResult fuck;
    slow_json::dumps(buffer, fuck);
    std::cout << buffer << std::endl;

    std::string json_str = R"({"key_point":[{"x":11,"y":11},{"x":12,"y":12},{"x":13,"y":13},{"x":14,"y":14},{"x":15,"y":15},{"x":16,"y":16}]})";

    MultiTargetDetectKeyPointResult shit;
    slow_json::loads(shit, json_str);
    for (auto &k: shit.key_point) {
        printf("(%d,%d) ", (int) k.x, (int) k.y);
    }
    std::cout << std::endl;

}