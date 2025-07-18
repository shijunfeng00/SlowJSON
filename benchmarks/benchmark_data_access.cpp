//
// Created by hyzh on 2025/7/18.
//
#include <slowjson.hpp>
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
#include <chrono>
#include <iostream>
#include <cassert>
#include <random>
#include <vector>
#include <string>
#include <functional>

namespace data_access {
    constexpr int ITERATIONS = 10000000;

    // 创建测试字典的函数
    inline slow_json::dict create_test_dict() {
        auto dict = slow_json::dict{
                {"x", {
                              {"xx", 20001026},
                               {"yy", std::string{"wori"}}
                      }},
                {"v", {1, 1, 4, 5, 1, 4}},
                {"list", {110, 1, 2, 3, std::string{"4"}, 5}},
                {"y", std::string{"2.3asd"}},
                {"z", 123456.789},
                {"dd", {
                              {"d1", 1},
                               {"d2", {
                                              {"123", 2.12},
                                              {"45", 6}
                                      }},
                              {"d3", nullptr},
                              {"d4", std::nullopt}
                      }}
        };
        return dict;
    }

    // SlowJSON 测试
    inline std::string benchmark_slowjson(slow_json::dict& current_dict, const std::vector<size_t>& random_indices) {
        slow_json::Buffer buffer{10000};
        // 生成 JSON 字符串用于一致性验证
        slow_json::dumps(buffer, current_dict);
        auto result = buffer.string();
        buffer.clear();

        // 定义固定的访问路径，封装为 lambda 函数
        std::vector<std::function<void(slow_json::dict&, volatile double& sum)>> access_paths = {
                // 路径 1: dict["x"]["xx"].cast<int>()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += static_cast<double>(d["x"]["xx"].cast<int>());
                },
                // 路径 2: dict["x"]["yy"].cast<std::string>()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += d["x"]["yy"].cast<std::string>().length();
                },
                // 路径 3: dict["v"][0].cast<int>()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += static_cast<double>(d["v"][0].cast<int>());
                },
                // 路径 4: dict["v"][2].cast<int>()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += static_cast<double>(d["v"][2].cast<int>());
                },
                // 路径 5: dict["list"][0].cast<int>()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += static_cast<double>(d["list"][0].cast<int>());
                },
                // 路径 6: dict["list"][4].cast<std::string>()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += d["list"][4].cast<std::string>().length();
                },
                // 路径 7: dict["y"].cast<std::string>()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += d["y"].cast<std::string>().length();
                },
                // 路径 8: dict["z"].cast<double>()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += d["z"].cast<double>();
                },
                // 路径 9: dict["dd"]["d1"].cast<int>()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += static_cast<double>(d["dd"]["d1"].cast<int>());
                },
                // 路径 10: dict["dd"]["d2"]["123"].cast<double>()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += d["dd"]["d2"]["123"].cast<double>();
                },
                // 路径 11: dict["dd"]["d2"]["45"].cast<int>()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += static_cast<double>(d["dd"]["d2"]["45"].cast<int>());
                },
                // 路径 12: dict["dd"]["d3"].is_null()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += d["dd"]["d3"].is_null() ? 1.0 : 0.0;
                },
                // 路径 13: dict["dd"]["d4"].is_null()
                [](slow_json::dict& d, volatile double& sum) {
                    sum += d["dd"]["d4"].is_null() ? 1.0 : 0.0;
                }
        };

        // 累加结果，避免优化
        volatile double sum = 0.0;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            access_paths[random_indices[i]](current_dict, sum);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "dict 数据访问: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms (sum=" << sum << ")\n";
        return result;
    }

    // RapidJSON 测试
    inline std::string benchmark_rapidjson(const rapidjson::Document& doc, const std::vector<size_t>& random_indices) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        auto result = std::string(buffer.GetString());
        buffer.Clear();
        writer.Reset(buffer);

        // 定义固定的访问路径，封装为 lambda 函数
        std::vector<std::function<void(const rapidjson::Value&, volatile double& sum)>> access_paths = {
                // 路径 1: doc["x"]["xx"].GetInt()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += static_cast<double>(v["x"]["xx"].GetInt());
                },
                // 路径 2: doc["x"]["yy"].GetStringLength()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += v["x"]["yy"].GetStringLength();
                },
                // 路径 3: doc["v"][0].GetInt()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += static_cast<double>(v["v"][0].GetInt());
                },
                // 路径 4: doc["v"][2].GetInt()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += static_cast<double>(v["v"][2].GetInt());
                },
                // 路径 5: doc["list"][0].GetInt()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += static_cast<double>(v["list"][0].GetInt());
                },
                // 路径 6: doc["list"][4].GetStringLength()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += v["list"][4].GetStringLength();
                },
                // 路径 7: doc["y"].GetStringLength()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += v["y"].GetStringLength();
                },
                // 路径 8: doc["z"].GetDouble()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += v["z"].GetDouble();
                },
                // 路径 9: doc["dd"]["d1"].GetInt()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += static_cast<double>(v["dd"]["d1"].GetInt());
                },
                // 路径 10: doc["dd"]["d2"]["123"].GetDouble()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += v["dd"]["d2"]["123"].GetDouble();
                },
                // 路径 11: doc["dd"]["d2"]["45"].GetInt()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += static_cast<double>(v["dd"]["d2"]["45"].GetInt());
                },
                // 路径 12: doc["dd"]["d3"].IsNull()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += v["dd"]["d3"].IsNull() ? 1.0 : 0.0;
                },
                // 路径 13: doc["dd"]["d4"].IsNull()
                [](const rapidjson::Value& v, volatile double& sum) {
                    sum += v["dd"]["d4"].IsNull() ? 1.0 : 0.0;
                }
        };

        // 累加结果，避免优化
        volatile double sum = 0.0;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            access_paths[random_indices[i]](doc, sum);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 数据访问: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms (sum=" << sum << ")\n";
        return result;
    }
}

int benchmark_data_access() {
    printf("-------复杂嵌套 JSON 数据随机访问-------\n");

    // 创建测试数据
    auto dict = data_access::create_test_dict();
    rapidjson::Document doc;
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    auto& allocator = doc.GetAllocator();
    doc.SetObject();
    rapidjson::Value x(rapidjson::kObjectType);
    x.AddMember("xx", 20001026, allocator); // int
    x.AddMember("yy", rapidjson::Value("wori", allocator).Move(), allocator); // std::string
    doc.AddMember("x", x, allocator);
    rapidjson::Value v(rapidjson::kArrayType);
    for (int val : {1, 1, 4, 5, 1, 4}) {
        v.PushBack(val, allocator); // int
    }
    doc.AddMember("v", v, allocator);
    rapidjson::Value list(rapidjson::kArrayType);
    for (int val : {110, 1, 2, 3}) {
        list.PushBack(val, allocator); // int
    }
    list.PushBack(rapidjson::Value("4", allocator).Move(), allocator); // std::string
    list.PushBack(5, allocator); // int
    doc.AddMember("list", list, allocator);
    doc.AddMember("y", rapidjson::Value("2.3asd", allocator).Move(), allocator); // std::string
    doc.AddMember("z", 123456.789, allocator); // double
    rapidjson::Value dd(rapidjson::kObjectType);
    dd.AddMember("d1", 1, allocator); // int
    rapidjson::Value d2(rapidjson::kObjectType);
    d2.AddMember("123", 2.12, allocator); // double
    d2.AddMember("45", 6, allocator); // int
    dd.AddMember("d2", d2, allocator);
    dd.AddMember("d3", rapidjson::Value(rapidjson::kNullType), allocator);
    dd.AddMember("d4", rapidjson::Value(rapidjson::kNullType), allocator);
    doc.AddMember("dd", dd, allocator);

    // 生成随机索引序列
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, 12); // 13 个路径 (0-12)
    std::vector<size_t> random_indices(data_access::ITERATIONS);
    for (int i = 0; i < data_access::ITERATIONS; ++i) {
        random_indices[i] = dist(gen);
    }

    // 运行基准测试
    auto slowjson_result = data_access::benchmark_slowjson(dict, random_indices);
    auto rapidjson_result = data_access::benchmark_rapidjson(doc, random_indices);
    assert(slowjson_result == rapidjson_result);
    std::cout << "JSON 输出一致性验证通过！\n";
    return 0;
}