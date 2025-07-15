
#include <slowjson.hpp>
#include <document.h>
#include <stringbuffer.h>
#include <writer.h>
#include <chrono>
#include <iostream>
#include <cassert>

namespace simple {
    constexpr int ITERATIONS = 1000000;

    // SlowJSON static_dict 测试
    inline std::string benchmark_slowjson_static_dict() {
        using namespace slow_json::static_string_literals;
        slow_json::Buffer buffer{1000};
        // 先运行一次以验证输出
        auto dict = slow_json::static_dict{
            std::pair{"name"_ss, "shijunfeng00"},
            std::pair{"age"_ss, 19},
            std::pair{"nation"_ss, "China"},
            std::pair{"friend"_ss, std::tuple{"zyy", "ly", "hah"}}
        };
        slow_json::dumps(buffer, dict);
        auto result = buffer.string();
        buffer.clear();
        // 性能测试，包含构造时间
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            auto dict_inner = slow_json::static_dict{
                std::pair{"name"_ss, "shijunfeng00"},
                std::pair{"age"_ss, 19},
                std::pair{"nation"_ss, "China"},
                std::pair{"friend"_ss, std::tuple{"zyy", "ly", "hah"}}
            };
            slow_json::dumps(buffer, dict_inner);
            buffer.clear();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "static_dict 序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
        return result;
    }

    // SlowJSON dict 测试
    inline std::string benchmark_slowjson_dict() {
        slow_json::Buffer buffer{1000};
        // 先运行一次以验证输出
        auto dict = slow_json::dict{
            {"name", "shijunfeng00"},
            {"age", 19},
            {"nation", "China"},
            {"friend", {"zyy", "ly", "hah"}}
        };
        slow_json::dumps(buffer, dict);
        auto result = buffer.string();
        buffer.clear();
        // 性能测试，包含构造时间
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            auto dict_inner = slow_json::dict{
                {"name", "shijunfeng00"},
                {"age", 19},
                {"nation", "China"},
                {"friend", {"zyy", "ly", "hah"}}
            };
            slow_json::dumps(buffer, dict_inner);
            buffer.clear();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "dict 序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
        return result;
    }

    // RapidJSON 测试
    inline std::string benchmark_rapidjson() {
        rapidjson::Document doc;
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.SetObject();
        auto& allocator = doc.GetAllocator();
        // 先运行一次以验证输出
        doc.AddMember("name", "shijunfeng00", allocator);
        doc.AddMember("age", 19, allocator);
        doc.AddMember("nation", "China", allocator);
        rapidjson::Value friends(rapidjson::kArrayType);
        for (auto&& name : {"zyy", "ly", "hah"}) {
            friends.PushBack(rapidjson::StringRef(name), allocator);
        }
        doc.AddMember("friend", friends, allocator);
        doc.Accept(writer);
        auto result = std::string(buffer.GetString());
        buffer.Clear();
        writer.Reset(buffer);
        doc.SetObject();
        // 性能测试，包含构造时间
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            doc.SetObject();
            doc.AddMember("name", "shijunfeng00", allocator);
            doc.AddMember("age", 19, allocator);
            doc.AddMember("nation", "China", allocator);
            rapidjson::Value friends_inner(rapidjson::kArrayType);
            for (auto&& name : {"zyy", "ly", "hah"}) {
                friends_inner.PushBack(rapidjson::StringRef(name), allocator);
            }
            doc.AddMember("friend", friends_inner, allocator);
            doc.Accept(writer);
            buffer.Clear();
            writer.Reset(buffer);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
        return result;
    }
}

int benchmark_simple() {
    printf("-------生成简单 JSON-------\n");
    // 验证 JSON 输出一致性
    auto static_dict_result = simple::benchmark_slowjson_static_dict();
    auto dict_result = simple::benchmark_slowjson_dict();
    auto rapidjson_result = simple::benchmark_rapidjson();
    assert(static_dict_result == dict_result && dict_result == rapidjson_result);
    std::cout << "JSON 输出一致性验证通过！\n";
    return 0;
}