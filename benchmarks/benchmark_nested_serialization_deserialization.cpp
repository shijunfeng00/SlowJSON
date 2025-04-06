//
// Created by hyzh on 2025/3/18.
//
#include "slowjson.hpp"
#include <chrono>
#include <iostream>
#include <list>
#include "document.h"
#include "stringbuffer.h"
#include "writer.h"
namespace nested_serialization_deserialization {
// ------------------ 数据结构定义 ------------------
    struct NestedNode {
        int x;
        float y;
        std::string z;
        std::vector<int> vec;

        $config(NestedNode, x, y, z, vec);
    };

    struct NestedDrivedNode : public NestedNode {
        int x1;
        float x2;
        std::string z2;
        double arr[16];

        $$config(<NestedNode>, NestedDrivedNode, x1, x2, z2, arr);
    };

    struct NestedObject {
        std::list<NestedNode> list;
        std::tuple<int, float> tp;

        $config(NestedObject, list, tp);
    };

// ------------------ RapidJSON 序列化/反序列化实现 ------------------
    namespace rapidjson_helpers {

        void serialize(const NestedNode &obj, rapidjson::Value &val, rapidjson::Document::AllocatorType &allocator) {
            val.SetObject();
            val.AddMember("x", obj.x, allocator);
            val.AddMember("y", obj.y, allocator);
            val.AddMember("z", rapidjson::StringRef(obj.z.c_str()), allocator);

            rapidjson::Value vec(rapidjson::kArrayType);
            for (auto v: obj.vec) vec.PushBack(v, allocator);
            val.AddMember("vec", vec, allocator);
        }

        void deserialize(NestedNode &obj, const rapidjson::Value &val) {
            obj.x = val["x"].GetInt();
            obj.y = val["y"].GetFloat();
            obj.z = val["z"].GetString();

            obj.vec.clear();
            for (const auto &v: val["vec"].GetArray())
                obj.vec.push_back(v.GetInt());
        }

        void
        serialize(const NestedDrivedNode &obj, rapidjson::Value &val, rapidjson::Document::AllocatorType &allocator) {
            serialize(static_cast<const NestedNode &>(obj), val, allocator);
            val.AddMember("x1", obj.x1, allocator);
            val.AddMember("x2", obj.x2, allocator);
            val.AddMember("z2", rapidjson::StringRef(obj.z2.c_str()), allocator);

            rapidjson::Value arr(rapidjson::kArrayType);
            for (int i = 0; i < 16; ++i) arr.PushBack(obj.arr[i], allocator);
            val.AddMember("arr", arr, allocator);
        }

        void deserialize(NestedDrivedNode &obj, const rapidjson::Value &val) {
            deserialize(static_cast<NestedNode &>(obj), val);
            obj.x1 = val["x1"].GetInt();
            obj.x2 = val["x2"].GetFloat();
            obj.z2 = val["z2"].GetString();

            const auto &arr = val["arr"].GetArray();
            for (size_t i = 0; i < 16; ++i)
                obj.arr[i] = arr[i].GetDouble();
        }

        void serialize(const NestedObject &obj, rapidjson::Value &val, rapidjson::Document::AllocatorType &allocator) {
            val.SetObject();

            rapidjson::Value list(rapidjson::kArrayType);
            for (const auto &item: obj.list) {
                rapidjson::Value itemVal;
                serialize(item, itemVal, allocator);
                list.PushBack(itemVal, allocator);
            }
            val.AddMember("list", list, allocator);

            rapidjson::Value tp(rapidjson::kArrayType);
            tp.PushBack(std::get<0>(obj.tp), allocator);
            tp.PushBack(std::get<1>(obj.tp), allocator);
            val.AddMember("tp", tp, allocator);
        }

        void deserialize(NestedObject &obj, const rapidjson::Value &val) {
            obj.list.clear();
            for (const auto &v: val["list"].GetArray()) {
                NestedNode node;
                deserialize(node, v);
                obj.list.push_back(node);
            }

            const auto &tp = val["tp"].GetArray();
            obj.tp = std::make_tuple(
                    tp[0].GetInt(),
                    tp[1].GetFloat()
            );
        }

    } // namespace rapidjson_helpers

// ------------------ 性能测试 ------------------
    constexpr int ITERATIONS = 1000000;

    inline void bench_slowjson() {
        // 准备测试数据
        NestedObject obj;
        for (int i = 0; i < 5; ++i) {
            obj.list.emplace_back(NestedNode{
                    i,
                    i * 1.1f,
                    "Node" + std::to_string(i),
                    {i * 1, i * 2, i * 3}
            });
        }
        obj.tp = {2024, 3.14f};

        // 序列化测试
        slow_json::Buffer buffer;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            buffer.clear();
            slow_json::dumps(buffer, obj);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "SlowJSON 序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        // 反序列化测试
        std::string json_str = buffer.string();
        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            NestedObject temp;
            slow_json::loads(temp, json_str);
        }
        end = std::chrono::high_resolution_clock::now();
        std::cout << "SlowJSON 反序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
    }

    inline void bench_rapidjson() {
        // 准备相同测试数据
        NestedObject obj;
        for (int i = 0; i < 5; ++i) {
            obj.list.emplace_back(NestedNode{
                    i,
                    i * 1.1f,
                    "Node" + std::to_string(i),
                    {i * 1, i * 2, i * 3}
            });
        }
        obj.tp = {2024, 3.14f};

        // 序列化测试
        rapidjson::Document doc;
        std::string json_str;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            doc.SetObject();
            rapidjson_helpers::serialize(obj, doc, doc.GetAllocator());

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            json_str = buffer.GetString();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";

        // 反序列化测试

        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            doc.Parse(json_str.c_str());
            NestedObject temp;
            rapidjson_helpers::deserialize(temp, doc);
        }
        end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 反序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
    }
}
int benchmark_nested_serialization_deserialization() {
    printf("-------复杂嵌套C++对象序列化和反序列化-------\n");
    nested_serialization_deserialization::bench_slowjson();
    nested_serialization_deserialization::bench_rapidjson();
    return 0;
}
