//
// Created by hy-20 on 2024/7/19.
//

#ifndef SLOWJSON_INDENT_HPP
#define SLOWJSON_INDENT_HPP

#include <string>
#include <stack>
#include <stdexcept>
#include "buffer.hpp"

namespace slow_json {
    /**
      * 格式化JSON字符串，添加首行缩进
      * @param json 存储JSON的缓冲区数据
      * @param indent 缩进长度
      */
    static void indent(slow_json::Buffer &json, std::size_t indent = 4) {
        std::size_t init_size = json.size();
        std::stack<char> brackets; // 存储左括号
        bool in_string = false; // 记录是否在字符串中
        char match; // 存储匹配的右括号
        for (int idx = 0; idx < init_size; idx++) { // 遍历JSON字符串中的每个字符
            char &c = json[idx];
            if (c == '{' || c == '[') { // 如果是左括号
                brackets.push(c); // 入栈
                json += c; // 原样输出
                json += '\n'; // 换行
                for (int i = 0; i < (int) brackets.size() * indent; i++) { // 输出缩进空格
                    json += ' ';
                }
            } else if (c == '}' || c == ']') { // 如果是右括号
                if (brackets.empty()) { // 如果栈为空，说明没有匹配的左括号
                    throw std::runtime_error("Invalid JSON format: unmatched right bracket"); // 抛出异常
                }
                match = brackets.top(); // 获取栈顶的左括号
                if ((match == '{' && c != '}') || (match == '[' && c != ']')) { // 如果不匹配
                    throw std::runtime_error("Invalid JSON format: mismatched brackets"); // 抛出异常
                }
                brackets.pop(); // 出栈
                json += '\n'; // 换行
                for (int i = 0; i < (int) brackets.size() * indent; i++) { // 输出缩进空格
                    json += ' ';
                }
                json += c; // 原样输出
            } else if (c == ',') { // 如果是逗号
                json += c; // 原样输出
                json += '\n'; // 换行
                for (int i = 0; i < (int) brackets.size() * indent; i++) { // 输出缩进空格
                    json += ' ';
                }
            } else if (c == '"') { // 如果是双引号
                in_string = !in_string; // 切换字符串状态
                json += c; // 原样输出
            } else if (c == ' ' || c == '\n' || c == '\t') { // 如果是空白字符
                if (in_string) { // 如果在字符串中
                    json += c; // 原样输出
                }
            } else { // 其他字符
                json += c; // 原样输出
            }
        }
        if (!brackets.empty()) { // 如果遍历完后栈不为空，说明有多余的左括号
            throw std::runtime_error("Invalid JSON format: unmatched left bracket"); // 抛出异常
        }
        json.erase(init_size);
    }

    /**
      * 格式化JSON字符串，添加首行缩进
      * @param json JSON原始数据
      * @param indent 缩进长度
      * @return 格式化之后的JSON字符串
      */
    static std::string indent(const std::string &json, int indent = 4) {
        std::string result; // 存储格式化后的结果
        std::stack<char> brackets; // 存储左括号
        bool in_string = false; // 记录是否在字符串中
        char match; // 存储匹配的右括号
        for (char c: json) { // 遍历JSON字符串中的每个字符
            if (c == '{' || c == '[') { // 如果是左括号
                brackets.push(c); // 入栈
                result += c; // 原样输出
                result += '\n'; // 换行
                for (int i = 0; i < (int) brackets.size() * indent; i++) { // 输出缩进空格
                    result += ' ';
                }
            } else if (c == '}' || c == ']') { // 如果是右括号
                if (brackets.empty()) { // 如果栈为空，说明没有匹配的左括号
                    throw std::runtime_error("Invalid JSON format: unmatched right bracket:" + json); // 抛出异常
                }
                match = brackets.top(); // 获取栈顶的左括号
                if ((match == '{' && c != '}') || (match == '[' && c != ']')) { // 如果不匹配
                    throw std::runtime_error("Invalid JSON format: mismatched brackets:" + json); // 抛出异常
                }
                brackets.pop(); // 出栈
                result += '\n'; // 换行
                for (int i = 0; i < (int) brackets.size() * indent; i++) { // 输出缩进空格
                    result += ' ';
                }
                result += c; // 原样输出
            } else if (c == ',') { // 如果是逗号
                result += c; // 原样输出
                result += '\n'; // 换行
                for (int i = 0; i < (int) brackets.size() * indent; i++) { // 输出缩进空格
                    result += ' ';
                }
            } else if (c == '"') { // 如果是双引号
                in_string = !in_string; // 切换字符串状态
                result += c; // 原样输出
            } else if (c == ' ' || c == '\n' || c == '\t') { // 如果是空白字符
                if (in_string) { // 如果在字符串中
                    result += c; // 原样输出
                }
            } else { // 其他字符
                result += c; // 原样输出
            }
        }
        if (!brackets.empty()) { // 如果遍历完后栈不为空，说明有多余的左括号
            throw std::runtime_error("Invalid JSON format: unmatched left bracket:" + json); // 抛出异常
        }
        return result; // 返回结果
    }
}
#endif //SLOWJSON_INDENT_HPP
