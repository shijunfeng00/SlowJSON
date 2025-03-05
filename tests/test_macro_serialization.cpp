//
// Created by hyzh on 2025/3/5.
//
#include "slowjson.hpp"
#include <iostream>
#include <deque>
#include <fstream>

using namespace slow_json::static_string_literals;

struct TestMacro {
    float value;

    static constexpr auto get_config() noexcept {
        return slow_json::static_dict{
                std::pair{"value"_ss, &TestMacro::value}
        };
    }
};

struct NodeMacro {
    int xxx;
    float yyy;
    std::string zzz;
    TestMacro test;
    std::deque<std::string> dq;
    std::pair<int,int>pr;
    std::tuple<int,float>tp;
    $config(NodeMacro,xxx,yyy,zzz,test,dq,pr,tp);
};

struct NodeMacroDrived : public NodeMacro {
    long long hahaha;
    $$config(<NodeMacro>,NodeMacroDrived,hahaha);
};


void test_macro_serialization(){
    printf("run %s\n", __PRETTY_FUNCTION__);
    NodeMacroDrived p;
    p.xxx=1;
    p.yyy=1.2345;
    p.zzz="shijunfeng";
    p.test.value=123.456;
    p.dq={"a","b","c","d"};
    p.hahaha=1234233;
    p.pr={2,3};
    p.tp={1,2};
    slow_json::Buffer buffer{1000};
    slow_json::dumps(buffer, p);
    assert_with_message(
            buffer.string() == R"({"xxx":1,"yyy":1.2345,"zzz":"shijunfeng","test":{"value":123.456},"dq":["a","b","c","d"],"pr":[2,3],"tp":[1,2.0],"hahaha":1234233})",
            "通过slow_json::dumps序列化得到的结果不正确");
}