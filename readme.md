# SlowJSON：简单、高效、类型安全的 C++ JSON 序列化/反序列化库

**目录**

- [为什么选择 SlowJSON？](#为什么选择-slowjson)
- [安装与使用](#安装与使用)
  - [依赖](#依赖)
  - [使用](#使用)
  - [编译选项](#编译选项)
- [开篇示例](#开篇示例)
  - [从一个简单的例子出发](#从一个简单的例子出发)
  - [一句话完成自定义类的序列化与反序列化](#一句话完成自定义类的序列化与反序列化)
  - [复杂 JSON 处理](#复杂-json-处理)
- [与 RapidJSON 对比](#与-rapidjson-对比)
- [核心特性](#核心特性)
  - [Header-Only 设计](#header-only-设计)
  - [广泛的类型支持](#广泛的类型支持)
  - [动态与静态 JSON 处理](#动态与静态-json-处理)
  - [主要接口](#主要接口)
  - [自定义类型序列化（三种方式）](#自定义类型序列化三种方式)
  - [继承支持](#继承支持)
  - [错误处理](#错误处理)
- [性能评估](#性能评估)
- [优势](#优势)
- [局限性](#局限性)
- [未来计划](#未来计划)
- [贡献](#贡献)
- [许可证](#许可证)

---

**SlowJSON** 是一个现代化、header-only 的 C++ JSON 序列化与反序列化库，旨在简化 RapidJSON 的繁琐操作，提供直观、易用的接口，同时保持高性能。利用 C++20 `concepts` 提供广泛类型支持，通过小对象优化（SOO）、编译期优化（如 `static_dict`）和高效内存管理，SlowJSON 并非真的“慢”，其在复杂 JSON 结构和自定义类型序列化场景中性能媲美甚至超越 RapidJSON。

## 为什么选择 SlowJSON？

- **Python 风格的接口**：提供类似 Python `dict` 的便捷 JSON 操作，支持直接将 STL 容器及适配器（如 `std::vector`、`std::unordered_map`）转化为 JSON，无需繁琐代码。
- **自定义类型序列化**：通过 `$config` 宏等机制，一句话实现自定义类的序列化与反序列化，支持嵌套类型，减少开发工作量。
- **高性能与灵活性**：在保持易用性的同时，通过编译期优化和小对象优化实现卓越性能。

## 安装与使用

### 依赖

- **C++ 标准**：C++20（基于 Ubuntu 20.04/GCC 9.4 开发）
- **RapidJSON**：包含在 `3rd_party/rapidjson`（未来计划移除）
- **可选**：OpenCV（支持 `cv::Mat`）、Eigen（支持矩阵和点类型）
- **Header-Only**：无需编译，直接包含头文件即可。

### 使用

1. 克隆仓库：
   ```bash
   git clone https://github.com/shijunfeng00/SlowJSON.git
   ```
2. 配置 CMake：
   ```cmake
   cmake_minimum_required(VERSION 3.16)
   project(SlowJson)
   set(CMAKE_CXX_STANDARD 20)
   include_directories("slowjson" SYSTEM "3rd_party/rapidjson")
   add_executable(SlowJson main.cpp)
   ```
3. 包含头文件：
   ```cpp
   #include <slowjson.hpp>
   ```

### 编译选项

- **Debug 模式**：默认启用 `assert_with_message`，提供详细错误信息（表达式、文件、行号等）并终止程序。
- **Release 模式**：默认移除断言以提升性能。
- **SLOW_JSON_ASSERT_AS_EXCEPTION**：
  - 启用（`-DSLOW_JSON_ASSERT_AS_EXCEPTION=ON`）：将 `assert_with_message` 转化为 `std::runtime_error`，在 Debug/Release 模式下均保留详细错误信息。
  - 示例：
    ```bash
    cmake -DSLOW_JSON_ASSERT_AS_EXCEPTION=ON -DCMAKE_BUILD_TYPE=Debug ..
    make -j
    ./SlowJson
    ```
- **BUILD_TEST_UNIT**：
  - 启用单元测试，验证接口正确性：
    ```bash
    cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TEST_UNIT=ON ..
    make -j
    ./SlowJson
    ```
- **BUILD_BENCHMARK**：
  - 启用基准测试，评估性能：
    ```bash
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARK=ON ..
    make -j
    ./SlowJson
    ```

## 开篇示例

### 从一个简单的例子出发

以下示例展示 SlowJSON 的核心功能：快速构建 JSON、访问数据、生成字符串和反序列化：

```cpp
#include <slowjson.hpp>

int main() {
    // 轻松构建 JSON 字典
    slow_json::dict dict{
        {"key1", "value1"},
        {"key2", 2},
        {"key3", {1, 2, 3, 4, nullptr}},
        {"key4", {
            {"nested-key1", 5.4},
            {"nested-key2", "shijunfeng"}
        }}
    };

    // 访问 JSON 数据
    std::cout << dict["key1"].cast<const char*>() << std::endl;
    std::cout << dict["key2"].cast<int>() << std::endl;

    // 访问列表
    for (auto& v : dict["key3"].as_list()) {
        if (!v.empty()) {
            std::cout << v.cast<int>() << " ";
        }
    }
    std::cout << std::endl;

    // 访问字典
    for (auto& [k, v] : dict["key4"].as_dict()) {
        slow_json::visit(v, [&](auto&& v) {
            std::cout << k << " : " << v << std::endl;
        });
    }

    // 修改字典，支持 STL
    dict["key3"] = std::vector{"a", "b", "c"};
    std::cout << dict["key3"].cast<std::vector<const char*>>()[0] << std::endl;

    // 序列化为 JSON 字符串
    slow_json::Buffer buffer{1024};
    slow_json::dumps(buffer, dict, 4);
    std::cout << "JSON:\n" << buffer.c_str() << std::endl;

    // 从字符串反序列化
    std::string json_str = R"({
        "key1": "value12",
        "key2": 2147,
        "key3": ["AA", "BB", "CC"],
        "key4": {
            "nested-key1": 114.514,
            "nested-key2": "slowjson"
        }
    })";
    slow_json::dict dict2;
    slow_json::loads(dict2, json_str);

    // 验证反序列化结果
    std::cout << dict2["key4"]["nested-key2"].cast<std::string>() << std::endl;
    std::cout << dict2["key3"][0].cast<std::string>() << std::endl;
}
```

**输出**:

```json
value1
2
1 2 3 4
nested-key1 : 5.4
nested-key2 : shijunfeng
a
JSON:
{
    "key1": "value1",
    "key2": 2,
    "key3": [
        "a",
        "b",
        "c"
    ],
    "key4": {
        "nested-key1": 5.4,
        "nested-key2": "shijunfeng"
    }
}
slowjson
AA
```

### 一句话完成自定义类的序列化与反序列化

以下示例展示如何通过 `$config` 宏实现自定义类的序列化与反序列化，支持嵌套类型和 STL 容器：

```cpp
#include <iostream>
#include <slowjson.hpp>

struct Student {
    std::string name;
    int age = 0;
    std::vector<float> scores;
    $config(Student, name, age, scores); // 一句话启用序列化
};

struct Class {
    std::vector<Student> students;
    char class_name[256];
    std::tuple<int, float, std::string> infos;
    $config(Class, students, class_name, infos); // 支持嵌套序列化
};

int main() {
    // 构建 Class 对象
    Class cls;
    cls.students.emplace_back(Student{.name = "shijunfeng", .age = 25, .scores = {100, 99, 99}});
    cls.students.emplace_back(Student{.name = "liujingyi", .age = 24, .scores = {92, 97, 105}});
    memset(cls.class_name, 0, sizeof(cls.class_name));
    memcpy(cls.class_name, "string", sizeof("string"));
    cls.infos = std::tuple{10, 15.f, std::string{"another string"}};

    // 序列化为 JSON
    slow_json::Buffer buffer;
    slow_json::dumps(buffer, cls, 4);
    std::cout << buffer << std::endl;

    // 从字符串反序列化
    Class cls2;
    std::string json_str = R"({
        "students": [{
            "name": "liujingyi",
            "age": 24,
            "scores": [92.0, 97.0, 105.0, 100.0]
        }],
        "class_name": "some class_name",
        "infos": [10, 15.0, "some info string"]
    })";
    slow_json::loads(cls2, json_str);

    // 验证反序列化结果
    std::cout << std::get<2>(cls2.infos) << std::endl;
    std::cout << cls2.class_name << std::endl;
    std::cout << cls2.students[0].scores[1] << std::endl;
}
```

**输出**:

```json
{
    "students": [
        {
            "name": "shijunfeng",
            "age": 25,
            "scores": [
                100.0,
                99.0,
                99.0
            ]
        },
        {
            "name": "liujingyi",
            "age": 24,
            "scores": [
                92.0,
                97.0,
                105.0
            ]
        }
    ],
    "class_name": "string",
    "infos": [
        10,
        15.0,
        "another string"
    ]
}
some info string
some class_name
97
```

### 复杂 JSON 处理

以下示例展示 SlowJSON 的强大功能，包括动态 JSON 构造、自定义类型序列化、反序列化、遍历和 STL 集成：

```cpp
#include <slowjson.hpp>

struct User {
    int id;
    std::string name;
    std::vector<int> scores;
    $config(User, id, name, scores); // 一句话启用序列化
};

int main() {
    // 构造复杂 JSON 结构
    slow_json::dict dict{
        {"user", User{1001, "Alice", {95, 88, 92}}},                       // 自定义类
        {"settings", {{"theme", "dark"}, {"notifications", std::nullopt}}}, // 嵌套字典和 std::nullopt
        {"records", std::vector<std::unordered_map<std::string, int64_t>>{  // STL 容器适配器
            {{"id", 1}, {"timestamp", 1697059200}},
            {{"id", 2}, {"timestamp", 1697062800}}
        }},
        {"coordinates", std::pair<float, float>{1.5f, 2.5f}},              // STL pair
        {"params", {0.1, "string", 105}},                                  // 混合类型列表
        {"dict", {{"key1", "value1"}, {"key2", {"value2", 3, 4, std::tuple{1, 2, 3}}}}}, // 深层嵌套
        {"func", [](){ return "callback"; }}                                // std::function
    };

    // 序列化为格式化 JSON
    slow_json::Buffer buffer;
    slow_json::dumps(buffer, dict, 4);
    std::cout << "Serialized JSON:\n" << buffer.string() << std::endl;

    // 遍历子字典
    std::cout << "Settings:\n";
    for (const auto& [key, value] : dict["settings"].as_dict()) {
        if (value.empty()) {
            std::cout << key << ": " << "null" << std::endl;
        } else {
            std::cout << key << ": " << value.cast<const char*>() << std::endl;
        }
    }

    // 遍历子列表
    std::cout << "Params:\n";
    for (const auto& value : dict["params"].as_list()) {
        slow_json::visit(value, [](auto&& value) { std::cout << value << ","; });
    }
    std::cout << std::endl;

    // 访问自定义类型
    std::cout << "User name: " << dict["user"].cast<User>().name << std::endl;

    // 检查和修改
    if (!dict["settings"].empty() && dict["settings"].contains("theme")) {
        dict["settings"]["theme"] = "white";
    }
    std::cout << "Theme: " << dict["settings"]["theme"].cast<const char*>() << std::endl;

    // 反序列化到自定义类型
    std::string json = R"({"id":1002,"name":"Charlie","scores":[90,85,88]})";
    User user;
    slow_json::loads(user, json);
    std::cout << "Deserialized user: " << user.name << ", ID: " << user.id << std::endl;
}
```

**输出**:

```json
{
    "user": {
        "id": 1001,
        "name": "Alice",
        "scores": [
            95,
            88,
            92
        ]
    },
    "settings": {
        "theme": "dark",
        "notifications": null
    },
    "records": [
        {
            "id": 1,
            "timestamp": 1697059200
        },
        {
            "id": 2,
            "timestamp": 1697062800
        }
    ],
    "coordinates": [
        1.5,
        2.5
    ],
    "params": [
        0.1,
        "string",
        105
    ],
    "dict": {
        "key1": "value1",
        "key2": [
            "value2",
            3,
            4,
            [
                1,
                2,
                3
            ]
        ]
    },
    "func": "callback"
}
Settings:
theme: dark
notifications: null
Params:
0.1,string,105,
User name: Alice
Theme: white
Deserialized user: Charlie, ID: 1002
```

## 与 RapidJSON 对比

生成相同 JSON 结构的 RapidJSON 代码需要繁琐的 DOM 操作，而 SlowJSON 简洁直观：

```cpp
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

int main() {
    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
    writer.SetIndent(' ', 4);

    writer.StartObject();

    // user 字段
    writer.Key("user");
    writer.StartObject();
    writer.Key("id"); writer.Int(1001);
    writer.Key("name"); writer.String("Alice");
    writer.Key("scores"); writer.StartArray();
    writer.Int(95); writer.Int(88); writer.Int(92);
    writer.EndArray();
    writer.EndObject();

    // settings 字段
    writer.Key("settings");
    writer.StartObject();
    writer.Key("theme"); writer.String("dark");
    writer.Key("notifications"); writer.Null();
    writer.EndObject();

    // records 字段
    writer.Key("records");
    writer.StartArray();
    writer.StartObject();
    writer.Key("id"); writer.Int64(1);
    writer.Key("timestamp"); writer.Int64(1697059200);
    writer.EndObject();
    writer.StartObject();
    writer.Key("id"); writer.Int64(2);
    writer.Key("timestamp"); writer.Int64(1697062800);
    writer.EndObject();
    writer.EndArray();

    // coordinates 字段
    writer.Key("coordinates");
    writer.StartArray();
    writer.Double(1.5);
    writer.Double(2.5);
    writer.EndArray();

    // params 字段
    writer.Key("params");
    writer.StartArray();
    writer.Double(0.1);
    writer.String("string");
    writer.Int(105);
    writer.EndArray();

    // dict 字段
    writer.Key("dict");
    writer.StartObject();
    writer.Key("key1"); writer.String("value1");
    writer.Key("key2"); writer.StartArray();
    writer.String("value2");
    writer.Int(3);
    writer.Int(4);
    writer.StartArray();
    writer.Int(1); writer.Int(2); writer.Int(3);
    writer.EndArray();
    writer.EndArray();
    writer.EndObject();

    // func 字段
    writer.Key("func"); writer.String("callback");

    writer.EndObject();

    std::cout << "Serialized JSON:\n" << s.GetString() << std::endl;
}
```

**优势**：SlowJSON 利用模板元编程和 `$config` 宏大幅减少代码量，提升开发效率和可读性。

## 核心特性

### Header-Only 设计

- 无需编译，只需包含 `<slowjson.hpp>`。
- 集成简单，基于 C++20 语法。

### 广泛的类型支持

SlowJSON 通过 C++20 `concepts` 提供类型安全和扩展性，支持以下类型：

- **基本类型**：`int`, `float`, `double`, `bool`, `std::string`, `std::string_view`, `const char*`, C 数组（如 `int[5]`）。
- **STL 容器及适配器**：
  - 列表：`std::vector`, `std::list`, `std::deque`, `std::array`。
  - 字典：`std::map`, `std::unordered_map`。
  - 集合：`std::set`, `std::unordered_set`。
  - 其他：`std::pair`（序列化为 `[key, value]`）、`std::tuple`（序列化为数组）、`std::optional<T>`（`has_value() == false` 序列化为 `null`）、`std::unique_ptr`, `std::shared_ptr`, 原始指针（指向 `nullptr` 序列化为 `null`）、`std::variant`（根据当前类型序列化）、`std::function<void()>`（序列化为返回值，如 `std::string`）。
- **自定义类型**：
  - 满足 `concepts::iterable`（提供 `begin()` 和 `end()`）的列表类型。
  - 满足 `concepts::dict`（提供 `mapped_type` 和迭代器）的字典类型。
  - 通过以下三种方式支持任意自定义类型（包括 OpenCV `cv::Point`, `cv::Mat`, Eigen 类型）。

### 动态与静态 JSON 处理

- **`slow_json::dict`**：
  - 动态 JSON 结构，支持运行时修改、复杂嵌套和混合类型。
  - 使用 `{}` 构造，`cast<T>` 类型无限制（与构造时类型一致）。
  - 优化：小对象优化（SOO）、元数据嵌入、延迟初始化、移动语义。
- **`slow_json::static_dict`**：
  - 针对固定 JSON 结构，基于 `std::tuple`，提供编译期 O(1) 键查找，性能比 `dict` 高数倍，但灵活性较低。
  - 使用静态字符串（如 `"key"_ss`）访问和修改。
  - 示例：
    ```cpp
    #include <iostream>
    #include <slowjson.hpp>

    int main() {
        using namespace slow_json::static_string_literals;
        constexpr auto dict = slow_json::static_dict{
            std::pair{"key1"_ss, 42},
            std::pair{"key2"_ss, slow_json::static_dict{
                std::pair{"key3"_ss, "value3"},
                std::pair{"key4"_ss, std::tuple{1, 2, 3, 4}}
            }}
        };
        constexpr auto value = dict["key1"_ss];
        std::cout << value << std::endl; // 输出: 42
        slow_json::Buffer buffer;
        slow_json::dumps(buffer, dict, 4);
        std::cout << buffer << std::endl;
    }
    ```

    **输出**:
    ```json
    42
    {
        "key1": 42,
        "key2": {
            "key3": "value3",
            "key4": [
                1,
                2,
                3,
                4
            ]
        }
    }
    ```

### 主要接口

- **序列化/反序列化**：
  - `slow_json::dumps(buffer, data, indent)`：序列化为 JSON 字符串，支持缩进。
  - `slow_json::loads(data, json_str)`：从 JSON 字符串解析为 `dict` 或自定义类型。
- **dict 操作**：
  - `is_dict()`：检查是否为字典。
  - `is_array()`：检查是否为列表。
  - `is_fundamental()`：检查是否为基本类型。
  - `empty()`：检查数据是否为 `null`。
  - `contains(key)`：检查是否包含键。
  - `size()`：返回元素数量。
  - `as_dict()`：转换为 `std::unordered_map<const char*, dict>`.
  - `as_list()`：转换为 `std::vector<dict>`.
  - `cast<T>()`：类型转换（反序列化时限于 `int64_t`, `std::string`, `double`；构造时无限制）。
  - `as_type<T>()`：检查类型转换是否可行。
  - `slow_json::visit(fn)`：访问值并调用回调，类似 `std::visit`，需显示指定类别，默认为 `<int64_t, double, std::string>`。
  - 示例：
    ```cpp
    for (const auto& [key, value] : dict.as_dict()) {
        slow_json::visit(value, [](auto&& v) {
            std::cout << "Value: " << v << std::endl;
        });
    }
    ```

### 自定义类型序列化（三种方式）

1. **推荐：使用 `$config` 系列宏**：
   - 使用 `$config`, `$config_decl`, `$config_impl`（无继承）或 `$$config`, `$$config_decl`, `$$config_impl`（支持继承）定义字段，编译期类型安全。
   - 示例：
     ```cpp
     struct User {
         int id;
         std::string name;
         $config(User, id, name);
     };
     User user{1001, "Alice"};
     slow_json::Buffer buffer;
     slow_json::dumps(buffer, user); // 输出: {"id":1001,"name":"Alice"}
     ```
   - 支持声明与实现分离：
     ```cpp
     struct User {
         int id;
         std::string name;
         $config_decl(User, id, name);
     };
     $config_impl(User, id, name);
     ```
2. **非侵入式：偏特化 `DumpToString`/`LoadFromDict`**：
   - 适合第三方类或无法修改的类，支持 OpenCV `cv::Point`, `cv::Mat`, Eigen 类型等。
   - 示例（支持 OpenCV `cv::Mat`）：
     ```cpp
     #include <opencv2/core.hpp>
     template<> struct slow_json::DumpToString<cv::Mat> {
         static void dump(slow_json::Buffer& buffer, const cv::Mat& mat) {
             buffer.append('[');
             for (int i = 0; i < mat.rows; ++i) {
                 buffer.append('[');
                 for (int j = 0; j < mat.cols; ++j) {
                     slow_json::DumpToString<double>::dump(buffer, mat.at<double>(i, j));
                     if (j < mat.cols - 1) buffer.append(',');
                 }
                 buffer.append(']');
                 if (i < mat.rows - 1) buffer.append(',');
             }
             buffer.append(']');
         }
     };
     template<> struct slow_json::LoadFromDict<cv::Mat> {
         static void load(cv::Mat& mat, const slow_json::dynamic_dict& dict) {
             auto list = dict.as_list();
             mat.create(list.size(), list[0].as_list().size(), CV_64F);
             for (size_t i = 0; i < list.size(); ++i) {
                 auto row = list[i].as_list();
                 for (size_t j = 0; j < row.size(); ++j) {
                     mat.at<double>(i, j) = row[j].cast<double>();
                 }
             }
         }
     };
     cv::Mat mat = (cv::Mat_<double>(2, 2) << 1.0, 2.0, 3.0, 4.0);
     slow_json::dict dict{{"matrix", mat}};
     slow_json::dumps(buffer, dict); // 输出: {"matrix":[[1,2],[3,4]]}
     ```
3. **面向对象派生类：继承 `ISerializable`**：
   - 实现 `get_config` 和 `from_config`，支持自定义逻辑（如处理缺失键或 `null` 值）。
   - 示例：
     ```cpp
     struct User : slow_json::ISerializable {
         int id = 0; // 默认值
         std::string name;
         slow_json::dict get_config() const noexcept override {
             return {{"id", id}, {"name", name}};
         }
         void from_config(const slow_json::dynamic_dict& json) override {
             id = json.contains("id") && !json["id"].empty() ? json["id"].cast<int>() : 0;
             name = json.contains("name") && !json["name"].empty() ? json["name"].cast<std::string>() : "";
         }
     };
     ```

### 继承支持

- 使用 `$$config` 宏支持多级继承，自动合并基类和派生类字段。
- 示例：
  ```cpp
  struct Base {
      int x;
      $config(Base, x);
  };
  struct Derived : Base {
      std::string y;
      $$config(<Base>, Derived, y);
  };
  Derived obj{10, "test"};
  slow_json::Buffer buffer;
  slow_json::dumps(buffer, obj); // 输出: {"x":10,"y":"test"}
  ```

### 错误处理

- **类型安全**：通过 `concepts` 和 `assert_with_message` 确保类型正确性。
- **错误信息**：Debug 模式下提供详细错误信息（表达式、文件、行号等）。
- **示例（类型转换错误）**：
  ```cpp
  slow_json::dict dict{{"value", 42}};
  dict["value"].cast<float>(); // 抛出: 类型不正确，预期为`float`，实际为`int64_t`
  ```

## 性能评估

测试环境：AMD EPYC 7C13 64-Core Processor（2.0 GHz, 30 核, 64-bit, L2 Cache 64KB）。

| 场景                   | static_dict | dict   | RapidJSON |
|------------------------|-------------|--------|-----------|
| 简单 JSON 序列化       | 47 ms       | 166 ms | 433 ms    |
| 复杂嵌套 JSON 序列化   | 264 ms      | 593 ms | 1563 ms   |
| 简单 C++ 对象序列化    | 42 ms       | N/A    | 333 ms    |
| 简单 C++ 对象反序列化  | 433 ms      | N/A    | 367 ms    |
| 复杂 C++ 对象序列化    | 544 ms      | N/A    | 2629 ms   |
| 复杂 C++ 对象反序列化  | 1863 ms     | N/A    | 1767 ms   |

## 优势

- **易用性**：`{}` 语法和 `$config` 宏简化 JSON 构造和序列化。
- **灵活性**：支持广泛类型，动态/静态 JSON 处理。
- **高性能**：序列化性能优于 RapidJSON，反序列化性能接近。
- **类型安全**：C++20 `concepts` 和运行时检查确保安全。

## 局限性

- `dict::cast<T>` 反序列化时仅支持 `int64_t`, `std::string`, `double`。
- 复杂嵌套结构序列化性能优于反序列化。

## 未来计划

- 移除 RapidJSON 依赖。
- 扩展 `cast<T>` 支持更多类型。
- 优化 `dict` 键查找。
- 支持 `dict["key"][index]` 风格穿透`std::vector<T>`等直接访问。

## 贡献

欢迎提交 Issue 或 Pull Request，创建一个现代化的高效便捷的C++ JSON库！

## 许可证

MIT License