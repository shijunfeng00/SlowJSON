#include "slowjson.hpp"
#include <iostream>
#include <climits>
#include <cstdint>
#include <vector>
#include <type_traits>
#include <string>

void test_integer_dumps() {
    printf("run %s\n", __PRETTY_FUNCTION__);
    slow_json::Buffer buffer(100);
    
    // 辅助宏用于生成错误信息
    #define ASSERT_INT_SERIALIZATION(value, expected) \
        do { \
            buffer.clear(); \
            slow_json::dumps(buffer, (value)); \
            const std::string actual = buffer.string(); \
            const std::string exp_str = (expected); \
            assert_with_message( \
                actual == exp_str, \
                "slow_json::dumps序列化错误，类型=%s, 输入值=%lld, 预期结果='%s'，实际结果='%s'", \
                #value, \
                static_cast<long long>(value), \
                exp_str.c_str(), \
                actual.c_str() \
            ); \
        } while(0)
    
    // 测试 int8_t
    {
        int8_t values[] = {0, 1, -1, 2, -2, 127, -128};
        const char* expected[] = {"0", "1", "-1", "2", "-2", "127", "-128"};
        
        for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            ASSERT_INT_SERIALIZATION(values[i], expected[i]);
        }
    }
    
    // 测试 uint8_t
    {
        uint8_t values[] = {0, 1, 2, 127, 128, 255};
        const char* expected[] = {"0", "1", "2", "127", "128", "255"};
        
        for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            ASSERT_INT_SERIALIZATION(values[i], expected[i]);
        }
    }
    
    // 测试 int16_t
    {
        int16_t values[] = {0, 1, -1, 32767, -32768};
        const char* expected[] = {"0", "1", "-1", "32767", "-32768"};
        
        for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            ASSERT_INT_SERIALIZATION(values[i], expected[i]);
        }
    }
    
    // 测试 uint16_t
    {
        uint16_t values[] = {0, 1, 32767, 32768, 65535};
        const char* expected[] = {"0", "1", "32767", "32768", "65535"};
        
        for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            ASSERT_INT_SERIALIZATION(values[i], expected[i]);
        }
    }
    
    // 测试 int32_t 边界值
    {
        int32_t values[] = {0, 1, -1, INT32_MAX, INT32_MIN};
        const char* expected[] = {"0", "1", "-1", "2147483647", "-2147483648"};
        
        for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            ASSERT_INT_SERIALIZATION(values[i], expected[i]);
        }
    }
    
    // 测试 uint32_t 边界值
    {
        uint32_t values[] = {0, 1, UINT32_MAX};
        const char* expected[] = {"0", "1", "4294967295"};
        
        for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            ASSERT_INT_SERIALIZATION(values[i], expected[i]);
        }
    }
    
    // 测试 int64_t 边界值
    {
        int64_t values[] = {0, 1, -1, INT64_MAX, INT64_MIN};
        const char* expected[] = {"0", "1", "-1", "9223372036854775807", "-9223372036854775808"};
        
        for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            ASSERT_INT_SERIALIZATION(values[i], expected[i]);
        }
    }
    
    // 测试 uint64_t 边界值
    {
        uint64_t values[] = {0, 1, UINT64_MAX};
        const char* expected[] = {"0", "1", "18446744073709551615"};
        
        for (size_t i = 0; i < sizeof(values)/sizeof(values[0]); ++i) {
            ASSERT_INT_SERIALIZATION(values[i], expected[i]);
        }
    }
    
    // 测试各种类型的0值
    {
        int8_t i8 = 0;
        uint8_t u8 = 0;
        int16_t i16 = 0;
        uint16_t u16 = 0;
        int32_t i32 = 0;
        uint32_t u32 = 0;
        int64_t i64 = 0;
        uint64_t u64 = 0;
        
        ASSERT_INT_SERIALIZATION(i8, "0");
        ASSERT_INT_SERIALIZATION(u8, "0");
        ASSERT_INT_SERIALIZATION(i16, "0");
        ASSERT_INT_SERIALIZATION(u16, "0");
        ASSERT_INT_SERIALIZATION(i32, "0");
        ASSERT_INT_SERIALIZATION(u32, "0");
        ASSERT_INT_SERIALIZATION(i64, "0");
        ASSERT_INT_SERIALIZATION(u64, "0");
    }
    
    // 测试接近边界值的数字
    {
        int32_t near_max = INT32_MAX - 1;
        int32_t near_min = INT32_MIN + 1;
        
        ASSERT_INT_SERIALIZATION(near_max, "2147483646");
        ASSERT_INT_SERIALIZATION(near_min, "-2147483647");
    }
    
    // 测试大整数向量 (性能/正确性双重测试)
    // {
    //     std::vector<int> big_vec;
    //     const int count = 10000;
    //     big_vec.reserve(count);
        
    //     // 交替正负值
    //     for (int i = 0; i < count; i++) {
    //         big_vec.push_back((i % 2 == 0) ? i : -i);
    //     }
        
    //     buffer.clear();
    //     slow_json::dumps(buffer, big_vec);
        
    //     // 检查首尾是否正确
    //     std::string result = buffer.string();
    //     assert_with_message(
    //         !result.empty() && result[0] == '[' && result[result.size()-1] == ']',
    //         "向量序列化格式错误: 结果='%s'", 
    //         result.c_str()
    //     );
        
    //     // 检查几个关键位置的数字
    //     const std::string first_element = result.substr(1, 1);
    //     assert_with_message(
    //         first_element == "0",
    //         "向量第一个元素错误: 预期='0', 实际='%s', 完整结果='%s'",
    //         first_element.c_str(),
    //         result.c_str()
    //     );
        
    //     const bool found_neg_one = result.find(",-1,") != std::string::npos;
    //     assert_with_message(
    //         found_neg_one,
    //         "向量未找到 -1 元素, 完整结果='%s'",
    //         result.c_str()
    //     );
        
    //     const bool found_9998 = result.find(",9998,") != std::string::npos;
    //     assert_with_message(
    //         found_9998,
    //         "向量未找到 9998 元素, 完整结果='%s'",
    //         result.c_str()
    //     );
        
    //     const bool found_neg_9999 = result.find(",-9999,") != std::string::npos;
    //     assert_with_message(
    //         found_neg_9999,
    //         "向量未找到 -9999 元素, 完整结果='%s'",
    //         result.c_str()
    //     );
    // }
    
    // 测试混合类型
    {
        buffer.clear();
        std::vector<std::variant<int8_t, uint16_t, int32_t, uint64_t>> mixed = {
            static_cast<int8_t>(-10),
            static_cast<uint16_t>(65535),
            static_cast<int32_t>(123456),
            static_cast<uint64_t>(18446744073709551615ULL)
        };
        
        slow_json::dumps(buffer, mixed);
        const std::string expected = "[-10,65535,123456,18446744073709551615]";
        assert_with_message(
            buffer.string() == expected,
            "混合类型序列化错误\n预期: %s\n实际: %s",
            expected.c_str(),
            buffer.string().c_str()
        );
    }
    
    // 测试嵌套结构
    {
        buffer.clear();
        std::map<std::string, std::vector<int64_t>> nested = {
            {"min_values", {INT64_MIN, -1, 0}},
            {"max_values", {1, INT64_MAX}}
        };
        
        slow_json::dumps(buffer, nested);
        const std::string expected = 
            "{\"max_values\":[1,9223372036854775807],"
            "\"min_values\":[-9223372036854775808,-1,0]}";
            
        assert_with_message(
            buffer.string() == expected,
            "嵌套结构序列化错误\n预期: %s\n实际: %s",
            expected.c_str(),
            buffer.string().c_str()
        );
    }
    
    // 测试边界值向量
    {
        buffer.clear();
        std::vector<int32_t> bounds = {INT32_MIN, -1, 0, 1, INT32_MAX};
        slow_json::dumps(buffer, bounds);
        
        const std::string expected = 
            "[-2147483648,-1,0,1,2147483647]";
            
        assert_with_message(
            buffer.string() == expected,
            "边界值向量序列化错误\n预期: %s\n实际: %s",
            expected.c_str(),
            buffer.string().c_str()
        );
    }
    
    // 测试性能关键路径（大型连续内存块）
    // {
    //     const size_t large_size = 100000; // 10万个元素
    //     std::vector<uint32_t> large_vec(large_size, 123456789);
        
    //     buffer.clear();
    //     slow_json::dumps(buffer, large_vec);
        
    //     // 验证基本结构
    //     assert_with_message(
    //         buffer.string().size() > 800000, // 预期大小约 10万*8字节 = 800KB
    //         "大型向量序列化大小异常: 预期>800000字节, 实际=%zu字节",
    //         buffer.string().size()
    //     );
        
    //     // 验证开头和结尾
    //     assert_with_message(
    //         buffer.string().substr(0, 10) == "[123456789",
    //         "大型向量开头异常: %s",
    //         buffer.string().substr(0, 20).c_str()
    //     );
        
    //     assert_with_message(
    //         buffer.string().substr(buffer.string().size() - 10) == "123456789]",
    //         "大型向量结尾异常: %s",
    //         buffer.string().substr(buffer.string().size() - 20).c_str()
    //     );
    // }
    
    #undef ASSERT_INT_SERIALIZATION
}