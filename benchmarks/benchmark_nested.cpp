//
// Created by hyzh on 2025/3/18.
//
#include "slowjson.hpp"
#include <chrono>
#include <iostream>
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
namespace nested {
    constexpr int ITERATIONS = 1000000;

// SlowJSON static_dict 测试
    inline void benchmark_slowjson_static_dict() {
        using slow_json::static_dict;
        auto start = std::chrono::high_resolution_clock::now();
        slow_json::Buffer buffer{1000};
        for (int i = 0; i < ITERATIONS; ++i) {
            auto dict = static_dict{
                    std::pair{"user", static_dict{
                            std::pair{"basic", static_dict{
                                    std::pair{"name", "shijunfeng00"},
                                    std::pair{"age", 19}
                            }},
                            std::pair{"contact", static_dict{
                                    std::pair{"emails", std::tuple{"a@b.com", "c@d.com"}},
                                    std::pair{"address", static_dict{
                                            std::pair{"country", "China"},
                                            std::pair{"city", "Chengdu"}
                                    }}
                            }}
                    }},
                    std::pair{"metadata", static_dict{
                            std::pair{"tags", std::tuple{1, 2, 3}},
                            std::pair{"scores", std::tuple{98.5, 87.5, 92.0}}
                    }}
            };
            slow_json::dumps(buffer, dict);
            buffer.clear();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "static_dict 嵌套序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
    }

// SlowJSON dict 测试
    inline void benchmark_slowjson_dict() {

        slow_json::Buffer buffer{1000};
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            auto dict = slow_json::dict{
                    {"user",{
                        {"basic", {
                            {"name", "shijunfeng00"},
                            {"age", 19}
                        }},
                        {"contact", {
                            {"emails", {"a@b.com", "c@d.com"}}, //这里有歧义
                            {"address", {
                                {"country", "China"},
                                {"city", "Chengdu"}
                            }}
                        }}
                    }},
                    {"metadata", {
                        {"tags",  {1, 2, 3}},
                        {"scores",  {98.5, 87.5, 92.0}}
                    }}
            };
            slow_json::dumps(buffer, dict);
            buffer.clear();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "dict 嵌套序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
    }

// RapidJSON 测试
    inline void benchmark_rapidjson() {
        auto start = std::chrono::high_resolution_clock::now();
        rapidjson::Document doc;
        auto &allocator = doc.GetAllocator();
        for (int i = 0; i < ITERATIONS; ++i) {
            doc.SetObject();
            rapidjson::Value user(rapidjson::kObjectType);
            {
                rapidjson::Value basic(rapidjson::kObjectType);
                basic.AddMember("name", "shijunfeng00", allocator);
                basic.AddMember("age", 19, allocator);
                rapidjson::Value contact(rapidjson::kObjectType);
                rapidjson::Value emails(rapidjson::kArrayType);
                for (auto &&email: {"a@b.com", "c@d.com"}) {
                    emails.PushBack(rapidjson::StringRef(email), allocator);
                }
                contact.AddMember("emails", emails, allocator);
                rapidjson::Value address(rapidjson::kObjectType);
                address.AddMember("country", "China", allocator);
                address.AddMember("city", "Chengdu", allocator);
                contact.AddMember("address", address, allocator);
                user.AddMember("basic", basic, allocator);
                user.AddMember("contact", contact, allocator);
            }
            rapidjson::Value metadata(rapidjson::kObjectType);
            rapidjson::Value tags(rapidjson::kArrayType);
            for (auto tag: {1, 2, 3}) tags.PushBack(tag, allocator);
            rapidjson::Value scores(rapidjson::kArrayType);
            for (auto score: {98.5, 87.5, 92.0}) scores.PushBack(score, allocator);
            metadata.AddMember("tags", tags, allocator);
            metadata.AddMember("scores", scores, allocator);
            doc.AddMember("user", user, allocator);
            doc.AddMember("metadata", metadata, allocator);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            doc.Clear();
            buffer.GetString();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 嵌套序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
    }
}

int benchmark_nested() {
    printf("-------生成复杂嵌套JSON-------\n");
    nested::benchmark_slowjson_static_dict();
    nested::benchmark_slowjson_dict();
    nested::benchmark_rapidjson();
    return 0;
}