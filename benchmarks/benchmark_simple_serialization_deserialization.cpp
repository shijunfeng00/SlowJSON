#include "slowjson.hpp"
#include <chrono>
#include <iostream>
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
namespace simple_serialization_deserialization {
    struct SimpleUser {
        std::string name;
        int age;
        std::vector<std::string> friends;

        // SlowJSON 配置
        $config(SimpleUser, name, age, friends);
    };

    constexpr int ITERATIONS = 1000000;
// SlowJSON 测试 ==============================================
    inline void benchmark_slowjson() {
        SimpleUser user{"shijunfeng00", 19, {"zyy", "ly", "hah"}};
        slow_json::Buffer buffer;
        std::string json_str;

        // 序列化
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            buffer.clear();
            slow_json::dumps(buffer, user);
            json_str = buffer.string(); // 确保完成序列化
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "SlowJSON 序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        // 反序列化
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            SimpleUser temp;
            slow_json::loads(temp, json_str);
        }
        end = std::chrono::high_resolution_clock::now();
        std::cout << "SlowJSON 反序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
    }

// RapidJSON 测试 ==============================================
    inline void rapidjson_serialize(const SimpleUser &user, rapidjson::Document &doc) {
        auto &allocator = doc.GetAllocator();
        doc.SetObject();

        doc.AddMember("name", rapidjson::StringRef(user.name.c_str()), allocator);
        doc.AddMember("age", user.age, allocator);

        rapidjson::Value friends(rapidjson::kArrayType);
        for (const auto &f: user.friends) {
            friends.PushBack(rapidjson::StringRef(f.c_str()), allocator);
        }
        doc.AddMember("friends", friends, allocator);
    }

    inline void rapidjson_deserialize(SimpleUser &user, const rapidjson::Document &doc) {
        user.name = doc["name"].GetString();
        user.age = doc["age"].GetInt();

        user.friends.clear();
        const auto &friends = doc["friends"];
        for (auto &v: friends.GetArray()) {
            user.friends.emplace_back(v.GetString());
        }
    }

    inline void benchmark_rapidjson() {
        SimpleUser user{"shijunfeng00", 19, {"zyy", "ly", "hah"}};
        std::string json_str;

        // 序列化
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            rapidjson::Document doc;
            rapidjson_serialize(user, doc);
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            json_str = buffer.GetString(); // 确保完成序列化
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        // 反序列化


        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            SimpleUser temp;
            rapidjson::Document doc;
            doc.Parse(json_str.c_str());
            rapidjson_deserialize(temp, doc);
        }
        end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 反序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
    }
}
int benchmark_simple_serialization_deserialization() {
    printf("-------简单C++对象序列化和反序列化-------\n");
    simple_serialization_deserialization::benchmark_slowjson();
    simple_serialization_deserialization::benchmark_rapidjson();
    return 0;
}