//
// Created by hy-20 on 2024/7/29.
//
#include "slowjson.hpp"
#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<list>
#include<opencv2/opencv.hpp>

namespace slow_json {
    template<>
    struct LoadFromDict<cv::Mat> : public ILoadFromDict<LoadFromDict<cv::Mat>> {
        static void load_impl(cv::Mat &value, const slow_json::dynamic_dict &dict) {
            value = cv::Mat(3, 3, CV_8UC1);
            for (int i = 0; i < dict.size(); i++) {
                for (int j = 0; j < dict[i].size(); j++) {
                    value.at<uint8_t>(i, j) = dict[i][j].cast<int32_t>();
                }
            }
        }
    };
}

int main() {
    std::string json_str = R"([[1,2,3],[4,5,6],[7,8,9]])";
    cv::Mat mat;
    slow_json::loads(mat, json_str);
    std::cout << mat << std::endl;
}