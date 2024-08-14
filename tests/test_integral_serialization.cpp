//
// Created by hy-20 on 2024/8/14.
//
#include "slowjson.hpp"
#include <iostream>

using namespace slow_json::static_string_literals;

void test_integral_serialization() {
    struct Integral {
        char a = 'p';
        int b = 123;
        unsigned int c = 3147483647ul;
        unsigned int d = 3147647ul;
        int64_t e = 214748364700ll;
        uint64_t f = 18446744073709551615ull;
        uint64_t g = 8446744073709551615ull;

        static constexpr auto get_config() noexcept {
            return slow_json::static_dict{
                    std::pair{"a"_ss, &Integral::a},
                    std::pair{"b"_ss, &Integral::b},
                    std::pair{"c"_ss, &Integral::c},
                    std::pair{"d"_ss, &Integral::d},
                    std::pair{"e"_ss, &Integral::e},
                    std::pair{"f"_ss, &Integral::f},
                    std::pair{"g"_ss, &Integral::g}
            };
        }
    } object;
    slow_json::Buffer buffer{1000};
    slow_json::dumps(buffer, object);
    assert_with_message(buffer.string() ==
                        "{\"a\":p,\"b\":123,\"c\":3147483647,\"d\":3147647,\"e\":214748364700,\"f\":18446744073709551615,\"g\":8446744073709551615}",
                        "通过slow_json::dumps序列化整数结果错误");
}