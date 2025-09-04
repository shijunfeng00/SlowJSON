//
// Created by hy-20 on 2024/8/12.
//
//
// Created by hy-20 on 2024/7/29.
//
#include "slowjson.hpp"
#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<list>

using namespace slow_json::static_string_literals;

/**
 * 考虑到并不是所有人都会依赖opencv，所以这里我自己写了一个类来假装是opencv的cv::Mat
 */
struct CVMat : private std::vector<std::vector<int>> {
    CVMat(std::size_t height, std::size_t width) {
        for (std::size_t i = 0; i < height; i++) {
            std::vector<int> line;
            for (std::size_t j = 0; j < width; j++) {
                line.emplace_back(0);
            }
            this->emplace_back(line);
        }
        this->cols = width;
        this->rows = height;
    }

    int &at(int i, int j) noexcept {
        return (*this)[i][j];
    }

    const int &at(int i, int j) const noexcept {
        return (*this)[i][j];
    }

    int cols;
    int rows;
};


namespace slow_json {
    template<>
    struct DumpToString<CVMat> : public IDumpToString<DumpToString<CVMat>> {
        static void dump_impl(Buffer &buffer, const CVMat &value) noexcept {
            std::vector<std::vector<int>> vec; //将cv::Mat转化为已知的可以处理的类型，然后调用对应类型的偏特化类的静态方法即可
            for (int i = 0; i < value.cols; i++) {
                std::vector<int> line;
                for (int j = 0; j < value.rows; j++) {
                    line.emplace_back(value.at(i, j));
                }
                vec.emplace_back(std::move(line));
            }
            DumpToString<decltype(vec)>::dump(buffer, vec);
        }
    };

    template<>
    struct LoadFromDict<CVMat> : public ILoadFromDict<LoadFromDict<CVMat>> {
        static void load_impl(CVMat &value, const slow_json::dict &dict) {
            value = CVMat(3, 3);
            for (int i = 0; i < dict.size(); i++) {
                for (int j = 0; j < dict[i].size(); j++) {
                    value.at(i, j) = dict[i][j].cast<int32_t>();
                }
            }
        }
    };
}

struct ImageMerger {
    int x = 100, y = 120, w = 1000, h = 2000;
    CVMat transform_mat{3, 3};

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


void test_non_intrusive_serialization_deserialization() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    std::string json_str = "[[9,8,7],[6,5,4],[3,2,1]]";
    CVMat mat(3, 3);
    for (int i = 1; i <= 3; i++) {
        for (int j = 1; j <= 3; j++) {
            mat.at(i - 1, j - 1) = i * 3 + j - 3;
        }
    }
    slow_json::loads(mat, json_str);
    slow_json::Buffer buffer{1000};
    slow_json::dumps(buffer, mat);
    assert_with_message(buffer.string() == json_str, "序列化的结果和反序列化的结果不一致");
    ImageMerger merger;
    merger.transform_mat = mat;
    buffer.clear();
    slow_json::dumps(buffer, merger, 4);
    std::string result_json_str = R"({
    "x":100,
    "y":120,
    "w":1000,
    "h":2000,
    "transform_mat":[
        [
            9,
            8,
            7
        ],
        [
            6,
            5,
            4
        ],
        [
            3,
            2,
            1
        ]
    ]
})";
    assert_with_message(buffer.string() == result_json_str, "序列化结果和反序列化结果不一致");
}