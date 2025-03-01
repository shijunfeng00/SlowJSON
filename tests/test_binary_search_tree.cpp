//
// Created by hyzh on 2025/3/1.
//
#include <iostream>
#include "slowjson.hpp"
#include <vector>

using namespace slow_json::static_string_literals;

class BinarySearchTree;
class BTNode{
    friend class BinarySearchTree;
private:
    BTNode*left;
    BTNode*right;
    int value=0;
public:
    BTNode():left(nullptr),right(nullptr){}
    $config(BTNode,left,right,value);
};

class BinarySearchTree{
private:
    BTNode*root;
    void inorder_traversal(BTNode*node,std::vector<int>&result){
        if(node->left) {
            inorder_traversal(node->left,result);
        }
        result.emplace_back(node->value);
        if(node->right) {
            inorder_traversal(node->right,result);
        }
    }
public:
    BinarySearchTree():root(nullptr){}
    void insert(int value){
        auto node=root;
        if(root==nullptr){
            root=new BTNode();
            root->value=value;
            return;
        }
        while(true) {
            if (value < node->value) {
                if(node->left) {
                    node = node->left;
                }else{
                    node->left=new BTNode();
                    node->left->value=value;
                    break;
                }
            } else if (value > node->value) {
                if(node->right) {
                    node = node->right;
                }else{
                    node->right=new BTNode();
                    node->right->value=value;
                    break;
                }
            } else {
                break;
            }
        }
    }
    std::vector<int>inorder_traversal(){
        std::vector<int>result;
        inorder_traversal(root,result);
        return result;
    }

    $config(BinarySearchTree,root);
};

void test_binary_search_tree(){
    printf("run %s\n", __PRETTY_FUNCTION__);

    BinarySearchTree tree;
    tree.insert(65);
    tree.insert(12);
    tree.insert(94);
    tree.insert(32);
    tree.insert(43);
    tree.insert(123);
    auto result=tree.inorder_traversal();
    slow_json::Buffer buffer;
    slow_json::dumps(buffer,tree);
    assert_with_message(buffer.string()==R"({"root":{"left":{"left":null,"right":{"left":null,"right":{"left":null,"right":null,"value":43},"value":32},"value":12},"right":{"left":null,"right":{"left":null,"right":null,"value":123},"value":94},"value":65}})",
                        "slow_json::dumps序列化结果不正确");
    BinarySearchTree tree2;
    slow_json::loads(tree2,R"({"root":{"left":{"left":null,"right":{"left":null,"right":{"left":null,"right":null,"value":43},"value":32},"value":12},"right":{"left":null,"right":{"left":null,"right":null,"value":123},"value":94},"value":65}})");

    assert_with_message(tree2.inorder_traversal()==tree.inorder_traversal(),"slow_json::loads反序列化结果不正确");
}