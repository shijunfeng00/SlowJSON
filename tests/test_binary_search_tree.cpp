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
class BinarySearchTree {
private:
    BTNode* root;

    // 递归中序遍历
    void inorder_traversal(BTNode* node, std::vector<int>& result) {
        if (!node) return;
        inorder_traversal(node->left, result);
        result.emplace_back(node->value);
        inorder_traversal(node->right, result);
    }

    // 递归销毁整棵子树
    void destroy(BTNode* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    BinarySearchTree(): root(nullptr) {}

    // 析构时销毁整棵树
    ~BinarySearchTree() {
        destroy(root);
    }

    void insert(int v) {
        if (!root) {
            root = new BTNode();
            root->value = v;
            return;
        }
        BTNode* cur = root;
        while (true) {
            if (v < cur->value) {
                if (cur->left) {
                    cur = cur->left;
                } else {
                    cur->left = new BTNode();
                    cur->left->value = v;
                    break;
                }
            } else if (v > cur->value) {
                if (cur->right) {
                    cur = cur->right;
                } else {
                    cur->right = new BTNode();
                    cur->right->value = v;
                    break;
                }
            } else {
                // 相等则不插入
                break;
            }
        }
    }

    std::vector<int> inorder_traversal() {
        std::vector<int> result;
        inorder_traversal(root, result);
        return result;
    }

    $config(BinarySearchTree, root);
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