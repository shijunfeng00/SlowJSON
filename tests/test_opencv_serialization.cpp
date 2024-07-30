//
// Created by hy-20 on 2024/7/29.
//
#include "slowjson.hpp"
#include<iostream>
#include<opencv2/opencv.hpp>

using namespace slow_json::static_string_literals;

namespace slow_json {
    template<>
    struct DumpToString<cv::Mat> : public IDumpToString<DumpToString<cv::Mat>> {
        static void dump_impl(Buffer &buffer, const cv::Mat &value) noexcept {
            std::vector<std::vector<int>> vec; //将cv::Mat转化为已知的可以处理的类型，然后调用对应类型的偏特化类的静态方法即可
            for (int i = 0; i < value.cols; i++) {
                std::vector<int> line;
                for (int j = 0; j < value.rows; j++) {
                    line.emplace_back(value.at<int>(i, j));
                }
                vec.emplace_back(std::move(line));
            }
            // slow_json::dumps(buffer,vec);
            DumpToString<decltype(vec)>::dump(buffer, vec);
        }
    };
}

struct ImageMerger {
    int x = 100, y = 120, w = 1000, h = 2000;
    cv::Mat transform_mat = (cv::Mat_<int>(3, 3) << 1, 2, 3, 4, 5, 6, 7, 8, 9);

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"x"_ss, &ImageMerger::x},
                std::pair{"y"_ss, &ImageMerger::y},
                std::pair{"w"_ss, &ImageMerger::w},
                std::pair{"h"_ss, &ImageMerger::h},
                std::pair{"transform_mat"_ss, &ImageMerger::transform_mat}
        };
    }
};

int main() {
    slow_json::Buffer buffer(1000);
    ImageMerger merger;
    slow_json::dumps(buffer, merger);
    std::cout << buffer << std::endl;
}