// Created by hyzh on 2025/8/25

#ifndef SLOWJSON_DICT_HANDLER_HPP
#define SLOWJSON_DICT_HANDLER_HPP
#include <vector>
#include <cstring>
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
    DictHandler() : root_(nullptr) {
        object_stack_.reserve(10);
        value_key_stack_.reserve(10);
        array_stack_.reserve(10);
    }

    /**
     * @brief 处理对象开始事件
     * @return bool 始终返回true，表示继续解析
     * @details 如果是根对象，创建新的dict；否则创建嵌套dict并推入对象栈，同时将键推入值键栈。
     */
    bool StartObject() {

        if (!root_) {
            root_ = new dict{};
            object_stack_.push_back(root_);
        } else {
            value_key_stack_.push_back(current_key_);
            current_key_={};
            auto object = new dict{};
            object_stack_.push_back(object);
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
        dict* current = object_stack_.back();
        object_stack_.pop_back();

        if (!object_stack_.empty() || !array_stack_.empty()) {
            std::string_view key;
            if (!value_key_stack_.empty()) {
                key = std::move(value_key_stack_.back());
                value_key_stack_.pop_back();
            }
            serializable_wrapper wrapper{std::move(*current)};
            delete current;
            InsertValue(key, std::move(wrapper));
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
            auto array_ptr=(std::vector<serializable_wrapper>*)root_->_data_ptr->value();
            array_ptr->reserve(10);
            array_stack_.push_back(array_ptr);
        } else {
            value_key_stack_.push_back(current_key_);
            current_key_={};
            auto* arr = new std::vector<serializable_wrapper>{};
            arr->reserve(10);
            array_stack_.push_back(arr);
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
        auto* arr = array_stack_.back();
        array_stack_.pop_back();

        if (object_stack_.empty() && array_stack_.empty()) {
            return true;
        }

        serializable_wrapper wrapper{std::move(*arr)};
        std::string_view key;
        if (!value_key_stack_.empty()) {
            key = std::move(value_key_stack_.back());
            value_key_stack_.pop_back();
        }
        InsertValue(key, std::move(wrapper));
        delete arr;
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
        dict*current_dict=object_stack_.back();
        const char*key_cp=current_dict->allocate_and_copy_key(str,length);
        current_key_ = std::string_view(key_cp, length);
        return true;
    }

    /** @brief 处理字符串值 */
    bool String(const char* str, rapidjson::SizeType length, bool copy) {
        serializable_wrapper wrapper{std::string(str, length)};
        wrapper.set_base_type(serializable_wrapper::STRING_TYPE);
        std::string_view key=current_key_;
        current_key_={};
        InsertValue(key, std::move(wrapper));
        return true;
    }

    /** @brief 处理整数值 */
    bool Int(int i) {
        serializable_wrapper wrapper{static_cast<int64_t>(i)};
        wrapper.set_base_type(serializable_wrapper::INT64_TYPE);
        std::string_view key=current_key_;
        current_key_={};
        InsertValue(key, std::move(wrapper));
        return true;
    }

    /** @brief 处理无符号整数 */
    bool Uint(unsigned u) {
        serializable_wrapper wrapper{static_cast<uint64_t>(u)};
        wrapper.set_base_type(serializable_wrapper::UINT64_TYPE);
        std::string_view key=current_key_;
        current_key_={};
        InsertValue(key, std::move(wrapper));
        return true;
    }

    /** @brief 处理64位整数 */
    bool Int64(int64_t i) {
        serializable_wrapper wrapper{i};
        wrapper.set_base_type(serializable_wrapper::INT64_TYPE);
        std::string_view key=current_key_;
        current_key_={};
        InsertValue(key, std::move(wrapper));
        return true;
    }

    /** @brief 处理64位无符号整数 */
    bool Uint64(uint64_t u) {
        serializable_wrapper wrapper{u};
        wrapper.set_base_type(serializable_wrapper::UINT64_TYPE);
        std::string_view key=current_key_;
        current_key_={};
        InsertValue(key, std::move(wrapper));
        return true;
    }

    /** @brief 处理双精度浮点 */
    bool Double(double d) {
        serializable_wrapper wrapper{d};
        wrapper.set_base_type(serializable_wrapper::DOUBLE_TYPE);
        std::string_view key=current_key_;
        current_key_={};
        InsertValue(key, std::move(wrapper));
        return true;
    }

    /** @brief 处理布尔值 */
    bool Bool(bool b) {
        serializable_wrapper wrapper{b};
        wrapper.set_base_type(serializable_wrapper::BOOL_TYPE);
        std::string_view key=current_key_;
        current_key_={};
        InsertValue(key, std::move(wrapper));
        return true;
    }

    /** @brief 处理空值 */
    bool Null() {
        serializable_wrapper wrapper{nullptr};
        wrapper.set_base_type(serializable_wrapper::NULL_TYPE);
        std::string_view key=current_key_;
        current_key_={};
        InsertValue(key, std::move(wrapper));
        return true;
    }

    /**
     * @brief 将值插入当前上下文（对象或数组）
     * @param key 要使用的键（对于数组忽略）
     * @param value 要插入的serializable_wrapper（移动构造）
     */
    void InsertValue(const std::string_view& key, serializable_wrapper&& value) {
        if (!object_stack_.empty() && !key.empty()) {
            dict* current = object_stack_.back();
            current->_data.emplace_back(key.data(), std::move(value));
            current->set_copied(true);
            return;
        }

        if (!array_stack_.empty()) {
            auto* arr = array_stack_.back();
            arr->emplace_back(std::move(value));
            return;
        }

        if (!root_) {
            auto base_type = value.get_base_type();
            root_ = new dict{std::move(value)};
            root_->set_base_type(base_type);
            root_->set_heap_allocated(true);
            return;
        }

        throw std::runtime_error("值出现在无效上下文中");
    }

    /**
     * @brief 获取解析后的根dict
     * @return dict 解析完成的dict对象（通过移动构造返回）
     */
    dict GetRoot() {
        if (root_ == nullptr) {
            throw std::runtime_error("未解析到任何根对象");
        }
        dict result = std::move(*root_);
        delete root_;
        root_ = nullptr;
        return result;
    }

    /**
     * @brief 解析JSON字符串到slow_json::dict
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

        while (ss.Peek() == ' ' || ss.Peek() == '\n' || ss.Peek() == '\t' || ss.Peek() == '\r') {
            ss.Take();
        }
        if (ss.Tell() != json_str.size()) {
            throw std::runtime_error("JSON解析不完整，存在未解析的字符");
        }
        dict dict = handler.GetRoot();
        return dict;
    }

private:

    dict* root_; ///< 根dict指针
    std::vector<dict*> object_stack_; ///< 对象栈
    std::vector<std::vector<serializable_wrapper>*> array_stack_; ///< 数组栈
    std::vector<std::string_view> value_key_stack_; ///< 键栈
    std::string_view current_key_; ///< 当前键
};

static slow_json::dict parse_json_to_dict(std::string_view json_str) {
    return DictHandler::parse_json_to_dict(json_str);
}

} // namespace slow_json::details

#endif //SLOWJSON_DICT_HANDLER_HPP
