#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
#include <slowjson.hpp>
#include <chrono>
#include <iostream>
#include <cassert>
#include <vector>

namespace simple_serialization_deserialization {
    struct SimpleUser {
        std::string name;
        int age;
        std::vector<std::string> friends;
        $config(SimpleUser, name, age, friends);
    };

    constexpr int ITERATIONS = 3000000;

    // SlowJSON 测试
    inline std::string benchmark_slowjson() {
        // 准备测试数据
        simple_serialization_deserialization::SimpleUser user{"shijunfeng00", 19, {"zyy", "ly", "hah"}};
        slow_json::Buffer buffer{1000};

        // warmup：先跑一次，拿到 result 作为后续 loads 输入
        slow_json::dumps(buffer, user);
        auto result = buffer.string();
        buffer.clear();

        // === 序列化全流程（对象 -> JSON字符串）===
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            buffer.clear();
            slow_json::dumps(buffer, user);
            auto str = buffer.string();  // 计入 string 化
            // 不清理 str，因为 string 本身析构在循环外，避免额外影响
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "SlowJSON 序列化 (对象->JSON字符串): "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        // === 反序列化全流程（JSON字符串 -> 对象）===
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            simple_serialization_deserialization::SimpleUser temp;
            slow_json::loads(temp, result);  // 内部会 parse -> dict -> 对象
        }
        end = std::chrono::high_resolution_clock::now();
        std::cout << "SlowJSON 反序列化 (JSON字符串->对象): "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        return result;
    }


    // RapidJSON 序列化
    inline void rapidjson_serialize(const SimpleUser &user, rapidjson::Document &doc, rapidjson::Document::AllocatorType &allocator) {
        doc.SetObject();
        doc.AddMember("name", rapidjson::StringRef(user.name.c_str()), allocator);
        doc.AddMember("age", user.age, allocator);
        rapidjson::Value friends(rapidjson::kArrayType);
        for (const auto &f : user.friends) {
            friends.PushBack(rapidjson::StringRef(f.c_str()), allocator);
        }
        doc.AddMember("friends", friends, allocator);
    }

    // RapidJSON 反序列化
    inline void rapidjson_deserialize(SimpleUser &user, const rapidjson::Document &doc) {
        user.name = doc["name"].GetString();
        user.age = doc["age"].GetInt();
        user.friends.clear();
        const auto &friends = doc["friends"];
        for (auto &v : friends.GetArray()) {
            user.friends.emplace_back(v.GetString());
        }
    }

    // RapidJSON 测试
    inline std::string benchmark_rapidjson() {
        using namespace simple_serialization_deserialization;
        rapidjson::Document doc;
        auto& allocator = doc.GetAllocator();
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        // 准备测试数据
        SimpleUser user{"shijunfeng00", 19, {"zyy", "ly", "hah"}};

        // warmup：先跑一次，拿到 result 作为后续 Parse 输入
        rapidjson_serialize(user, doc, allocator);
        doc.Accept(writer);
        auto result = std::string(buffer.GetString());
        buffer.Clear();
        writer.Reset(buffer);
        doc.SetObject();

        // === 序列化全流程（对象 -> JSON字符串）===
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            doc.SetObject();
            rapidjson_serialize(user, doc, allocator);
            buffer.Clear();
            writer.Reset(buffer);
            doc.Accept(writer);
            auto str = std::string(buffer.GetString()); // 计入 string 化
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 序列化 (对象->JSON字符串): "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        // === 反序列化全流程（JSON字符串 -> 对象）===
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            SimpleUser temp;
            doc.Parse(result.c_str());    // 计入 parse
            rapidjson_deserialize(temp, doc);  // 计入对象化
        }
        end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 反序列化 (JSON字符串->对象): "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        return result;
    }
}

int benchmark_simple_serialization_deserialization() {
    printf("-------简单C++对象序列化和反序列化-------\n");
    auto slowjson_result = simple_serialization_deserialization::benchmark_slowjson();
    auto rapidjson_result = simple_serialization_deserialization::benchmark_rapidjson();
    assert(slowjson_result == rapidjson_result);
    std::cout << "JSON 输出一致性验证通过！\n";
    return 0;
}
