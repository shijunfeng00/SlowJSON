//
// Created by hy-20 on 2024/8/2.
//
#include "slowjson.hpp"
#include "iostream"

using namespace slow_json::static_string_literals;

enum Color {
    RED,
    GREEN,
    BLUE,
    BLACK
};

struct ObjectWithEnum {
    Color color;

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"color"_ss, &ObjectWithEnum::color}
        };
    }
};

void test_enum_serialization_deserialization() {

    printf("run %s\n", __PRETTY_FUNCTION__);

    Color p = slow_json::string2enum<Color>("BLACK");

    assert_with_message(std::string_view{slow_json::type_name_v<decltype(p)>.str} == "Color",
                        "slow_json::string2enum返回值类型名称不对");
    assert_with_message(p == BLACK, "slow_json::string2enum反序列化的到的结果不对");


    Color color = RED;
    assert_with_message(slow_json::enum2string(color) == "RED", "slow_json::enum2string返回的结果不对");

    slow_json::Buffer buffer;
    slow_json::dumps(buffer, RED, 4);
    assert_with_message(buffer.string() == "\"RED\"", "slow_json::dumps返回的结果不对");

    Color color2;
    slow_json::loads(color2, "\"BLUE\""); //注意，这里是个字符串
    assert_with_message(color2 == BLUE, "slow_json::loads返回的结果不正确");

    ObjectWithEnum object{.color=GREEN};
    buffer.clear();
    slow_json::dumps(buffer, object);
    assert_with_message(buffer.string() == R"({"color":"GREEN"})", "slow_json::dumps返回的结果不对");
    ObjectWithEnum object2;
    slow_json::loads(object2, buffer.string());
    assert_with_message(object2.color == GREEN, "slow_json::loads返回的结果不对");
}
