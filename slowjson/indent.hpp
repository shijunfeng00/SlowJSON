#ifndef SLOWJSON_INDENT_HPP
#define SLOWJSON_INDENT_HPP

#include <string>
#include <stack>
#include <stdexcept>
#include "buffer.hpp"

namespace slow_json {
    /**
     * 格式化JSON字符串，添加首行缩进（操作Buffer）
     * @param json 存储JSON的缓冲区数据
     * @param indent 缩进长度
     */
    static void indent(slow_json::Buffer &json, std::size_t indent = 4) {
        std::size_t init_size = json.size();
        std::stack<char> brackets; // 存储左括号
        bool in_string = false; // 记录是否在字符串中
        char match; // 存储匹配的右括号
        for (std::size_t idx = 0; idx < init_size; idx++) { // 遍历JSON字符串中的每个字符
            char c = json[idx];
            if (c == '"' && (idx == 0 || json[idx - 1] != '\\')) { // 处理双引号，排除转义情况
                in_string = !in_string; // 切换字符串状态
                json += c; // 原样输出
            } else if (!in_string) { // 仅在非字符串状态下处理结构字符
                if (c == '{' || c == '[') { // 如果是左括号
                    brackets.push(c); // 入栈
                    json += c; // 原样输出
                    json += '\n'; // 换行
                    for (std::size_t i = 0; i < brackets.size() * indent; i++) { // 输出缩进空格
                        json += ' ';
                    }
                } else if (c == '}' || c == ']') { // 如果是右括号
                    if (brackets.empty()) { // 如果栈为空，说明没有匹配的左括号
                        throw std::runtime_error(
                                "无效的JSON格式：位置 " + std::to_string(idx) + " 处存在未匹配的右括号");
                    }
                    match = brackets.top(); // 获取栈顶的左括号
                    if ((match == '{' && c != '}') || (match == '[' && c != ']')) { // 如果不匹配
                        throw std::runtime_error("无效的JSON格式：位置 " + std::to_string(idx) + " 处的括号不匹配");
                    }
                    brackets.pop(); // 出栈
                    json += '\n'; // 换行
                    for (std::size_t i = 0; i < brackets.size() * indent; i++) { // 输出缩进空格
                        json += ' ';
                    }
                    json += c; // 原样输出
                } else if (c == ',') { // 如果是逗号
                    json += c; // 原样输出
                    json += '\n'; // 换行
                    for (std::size_t i = 0; i < brackets.size() * indent; i++) { // 输出缩进空格
                        json += ' ';
                    }
                } else if (c == ' ' || c == '\n' || c == '\t') { // 如果是空白字符
                    // 非字符串中的空白字符被忽略
                    continue;
                } else { // 其他字符
                    json += c; // 原样输出
                }
            } else { // 在字符串中
                json += c; // 原样输出
            }
        }
        if (!brackets.empty()) { // 如果遍历完后栈不为空，说明有多余的左括号
            throw std::runtime_error("无效的JSON格式：存在未匹配的左括号");
        }
        json.erase(init_size);
    }

    /**
     * 格式化JSON字符串，添加首行缩进（返回std::string）
     * @param json JSON原始数据
     * @param indent 缩进长度
     * @return 格式化之后的JSON字符串
     */
    static std::string indent(const std::string &json, int indent = 4) {
        std::string result; // 存储格式化后的结果
        std::stack<char> brackets; // 存储左括号
        bool in_string = false; // 记录是否在字符串中
        char match; // 存储匹配的右括号
        for (std::size_t idx = 0; idx < json.size(); idx++) { // 遍历JSON字符串中的每个字符
            char c = json[idx];
            if (c == '"' && (idx == 0 || json[idx - 1] != '\\')) { // 处理双引号，排除转义情况
                in_string = !in_string; // 切换字符串状态
                result += c; // 原样输出
            } else if (!in_string) { // 仅在非字符串状态下处理结构字符
                if (c == '{' || c == '[') { // 如果是左括号
                    brackets.push(c); // 入栈
                    result += c; // 原样输出
                    result += '\n'; // 换行
                    for (std::size_t i = 0; i < brackets.size() * indent; i++) { // 输出缩进空格
                        result += ' ';
                    }
                } else if (c == '}' || c == ']') { // 如果是右括号
                    if (brackets.empty()) { // 如果栈为空，说明没有匹配的左括号
                        throw std::runtime_error(
                                "无效的JSON格式：位置 " + std::to_string(idx) + " 处存在未匹配的右括号");
                    }
                    match = brackets.top(); // 获取栈顶的左括号
                    if ((match == '{' && c != '}') || (match == '[' && c != ']')) { // 如果不匹配
                        throw std::runtime_error("无效的JSON格式：位置 " + std::to_string(idx) + " 处的括号不匹配");
                    }
                    brackets.pop(); // 出栈
                    result += '\n'; // 换行
                    for (std::size_t i = 0; i < brackets.size() * indent; i++) { // 输出缩进空格
                        result += ' ';
                    }
                    result += c; // 原样输出
                } else if (c == ',') { // 如果是逗号
                    result += c; // 原样输出
                    result += '\n'; // 换行
                    for (std::size_t i = 0; i < brackets.size() * indent; i++) { // 输出缩进空格
                        result += ' ';
                    }
                } else if (c == ' ' || c == '\n' || c == '\t') { // 如果是空白字符
                    // 非字符串中的空白字符被忽略
                    continue;
                } else { // 其他字符
                    result += c; // 原样输出
                }
            } else { // 在字符串中
                result += c; // 原样输出
            }
        }
        if (!brackets.empty()) { // 如果遍历完后栈不为空，说明有多余的左括号
            throw std::runtime_error("无效的JSON格式：存在未匹配的左括号");
        }
        return result; // 返回结果
    }
}
#endif //SLOWJSON_INDENT_HPP