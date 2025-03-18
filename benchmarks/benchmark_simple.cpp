//
// Created by hyzh on 2025/3/18.
// 测试对于结构简单的JSON的序列化速度
//
#include "slowjson.hpp"
#include <chrono>
#include <iostream>
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"

constexpr int ITERATIONS = 1000000;

// SlowJSON static_dict 测试
inline void benchmark_slowjson_static_dict() {
    using namespace slow_json;
    Buffer buffer{1000};
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < ITERATIONS; ++i) {
        auto dict = static_dict{
                std::pair{"name", "shijunfeng00"},
                std::pair{"age", 19},
                std::pair{"nation", "China"},
                std::pair{"friend", std::vector{"zyy", "ly", "hah"}}
        };
        dumps(buffer, dict);
        buffer.clear();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "static_dict 序列化: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "ns\n";
}

// SlowJSON dict 测试
inline void benchmark_slowjson_dict() {
    auto start = std::chrono::high_resolution_clock::now();
    slow_json::Buffer buffer{1000};
    for(int i = 0; i < ITERATIONS; ++i) {
        auto dict = slow_json::dict{
                {"name", "shijunfeng00"},
                {"age", 19},
                {"nation", "China"},
                {"friend", {"zyy", "ly", "hah"}}
        };
        slow_json::dumps(buffer, dict);
        buffer.clear();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "dict 序列化: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "ns\n";
}

// RapidJSON 测试
inline void benchmark_rapidjson() {
    rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < ITERATIONS; ++i) {
        doc.AddMember("name", "shijunfeng00", allocator);
        doc.AddMember("age", 19, allocator);
        doc.AddMember("nation", "China", allocator);
        rapidjson::Value friends(rapidjson::kArrayType);
        for(auto&& name : {"zyy", "ly", "hah"}) {
            friends.PushBack(rapidjson::StringRef(name), allocator);
        }
        doc.AddMember("friend", friends, allocator);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        doc.Clear();
        buffer.GetString();
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "RapidJSON 序列化: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "ns\n";
}

int benchmark_simple() {
    printf("-------生成简单JSON-------\n");
    benchmark_slowjson_static_dict();
    benchmark_slowjson_dict();
    benchmark_rapidjson();
    return 0;
}