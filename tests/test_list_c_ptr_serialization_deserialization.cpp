#include <iostream>
#include "slowjson.hpp"
#include <vector>

using namespace slow_json::static_string_literals;

struct NodeUsingCPtr{
    NodeUsingCPtr(int value=0):value(value){}
    NodeUsingCPtr*next=nullptr;
    int value=0;
    $config(NodeUsingCPtr,next,value);
};
struct ListUsingCPtr{
    NodeUsingCPtr*begin{new NodeUsingCPtr(0)};
    void push_back(int value){
        auto back=this->begin;
        while(back->next!=nullptr){
            back=back->next;
        };
        back->next=new NodeUsingCPtr(value);
    }
    $config(ListUsingCPtr,begin);
};

void test_list_c_ptr_serialization_deserialization() {
    printf("run %s\n", __PRETTY_FUNCTION__);

    assert_with_message(slow_json::concepts::supported<int*> == 1, "相关concepts结果不正确");
    assert_with_message(slow_json::concepts::load_supported<int*> == 1, "相关concepts结果不正确");
    assert_with_message(slow_json::concepts::dump_supported<int*> == 1, "相关concepts结果不正确");

    ListUsingCPtr ls;
    ls.push_back(2);
    ls.push_back(3);
    ls.push_back(4);

    slow_json::Buffer buffer{10000};
    slow_json::dumps(buffer,ls);
    assert_with_message(buffer.string()==R"({"begin":{"next":{"next":{"next":{"next":null,"value":4},"value":3},"value":2},"value":0}})","通过slow_json::dumps序列化结果不正确");

    ListUsingCPtr ls2;
    slow_json::loads(ls2,R"({"begin":{"next":{"next":{"next":{"next":null,"value":44},"value":-33},"value":22},"value":-10}})");

    assert_with_message(ls2.begin->value==-10,"通过slow_json::loads反序列化结果不正确:%d",ls2.begin->value);
    assert_with_message(ls2.begin->next->value==22,"通过slow_json::loads反序列化结果不正确");
    assert_with_message(ls2.begin->next->next->value==-33,"通过slow_json::loads反序列化结果不正确");
    assert_with_message(ls2.begin->next->next->next->value==44,"通过slow_json::loads反序列化结果不正确");
    assert_with_message(ls2.begin->next->next->next->next==nullptr,"通过slow_json::loads反序列化结果不正确");
}
