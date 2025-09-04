#include <slowjson.hpp>
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
#include <chrono>
#include <iostream>
#include <cassert>
#include <map>
#include <optional>

namespace nested_serialization_deserialization {
    constexpr int ITERATIONS = 300000;
    // 数据结构定义
    struct NestedNode {
        int id;
        float score;
        std::string name;
        std::vector<int> values;
        $config(NestedNode, id, score, name, values);
    };

    struct NestedDrivedNode : public NestedNode {
        double prices[3];
        std::string category;
        std::optional<std::string> comment;
        $$config(<NestedNode>, NestedDrivedNode, prices, category, comment);
    };

    struct NestedObject {
        int timestamp;
        std::vector<NestedDrivedNode> nodes;
        std::map<std::string, float> metrics;
        std::tuple<int, std::string> meta;
        $config(NestedObject, timestamp, nodes, metrics, meta);
    };

    // RapidJSON 序列化/反序列化实现
    namespace rapidjson_helpers {
        void serialize(const NestedNode &obj, rapidjson::Value &val, rapidjson::Document::AllocatorType &allocator) {
            val.SetObject();
            val.AddMember("id", obj.id, allocator);
            val.AddMember("score", obj.score, allocator);
            val.AddMember("name", rapidjson::StringRef(obj.name.c_str()), allocator);
            rapidjson::Value values(rapidjson::kArrayType);
            for (auto v : obj.values) {
                values.PushBack(v, allocator);
            }
            val.AddMember("values", values, allocator);
        }

        void deserialize(NestedNode &obj, const rapidjson::Value &val) {
            obj.id = val["id"].GetInt();
            obj.score = val["score"].GetFloat();
            obj.name = val["name"].GetString();
            obj.values.clear();
            for (const auto &v : val["values"].GetArray()) {
                obj.values.push_back(v.GetInt());
            }
        }

        void serialize(const NestedDrivedNode &obj, rapidjson::Value &val, rapidjson::Document::AllocatorType &allocator) {
            serialize(static_cast<const NestedNode &>(obj), val, allocator);
            rapidjson::Value prices(rapidjson::kArrayType);
            for (int i = 0; i < 3; ++i) {
                prices.PushBack(obj.prices[i], allocator);
            }
            val.AddMember("prices", prices, allocator);
            val.AddMember("category", rapidjson::StringRef(obj.category.c_str()), allocator);
            if (obj.comment) {
                val.AddMember("comment", rapidjson::StringRef(obj.comment->c_str()), allocator);
            } else {
                val.AddMember("comment", rapidjson::Value(rapidjson::kNullType), allocator);
            }
        }

        void deserialize(NestedDrivedNode &obj, const rapidjson::Value &val) {
            deserialize(static_cast<NestedNode &>(obj), val);
            const auto &prices = val["prices"].GetArray();
            for (size_t i = 0; i < 3; ++i) {
                obj.prices[i] = prices[i].GetDouble();
            }
            obj.category = val["category"].GetString();
            obj.comment = val["comment"].IsNull() ? std::nullopt : std::optional<std::string>(val["comment"].GetString());
        }

        void serialize(const NestedObject &obj, rapidjson::Value &val, rapidjson::Document::AllocatorType &allocator) {
            val.SetObject();
            val.AddMember("timestamp", obj.timestamp, allocator);
            rapidjson::Value nodes(rapidjson::kArrayType);
            for (const auto &node : obj.nodes) {
                rapidjson::Value node_val;
                serialize(node, node_val, allocator);
                nodes.PushBack(node_val, allocator);
            }
            val.AddMember("nodes", nodes, allocator);
            rapidjson::Value metrics(rapidjson::kObjectType);
            for (const auto &pair : obj.metrics) {
                metrics.AddMember(rapidjson::StringRef(pair.first.c_str()), pair.second, allocator);
            }
            val.AddMember("metrics", metrics, allocator);
            rapidjson::Value meta(rapidjson::kArrayType);
            meta.PushBack(std::get<0>(obj.meta), allocator);
            meta.PushBack(rapidjson::StringRef(std::get<1>(obj.meta).c_str()), allocator);
            val.AddMember("meta", meta, allocator);
        }

        void deserialize(NestedObject &obj, const rapidjson::Value &val) {
            obj.timestamp = val["timestamp"].GetInt();
            obj.nodes.clear();
            for (const auto &v : val["nodes"].GetArray()) {
                NestedDrivedNode node;
                deserialize(node, v);
                obj.nodes.push_back(node);
            }
            obj.metrics.clear();
            for (const auto &m : val["metrics"].GetObject()) {
                obj.metrics[m.name.GetString()] = m.value.GetFloat();
            }
            const auto &meta = val["meta"].GetArray();
            obj.meta = std::make_tuple(meta[0].GetInt(), meta[1].GetString());
        }
    }

    // SlowJSON 测试
    inline std::string bench_slowjson() {
        slow_json::Buffer buffer{10000};
        // 准备测试数据
        NestedObject obj;
        obj.timestamp = 1722016800;
        obj.nodes.emplace_back(NestedDrivedNode{{1, 95.5f, "Node1", {10, 20, 30}}, {99.99, 149.99, 199.99}, "Electronics", "High quality"});
        obj.nodes.emplace_back(NestedDrivedNode{{2, 85.0f, "Node2", {40, 50}}, {29.99, 39.99, 49.99}, "Accessories", std::nullopt});
        obj.metrics = {{"accuracy", 0.95f}, {"latency", 12.5f}};
        obj.meta = {2024, "Production"};

        // warmup：先生成 JSON 字符串作为反序列化输入
        slow_json::dumps(buffer, obj);
        auto result = buffer.string();
        buffer.clear();

        // === 序列化全流程（对象->JSON字符串）===
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            buffer.clear();
            slow_json::dumps(buffer, obj);
            auto str = buffer.string(); // 计入 string 化
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "SlowJSON 序列化 (对象->JSON字符串): "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        // === 反序列化全流程（JSON字符串->对象）===
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            NestedObject temp;
            slow_json::loads(temp, result); // parse + dict -> 对象
        }
        end = std::chrono::high_resolution_clock::now();
        std::cout << "SlowJSON 反序列化 (JSON字符串->对象): "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        return result;
    }
// RapidJSON 测试
    inline std::string bench_rapidjson() {
        rapidjson::Document doc;
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        auto& allocator = doc.GetAllocator();

        // 准备测试数据
        NestedObject obj;
        obj.timestamp = 1722016800;
        obj.nodes.emplace_back(NestedDrivedNode{{1, 95.5f, "Node1", {10, 20, 30}}, {99.99, 149.99, 199.99}, "Electronics", "High quality"});
        obj.nodes.emplace_back(NestedDrivedNode{{2, 85.0f, "Node2", {40, 50}}, {29.99, 39.99, 49.99}, "Accessories", std::nullopt});
        obj.metrics = {{"accuracy", 0.95f}, {"latency", 12.5f}};
        obj.meta = {2024, "Production"};

        // warmup：先生成 JSON 字符串作为反序列化输入
        rapidjson_helpers::serialize(obj, doc, allocator);
        doc.Accept(writer);
        auto result = std::string(buffer.GetString());
        buffer.Clear();
        writer.Reset(buffer);
        doc.SetObject();

        // === 序列化全流程（对象->JSON字符串）===
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            doc.SetObject();
            rapidjson_helpers::serialize(obj, doc, allocator);
            buffer.Clear();
            writer.Reset(buffer);
            doc.Accept(writer);
            auto str = std::string(buffer.GetString()); // 计入 string 化
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 序列化 (对象->JSON字符串): "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        // === 反序列化全流程（JSON字符串->对象）===
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            NestedObject temp;
            doc.Parse(result.c_str());               // parse
            rapidjson_helpers::deserialize(temp, doc); // 对象化
        }
        end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 反序列化 (JSON字符串->对象): "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        return result;
    }
}

int benchmark_nested_serialization_deserialization() {
    printf("-------复杂嵌套C++对象序列化和反序列化-------\n");
    auto slowjson_result = nested_serialization_deserialization::bench_slowjson();
    auto rapidjson_result = nested_serialization_deserialization::bench_rapidjson();
    assert(slowjson_result == rapidjson_result);
    std::cout << "JSON 输出一致性验证通过！\n";
    return 0;
}
