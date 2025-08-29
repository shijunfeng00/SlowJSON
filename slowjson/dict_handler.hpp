
// Created by hyzh on 2025/8/25

#ifndef SLOWJSON_DICT_HANDLER_HPP
#define SLOWJSON_DICT_HANDLER_HPP
#include <stack>
#include "reader.h"
#include "error/en.h"
#include "dict.hpp"

namespace slow_json::details {
    /**
     * @brief 用于RapidJSON SAX解析的自定义Handler，构建slow_json::dict结构
     * @details 通过处理RapidJSON的SAX事件（如StartObject、Key、String等），直接构造slow_json::dict或serializable_wrapper。
     *          支持根对象为对象、数组或基本类型，使用栈管理嵌套结构，确保移动语义以减少拷贝。
     */
    struct DictHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, DictHandler> {
        /**
         * @brief 默认构造函数，初始化根对象指针为空
         */
        DictHandler() : root_(nullptr) {}

        /**
         * @brief 处理对象开始事件
         * @return bool 始终返回true，表示继续解析
         * @details 如果是根对象，创建新的dict；否则创建嵌套dict并推入对象栈，同时将键推入值键栈。
         */
        bool StartObject() {
            if (!root_) {
                root_ = new dict{};
                object_stack_.push(root_);
            } else {
                value_key_stack_.push(current_key_);
                auto object = new dict{};
                object_stack_.push(object);
            }
            return true;
        }

        /**
         * @brief 处理对象结束事件
         * @param memberCount 对象中的键值对数量
         * @return bool 始终返回true，表示继续解析
         * @details 弹出当前对象栈顶，如果非根对象，从值键栈弹出键并包装为serializable_wrapper插入父级。
         */
        bool EndObject(rapidjson::SizeType memberCount) {
            dict* current = object_stack_.top();
            object_stack_.pop();
            if (!object_stack_.empty()) {
                std::string key = value_key_stack_.top();
                value_key_stack_.pop();
                serializable_wrapper wrapper{std::move(*current)};
                delete current;  // 清理临时dict
                InsertValue(std::move(key), std::move(wrapper));
            }
            return true;
        }

        /**
         * @brief 处理数组开始事件
         * @return bool 始终返回true，表示继续解析
         * @details 如果是根对象，创建vector并存储到dict；否则创建临时vector推入数组栈，同时将键推入值键栈。
         */
        bool StartArray() {
            if (!root_) {
                root_ = new dict{std::vector<serializable_wrapper>{}};
                array_stack_.push((std::vector<serializable_wrapper>*)root_->_data_ptr->value());
            } else {
                value_key_stack_.push(current_key_);
                auto* arr = new std::vector<serializable_wrapper>{};
                array_stack_.push(arr);
            }
            return true;
        }

        /**
         * @brief 处理数组结束事件
         * @param elementCount 数组中的元素数量
         * @return bool 始终返回true，表示继续解析
         * @details 弹出当前数组栈顶，如果非根数组，从值键栈弹出键并包装为serializable_wrapper插入父级，并清理临时数组。
         */
        bool EndArray(rapidjson::SizeType elementCount) {
            auto* arr = array_stack_.top();
            array_stack_.pop();
            serializable_wrapper wrapper{std::move(*arr)};
            if (!array_stack_.empty() || !object_stack_.empty()) {
                std::string key = value_key_stack_.top();
                value_key_stack_.pop();
                InsertValue(std::move(key), std::move(wrapper));
                delete arr;  // 仅清理非根数组
            }
            return true;
        }

        /**
         * @brief 处理键事件
         * @param str 键字符串
         * @param length 键字符串长度
         * @param copy 是否复制字符串（忽略，总是复制）
         * @return bool 如果在对象上下文中返回true，否则false
         * @throws std::runtime_error 如果键出现在无效上下文
         */
        bool Key(const char* str, rapidjson::SizeType length, bool copy) {
            if (object_stack_.empty()) {
                throw std::runtime_error("键出现在无效上下文中");
            }
            current_key_ = std::string(str, length);
            return true;
        }

        /**
         * @brief 处理字符串值
         * @param str 字符串内容
         * @param length 字符串长度
         * @param copy 是否复制字符串（忽略，总是复制）
         * @return bool 始终返回true
         */
        bool String(const char* str, rapidjson::SizeType length, bool copy) {
            serializable_wrapper wrapper{std::string(str, length)};
            wrapper.set_base_type(serializable_wrapper::STRING_TYPE);
            std::string key = current_key_;
            current_key_ = "";
            InsertValue(std::move(key), std::move(wrapper));
            return true;
        }

        /**
         * @brief 处理整数值（32位有符号）
         * @param i 整数值
         * @return bool 始终返回true
         */
        bool Int(int i) {
            serializable_wrapper wrapper{static_cast<int64_t>(i)};
            wrapper.set_base_type(serializable_wrapper::INT64_TYPE);
            std::string key = current_key_;
            current_key_ = "";
            InsertValue(std::move(key), std::move(wrapper));
            return true;
        }

        /**
         * @brief 处理无符号整数值（32位）
         * @param u 无符号整数值
         * @return bool 始终返回true
         */
        bool Uint(unsigned u) {
            serializable_wrapper wrapper{static_cast<uint64_t>(u)};
            wrapper.set_base_type(serializable_wrapper::UINT64_TYPE);
            std::string key = current_key_;
            current_key_ = "";
            InsertValue(std::move(key), std::move(wrapper));
            return true;
        }

        /**
         * @brief 处理64位有符号整数值
         * @param i 64位整数值
         * @return bool 始终返回true
         */
        bool Int64(int64_t i) {
            serializable_wrapper wrapper{i};
            wrapper.set_base_type(serializable_wrapper::INT64_TYPE);
            std::string key = current_key_;
            current_key_ = "";
            InsertValue(std::move(key), std::move(wrapper));
            return true;
        }

        /**
         * @brief 处理64位无符号整数值
         * @param u 64位无符号整数值
         * @return bool 始终返回true
         */
        bool Uint64(uint64_t u) {
            serializable_wrapper wrapper{u};
            wrapper.set_base_type(serializable_wrapper::UINT64_TYPE);
            std::string key = current_key_;
            current_key_ = "";
            InsertValue(std::move(key), std::move(wrapper));
            return true;
        }

        /**
         * @brief 处理双精度浮点值
         * @param d 双精度浮点值
         * @return bool 始终返回true
         */
        bool Double(double d) {
            serializable_wrapper wrapper{d};
            wrapper.set_base_type(serializable_wrapper::DOUBLE_TYPE);
            std::string key = current_key_;
            current_key_ = "";
            InsertValue(std::move(key), std::move(wrapper));
            return true;
        }

        /**
         * @brief 处理布尔值
         * @param b 布尔值
         * @return bool 始终返回true
         */
        bool Bool(bool b) {
            serializable_wrapper wrapper{b};
            wrapper.set_base_type(serializable_wrapper::BOOL_TYPE);
            std::string key = current_key_;
            current_key_ = "";
            InsertValue(std::move(key), std::move(wrapper));
            return true;
        }

        /**
         * @brief 处理空值
         * @return bool 始终返回true
         */
        bool Null() {
            serializable_wrapper wrapper{nullptr};
            wrapper.set_base_type(serializable_wrapper::NULL_TYPE);
            std::string key = current_key_;
            current_key_ = "";
            InsertValue(std::move(key), std::move(wrapper));
            return true;
        }

        /**
         * @brief 将值插入当前上下文（对象或数组）
         * @param key 要使用的键（对于数组忽略）
         * @param value 要插入的serializable_wrapper（移动构造）
         * @throws std::runtime_error 如果上下文无效
         * @details 根据当前栈状态，将值插入到对象（使用键创建pair）或数组（忽略键），支持根对象为基本类型或数组。
         */
        void InsertValue(std::string key, serializable_wrapper&& value) {
            if (!array_stack_.empty()) {
                // 插入到当前数组（忽略键）
                array_stack_.top()->emplace_back(std::move(value));
            } else if (!object_stack_.empty()) {
                // 插入到当前对象作为pair
                dict* current = object_stack_.top();
                char* key_copy = new char[key.size() + 1];
                std::strcpy(key_copy, key.c_str());
                current->_data.emplace_back(key_copy, std::move(value));
                current->set_copied(true);
            } else if (root_ && root_->value_type() == serializable_wrapper::LIST_TYPE) {
                // 根为数组，更新_data_ptr
                *root_->_data_ptr = std::move(value);
            } else if (!root_) {
                // 根为基本类型
                root_ = new dict{std::move(value)};
            } else {
                throw std::runtime_error("值出现在无效上下文中");
            }
        }

        /**
         * @brief 获取解析后的根dict
         * @return dict 解析完成的dict对象（通过移动构造返回）
         * @throws std::runtime_error 如果未解析到根对象
         */
        dict GetRoot() {
            if (root_ == nullptr) {
                throw std::runtime_error("未解析到任何根对象");
            }
            dict result = std::move(*root_);
            delete root_; // 清理堆分配
            root_ = nullptr;
            return result;
        }

        /**
         * @brief 解析JSON字符串到slow_json::dict
         * @param json_str 输入的JSON字符串
         * @return dict 解析后的dict对象
         * @throws std::runtime_error 如果JSON格式无效或有未解析的剩余内容
         * @details 使用RapidJSON的SAX解析器（支持SIMD加速）直接构造dict，支持根对象为对象、数组或基本类型。
         *          确保整个输入被解析，检测多余字符。
         */
        static dict parse_json_to_dict(std::string_view json_str) {
            rapidjson::StringStream ss(json_str.data());
            rapidjson::GenericReader<rapidjson::UTF8<>, rapidjson::UTF8<>> reader;
            DictHandler handler;

            rapidjson::ParseResult result = reader.Parse<
                    rapidjson::kParseNanAndInfFlag | rapidjson::kParseStopWhenDoneFlag
            >(ss, handler);

            if (!result) {
                std::string error_msg = std::string("JSON解析错误：") +
                                        rapidjson::GetParseError_En(result.Code()) +
                                        " 在偏移量 " + std::to_string(result.Offset());
                throw std::runtime_error(error_msg);
            }

            // 检查是否所有输入都被解析
            if (ss.Tell() != json_str.size()) {
                throw std::runtime_error("JSON解析不完整，存在未解析的字符");
            }

            return handler.GetRoot();
        }

    private:
        dict* root_; ///< 根dict指针
        std::stack<dict*> object_stack_; ///< 对象栈，管理嵌套字典
        std::stack<std::vector<serializable_wrapper>*> array_stack_; ///< 数组栈，管理嵌套数组
        std::stack<std::string> value_key_stack_; ///< 值键栈，管理嵌套对象的键以防覆盖
        std::string current_key_; ///< 当前解析的键
    };
}
#endif //SLOWJSON_DICT_HANDLER_HPP
