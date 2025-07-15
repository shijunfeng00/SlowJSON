
#include <slowjson.hpp>
#include <document.h>
#include <stringbuffer.h>
#include <writer.h>
#include <chrono>
#include <iostream>
#include <cassert>

namespace nested {
    constexpr int ITERATIONS = 200000;

    // SlowJSON static_dict 测试
    inline std::string benchmark_slowjson_static_dict() {
        using namespace slow_json::static_string_literals;
        slow_json::Buffer buffer{10000};
        // 先运行一次以验证输出
        auto dict = slow_json::static_dict{
            std::pair{"timestamp"_ss, 1722016800},
            std::pair{"orders"_ss, std::tuple{
                slow_json::static_dict{
                    std::pair{"order_id"_ss, "ORD-12345"},
                    std::pair{"customer"_ss, slow_json::static_dict{
                        std::pair{"id"_ss, 1001},
                        std::pair{"name"_ss, "Alice Smith"},
                        std::pair{"preferences"_ss, slow_json::static_dict{
                            std::pair{"language"_ss, "en"},
                            std::pair{"currency"_ss, "USD"}
                        }}
                    }},
                    std::pair{"items"_ss, std::tuple{
                        slow_json::static_dict{
                            std::pair{"product"_ss, slow_json::static_dict{
                                std::pair{"id"_ss, 101},
                                std::pair{"name"_ss, "Laptop"},
                                std::pair{"price"_ss, 1299.99}
                            }},
                            std::pair{"quantity"_ss, 1},
                            std::pair{"discount"_ss, 0.1}
                        },
                        slow_json::static_dict{
                            std::pair{"product"_ss, slow_json::static_dict{
                                std::pair{"id"_ss, 205},
                                std::pair{"name"_ss, "Mouse"},
                                std::pair{"price"_ss, 29.99}
                            }},
                            std::pair{"quantity"_ss, 2},
                            std::pair{"discount"_ss, std::nullopt}
                        }
                    }}
                },
                slow_json::static_dict{
                    std::pair{"order_id"_ss, "ORD-12346"},
                    std::pair{"customer"_ss, slow_json::static_dict{
                        std::pair{"id"_ss, 1002},
                        std::pair{"name"_ss, "Bob Johnson"},
                        std::pair{"preferences"_ss, slow_json::static_dict{
                            std::pair{"language"_ss, "es"},
                            std::pair{"currency"_ss, "EUR"}
                        }}
                    }},
                    std::pair{"items"_ss, std::tuple{
                        slow_json::static_dict{
                            std::pair{"product"_ss, slow_json::static_dict{
                                std::pair{"id"_ss, 305},
                                std::pair{"name"_ss, "Keyboard"},
                                std::pair{"price"_ss, 89.99}
                            }},
                            std::pair{"quantity"_ss, 1}
                        }
                    }}
                }
            }},
            std::pair{"system_info"_ss, slow_json::static_dict{
                std::pair{"version"_ss, "1.4.2"},
                std::pair{"environment"_ss, "production"}
            }}
        };
        slow_json::dumps(buffer, dict);
        auto result = buffer.string();
        buffer.clear();
        // 性能测试，包含构造时间
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            auto dict_inner = slow_json::static_dict{
                std::pair{"timestamp"_ss, 1722016800},
                std::pair{"orders"_ss, std::tuple{
                    slow_json::static_dict{
                        std::pair{"order_id"_ss, "ORD-12345"},
                        std::pair{"customer"_ss, slow_json::static_dict{
                            std::pair{"id"_ss, 1001},
                            std::pair{"name"_ss, "Alice Smith"},
                            std::pair{"preferences"_ss, slow_json::static_dict{
                                std::pair{"language"_ss, "en"},
                                std::pair{"currency"_ss, "USD"}
                            }}
                        }},
                        std::pair{"items"_ss, std::tuple{
                            slow_json::static_dict{
                                std::pair{"product"_ss, slow_json::static_dict{
                                    std::pair{"id"_ss, 101},
                                    std::pair{"name"_ss, "Laptop"},
                                    std::pair{"price"_ss, 1299.99}
                                }},
                                std::pair{"quantity"_ss, 1},
                                std::pair{"discount"_ss, 0.1}
                            },
                            slow_json::static_dict{
                                std::pair{"product"_ss, slow_json::static_dict{
                                    std::pair{"id"_ss, 205},
                                    std::pair{"name"_ss, "Mouse"},
                                    std::pair{"price"_ss, 29.99}
                                }},
                                std::pair{"quantity"_ss, 2},
                                std::pair{"discount"_ss, std::nullopt}
                            }
                        }}
                    },
                    slow_json::static_dict{
                        std::pair{"order_id"_ss, "ORD-12346"},
                        std::pair{"customer"_ss, slow_json::static_dict{
                            std::pair{"id"_ss, 1002},
                            std::pair{"name"_ss, "Bob Johnson"},
                            std::pair{"preferences"_ss, slow_json::static_dict{
                                std::pair{"language"_ss, "es"},
                                std::pair{"currency"_ss, "EUR"}
                            }}
                        }},
                        std::pair{"items"_ss, std::tuple{
                            slow_json::static_dict{
                                std::pair{"product"_ss, slow_json::static_dict{
                                    std::pair{"id"_ss, 305},
                                    std::pair{"name"_ss, "Keyboard"},
                                    std::pair{"price"_ss, 89.99}
                                }},
                                std::pair{"quantity"_ss, 1}
                            }
                        }}
                    }
                }},
                std::pair{"system_info"_ss, slow_json::static_dict{
                    std::pair{"version"_ss, "1.4.2"},
                    std::pair{"environment"_ss, "production"}
                }}
            };
            slow_json::dumps(buffer, dict_inner);
            buffer.clear();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "static_dict 嵌套序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
        return result;
    }

    // SlowJSON dict 测试
    inline std::string benchmark_slowjson_dict() {
        slow_json::Buffer buffer{10000};
        // 先运行一次以验证输出
        auto dict = slow_json::dict{
            {"timestamp", 1722016800},
            {"orders", {
                slow_json::dict{
                    {"order_id", "ORD-12345"},
                    {"customer", {
                        {"id", 1001},
                        {"name", "Alice Smith"},
                        {"preferences", {
                            {"language", "en"},
                            {"currency", "USD"}
                        }}
                    }},
                    {"items", {
                        slow_json::dict{
                            {"product", {
                                {"id", 101},
                                {"name", "Laptop"},
                                {"price", 1299.99}
                            }},
                            {"quantity", 1},
                            {"discount", 0.1}
                        },
                        slow_json::dict{
                            {"product", {
                                {"id", 205},
                                {"name", "Mouse"},
                                {"price", 29.99}
                            }},
                            {"quantity", 2},
                            {"discount", std::nullopt}
                        }
                    }}
                },
                slow_json::dict{
                    {"order_id", "ORD-12346"},
                    {"customer", {
                        {"id", 1002},
                        {"name", "Bob Johnson"},
                        {"preferences", {
                            {"language", "es"},
                            {"currency", "EUR"}
                        }}
                    }},
                    {"items", slow_json::list{
                        slow_json::dict{
                            {"product", {
                                {"id", 305},
                                {"name", "Keyboard"},
                                {"price", 89.99}
                            }},
                            {"quantity", 1}
                        }
                    }}
                }
            }},
            {"system_info", {
                {"version", "1.4.2"},
                {"environment", "production"}
            }}
        };
        slow_json::dumps(buffer, dict);
        auto result = buffer.string();
        buffer.clear();
        // 性能测试，包含构造时间
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            auto dict_inner = slow_json::dict{
                {"timestamp", 1722016800},
                {"orders", {
                    slow_json::dict{
                        {"order_id", "ORD-12345"},
                        {"customer", {
                            {"id", 1001},
                            {"name", "Alice Smith"},
                            {"preferences", {
                                {"language", "en"},
                                {"currency", "USD"}
                            }}
                        }},
                        {"items", {
                            slow_json::dict{
                                {"product", {
                                    {"id", 101},
                                    {"name", "Laptop"},
                                    {"price", 1299.99}
                                }},
                                {"quantity", 1},
                                {"discount", 0.1}
                            },
                            slow_json::dict{
                                {"product", {
                                    {"id", 205},
                                    {"name", "Mouse"},
                                    {"price", 29.99}
                                }},
                                {"quantity", 2},
                                {"discount", std::nullopt}
                            }
                        }}
                    },
                    slow_json::dict{
                        {"order_id", "ORD-12346"},
                        {"customer", {
                            {"id", 1002},
                            {"name", "Bob Johnson"},
                            {"preferences", {
                                {"language", "es"},
                                {"currency", "EUR"}
                            }}
                        }},
                        {"items", slow_json::list{
                            slow_json::dict{
                                {"product", {
                                    {"id", 305},
                                    {"name", "Keyboard"},
                                    {"price", 89.99}
                                }},
                                {"quantity", 1}
                            }
                        }}
                    }
                }},
                {"system_info", {
                    {"version", "1.4.2"},
                    {"environment", "production"}
                }}
            };
            slow_json::dumps(buffer, dict_inner);
            buffer.clear();
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "dict 嵌套序列化: "
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
        doc.AddMember("timestamp", 1722016800, allocator);
        rapidjson::Value orders(rapidjson::kArrayType);
        {
            rapidjson::Value order1(rapidjson::kObjectType);
            order1.AddMember("order_id", "ORD-12345", allocator);
            rapidjson::Value customer1(rapidjson::kObjectType);
            customer1.AddMember("id", 1001, allocator);
            customer1.AddMember("name", "Alice Smith", allocator);
            rapidjson::Value preferences1(rapidjson::kObjectType);
            preferences1.AddMember("language", "en", allocator);
            preferences1.AddMember("currency", "USD", allocator);
            customer1.AddMember("preferences", preferences1, allocator);
            order1.AddMember("customer", customer1, allocator);
            rapidjson::Value items1(rapidjson::kArrayType);
            {
                rapidjson::Value item1(rapidjson::kObjectType);
                rapidjson::Value product1(rapidjson::kObjectType);
                product1.AddMember("id", 101, allocator);
                product1.AddMember("name", "Laptop", allocator);
                product1.AddMember("price", 1299.99, allocator);
                item1.AddMember("product", product1, allocator);
                item1.AddMember("quantity", 1, allocator);
                item1.AddMember("discount", 0.1, allocator);
                items1.PushBack(item1, allocator);
                rapidjson::Value item2(rapidjson::kObjectType);
                rapidjson::Value product2(rapidjson::kObjectType);
                product2.AddMember("id", 205, allocator);
                product2.AddMember("name", "Mouse", allocator);
                product2.AddMember("price", 29.99, allocator);
                item2.AddMember("product", product2, allocator);
                item2.AddMember("quantity", 2, allocator);
                item2.AddMember("discount", rapidjson::Value(rapidjson::kNullType), allocator);
                items1.PushBack(item2, allocator);
            }
            order1.AddMember("items", items1, allocator);
            orders.PushBack(order1, allocator);
            rapidjson::Value order2(rapidjson::kObjectType);
            order2.AddMember("order_id", "ORD-12346", allocator);
            rapidjson::Value customer2(rapidjson::kObjectType);
            customer2.AddMember("id", 1002, allocator);
            customer2.AddMember("name", "Bob Johnson", allocator);
            rapidjson::Value preferences2(rapidjson::kObjectType);
            preferences2.AddMember("language", "es", allocator);
            preferences2.AddMember("currency", "EUR", allocator);
            customer2.AddMember("preferences", preferences2, allocator);
            order2.AddMember("customer", customer2, allocator);
            rapidjson::Value items2(rapidjson::kArrayType);
            {
                rapidjson::Value item1(rapidjson::kObjectType);
                rapidjson::Value product1(rapidjson::kObjectType);
                product1.AddMember("id", 305, allocator);
                product1.AddMember("name", "Keyboard", allocator);
                product1.AddMember("price", 89.99, allocator);
                item1.AddMember("product", product1, allocator);
                item1.AddMember("quantity", 1, allocator);
                items2.PushBack(item1, allocator);
            }
            order2.AddMember("items", items2, allocator);
            orders.PushBack(order2, allocator);
        }
        doc.AddMember("orders", orders, allocator);
        rapidjson::Value system_info(rapidjson::kObjectType);
        system_info.AddMember("version", "1.4.2", allocator);
        system_info.AddMember("environment", "production", allocator);
        doc.AddMember("system_info", system_info, allocator);
        doc.Accept(writer);
        auto result = std::string(buffer.GetString());
        buffer.Clear();
        writer.Reset(buffer);
        doc.SetObject();
        // 性能测试，包含构造时间
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            doc.SetObject();
            doc.AddMember("timestamp", 1722016800, allocator);
            rapidjson::Value orders_inner(rapidjson::kArrayType);
            {
                rapidjson::Value order1(rapidjson::kObjectType);
                order1.AddMember("order_id", "ORD-12345", allocator);
                rapidjson::Value customer1(rapidjson::kObjectType);
                customer1.AddMember("id", 1001, allocator);
                customer1.AddMember("name", "Alice Smith", allocator);
                rapidjson::Value preferences1(rapidjson::kObjectType);
                preferences1.AddMember("language", "en", allocator);
                preferences1.AddMember("currency", "USD", allocator);
                customer1.AddMember("preferences", preferences1, allocator);
                order1.AddMember("customer", customer1, allocator);
                rapidjson::Value items1(rapidjson::kArrayType);
                {
                    rapidjson::Value item1(rapidjson::kObjectType);
                    rapidjson::Value product1(rapidjson::kObjectType);
                    product1.AddMember("id", 101, allocator);
                    product1.AddMember("name", "Laptop", allocator);
                    product1.AddMember("price", 1299.99, allocator);
                    item1.AddMember("product", product1, allocator);
                    item1.AddMember("quantity", 1, allocator);
                    item1.AddMember("discount", 0.1, allocator);
                    items1.PushBack(item1, allocator);
                    rapidjson::Value item2(rapidjson::kObjectType);
                    rapidjson::Value product2(rapidjson::kObjectType);
                    product2.AddMember("id", 205, allocator);
                    product2.AddMember("name", "Mouse", allocator);
                    product2.AddMember("price", 29.99, allocator);
                    item2.AddMember("product", product2, allocator);
                    item2.AddMember("quantity", 2, allocator);
                    item2.AddMember("discount", rapidjson::Value(rapidjson::kNullType), allocator);
                    items1.PushBack(item2, allocator);
                }
                order1.AddMember("items", items1, allocator);
                orders_inner.PushBack(order1, allocator);
                rapidjson::Value order2(rapidjson::kObjectType);
                order2.AddMember("order_id", "ORD-12346", allocator);
                rapidjson::Value customer2(rapidjson::kObjectType);
                customer2.AddMember("id", 1002, allocator);
                customer2.AddMember("name", "Bob Johnson", allocator);
                rapidjson::Value preferences2(rapidjson::kObjectType);
                preferences2.AddMember("language", "es", allocator);
                preferences2.AddMember("currency", "EUR", allocator);
                customer2.AddMember("preferences", preferences2, allocator);
                order2.AddMember("customer", customer2, allocator);
                rapidjson::Value items2(rapidjson::kArrayType);
                {
                    rapidjson::Value item1(rapidjson::kObjectType);
                    rapidjson::Value product1(rapidjson::kObjectType);
                    product1.AddMember("id", 305, allocator);
                    product1.AddMember("name", "Keyboard", allocator);
                    product1.AddMember("price", 89.99, allocator);
                    item1.AddMember("product", product1, allocator);
                    item1.AddMember("quantity", 1, allocator);
                    items2.PushBack(item1, allocator);
                }
                order2.AddMember("items", items2, allocator);
                orders_inner.PushBack(order2, allocator);
            }
            doc.AddMember("orders", orders_inner, allocator);
            rapidjson::Value system_info(rapidjson::kObjectType);
            system_info.AddMember("version", "1.4.2", allocator);
            system_info.AddMember("environment", "production", allocator);
            doc.AddMember("system_info", system_info, allocator);
            doc.Accept(writer);
            buffer.Clear();
            writer.Reset(buffer);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "RapidJSON 嵌套序列化: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << "ms\n";
        return result;
    }
}

int benchmark_nested() {
    printf("-------生成复杂嵌套 JSON-------\n");
    // 验证 JSON 输出一致性
    auto static_dict_result = nested::benchmark_slowjson_static_dict();
    auto dict_result = nested::benchmark_slowjson_dict();
    auto rapidjson_result = nested::benchmark_rapidjson();
    assert(static_dict_result == dict_result && dict_result == rapidjson_result);
    std::cout << "JSON 输出一致性验证通过！\n";
    return 0;
}