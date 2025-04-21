
# SlowJSON：简单通用的 C++ JSON 处理库

- SlowJSON是一个轻量级的 C++ JSON 处理库，旨在让开发者在 C++ 中也能像 Python 一样轻松处理 JSON 数据，同时保持高性能和灵活性。

- SlowJSON 力求以最简单的方式让自定义C++对象支持序列化与反序列化

- SlowJSON基于 RapidJSON 实现，采用 header-only 设计，基于模板元编程技术支持广泛的 C++ 类型，包括基本类型、STL 容器及其组合，以及用户自定义类。

- 无论是序列化还是反序列化，SlowJSON 都力求简单、高效、灵活。

## 为什么选择 SlowJSON？

- **简洁的使用接口**：通过 `dumps`、`loads`、`dict`、`static_dict`等接口轻松构建和解析JSON。
- **强大的泛型支持**：基于模板元编程，支持基本类型、STL 容器及其任意组合。
- **用户自定义类支持**：通过侵入式宏或非侵入式特化，快速实现自定义类的序列化和反序列化。
- **不算差的性能**:虽然该库名称为`SlowJSON`，但该库基于`RapidJSON`来实现序列化与反序列化，通过模板尽可能降低运行开销，性能并不会真的很差。
- **易于集成**：采用header-only设计，且无需其他额外依赖。

## 待解决的问题

- **可能缺乏一定的灵活性**: 该库主要设计于JSON解析和生成JSON，以及对象序列化和反序列化，对于JSON的修改可能缺乏一定的支持。

## 快速开始

### 使用 `dict` 和 `dumps` 构建 JSON

使用`slow_json::dict`，可以像python的字典那样快速且轻松的构建JSON，并且可以和C++的`STL`、`std::pair`、`std::optional`等组件组合使用

```cpp
#include "slowjson.hpp"
#include <iostream>
int main() {
    slow_json::Buffer buffer(1000);
    slow_json::dict dict{
            {"name", "SlowJSON"},
            {"version", 1.0},
            {"features", std::vector<std::string>{"easy", "generic"}},
            {"others", std::tuple{"age", 19}},
            {"nested_dict",{
                {"a",1},
                {"b",2},
                {"c",{1,2,3,4,5,6}},
                {"d",nullptr}
            }}
    };
    slow_json::dumps(buffer, dict, 4);
    std::cout << buffer << std::endl;
}
```

代码运行结果如下

```json
{
    "nested_dict":{
        "d":null,
        "c":[
            1,
            2,
            3,
            4,
            5,
            6
        ],
        "b":2,
        "a":1
    },
    "version":1.0,
    "others":[
        "age",
        19
    ],
    "features":[
        "easy",
        "generic"
    ],
    "name":"SlowJSON"
}
```
### 使用 `static_dict` 和`dumps`构建 JSON

* 如果完全确定JSON的结构的情况下，相比于`slow_json::dict`来说，`slow_json::static_dict`充分借助了模板元编程来在稍微牺牲易用性的情况下大幅提高了性能。

* 同样如`slow_json::dict`，`slow_json::static_dict`也可直接使用`STL`对象来定义`JSON`，同样可以像`Python`的字典一样简洁高效。

```cpp
#include "slowjson.hpp"
#include <iostream>
int main() {
    slow_json::Buffer buffer(1000);
    slow_json::static_dict dict{
            std::pair{"name", "SlowJSON"},
            std::pair{"version", 1.0},
            std::pair{"features", std::vector<std::string>{"easy", "generic"}},
            std::pair{"others", std::tuple{"age", 19}}
    };
    slow_json::dumps(buffer, dict, 4);
    std::cout << buffer << std::endl;
}
```

**输出：**

```json
{
  "name":"SlowJSON",
  "version":1.0,
  "features":[
    "easy",
    "generic"
  ],
  "others":[
    "age",
    19
  ]
}
```

### 使用 `dynamic_dict`解析 JSON 字符串

`dynamic_dict` 提供动态访问 JSON 数据的能力，同样类似于 `Python` 的字典简洁高效。

```cpp
#include "slowjson.hpp"
#include <iostream>

int main() {
    std::string json_str = R"({
        "x": [4],
        "y": [1],
        "z": [2, 3, 4, 5, 6],
        "t": null,
        "object": {
            "name": "zhihu"
        }
    })";
    slow_json::dynamic_dict dict(json_str);
    std::cout << dict["object"]["name"].cast<std::string>() << std::endl; // 直接访问数据
    std::cout << dict["t"].empty() << std::endl;                          // 是否为 null
    std::cout << dict["x"].empty() << std::endl;                          // 是否为 null
    std::cout << dict.contains("x") << std::endl;                         // 是否存在字段"x"
    std::cout << dict.contains("xx") << std::endl;                        // 是否存在字段"xx"
    std::cout << dict["z"].size() << std::endl;                           // 获取数组大小
}
```

**输出：**

```
zhihu
1
0
1
0
5
```

SlowJSON 也可以配合 `std::optional` 来处理 JSON 中值可能为空值(`null`)的情况。

```cpp
#include "slowjson.hpp"
#include <iostream>

int main() {
    slow_json::dynamic_dict dict("\"null\"");
    auto result = dict.cast<std::optional<std::string>>();
    if (result) {
        std::cout << result.value() << std::endl;
    } else {
        std::cout << "空数据" << std::endl;
    }
}
```

**输出：**

```
空数据
```

### 使用 `loads` 解析JSON字符串

`loads` 将 JSON 字符串直接反序列化为 C++ 对象。

```cpp
#include "slowjson.hpp"
#include <iostream>

int main() {
    std::string json_str = R"([1, null, 3])";
    std::vector<std::optional<int>> vec; //适配可能存在空值的情况，否则会触发断言。
    slow_json::loads(vec, json_str);
    for (const auto& num : vec) {
        std::cout << num.value_or(-1) << " ";
    }
    std::cout << std::endl;
}
```

**输出：**

```
1 -1 3
```


### 使用`$config`让自定义类支持`JSON`序列化

通过`<slow_json/macro.hpp>`中定义的`$config`宏，可以让用户自定义类也轻松支持JSON序列化和反序列化，只需要一行代码即可声明。

- 宏函数定义:`$config(类名,属性列表...)`

```cpp
#include "slowjson.hpp"
#include <iostream>

struct Test {
    int a;
    std::string b;
    std::vector<int> c;
    $config(Test, a, b, c);  // 宏定义序列化配置，如果有需要，其实可以只让部分属性参与序列化与反序列化
};

int main() {
    Test test{1, "example", {1, 1, 4, 5, 1, 4}};
    slow_json::Buffer buffer(1000);
    slow_json::dumps(buffer, test);
    std::cout << buffer << std::endl; //测试序列化结果
    
    
    std::string json_str=R"(
        {"a":12,"b":"sjf","c":[10,26]}
    )";
    slow_json::loads(test,json_str);
    std::cout<<test.a<<std::endl<<test.b<<std::endl; //测试反序列化结果
    for(auto&v:test.c){
        std::cout<<v<<" ";
    }
}
```



**输出：**

```text
{"a":1,"b":"example","c":[1,1,4,5,1,4]}
12
sjf
10 26 
```
### 使用`$config_decl`和`$config_impl`让自定义类支持`JSON`序列化

- 宏函数定义:`$config_decl(类名,属性列表...)`
- 宏函数定义:`$config_impl(类名,属性列表...)`

适用于需要分离定义和实现的场景，使用前者在类头文件中中进行声明，使用后者在源码文件中构建实现
```cpp
//test.hpp
#ifndef SLOWJSON_TEST_HPP
#define SLOWJSON_TEST_HPP
#include "slowjson.hpp"
struct Test {
    int a;
    std::string b;
    std::vector<int> c;
    $config_decl(Test, a, b, c);  // 宏定义序列化配置，这里只提供声明没有实现
};

#endif //SLOWJSON_TEST_HPP

//test.cpp
#include "test.hpp"
$config_impl(Test, a, b, c);  // 宏定义序列化配置，在源码文件中提供具体实现

//main.cpp
#include "slowjson.hpp"
#include "test.hpp"
#include <iostream>

int main() {
    Test test{1, "example", {1, 1, 4, 5, 1, 4}};
    slow_json::Buffer buffer(1000);
    slow_json::dumps(buffer, test);
    std::cout << buffer << std::endl; //测试序列化结果
    std::string json_str=R"(
            {"a":12,"b":"sjf","c":[10,26]}
        )";
    slow_json::loads(test,json_str);
    std::cout<<test.a<<std::endl<<test.b<<std::endl; //测试反序列化结果
    for(auto&v:test.c){
    std::cout<<v<<" ";
    }
}
```
**输出：**

```text
{"a":1,"b":"example","c":[1,1,4,5,1,4]}
12
sjf
10 26 
```

## 支持的类型

SlowJSON 支持以下类型及其组合：

- **基本类型**：`int`、`float`、`bool`、`std::string`、`std::string_view`、`const char*` 等。
- **STL 容器**：`std::vector`、`std::map`、`std::tuple`、`std::deque` 、std::list等。
- **指针与可选值**：`T*`、`std::shared_ptr<T>`、`std::optional<T>`，并支持将 `nullptr` 和 `std::nullopt`视作空值。
- **C/C++数组**：`std::array<T, N>`、`T[N]`。
- **元组和键值对**：`std::tuple`、`std::pair`、`slow_json::static_dict`、`slow_json::dict`。
- **枚举型变量**：`enum{...}`，如`enum{A,B,C}`,将`A`变为字符串`"A"`，或者从字符串`"C"`还原枚举型变量`C`
- **用户自定义类型**：通过侵入式或非侵入式方式提供支持的类,。
- **复杂嵌套类型**：上述类型的任意组合，例如std::tuple<std::unordered_map<std::string,MyClass>,std::vector<int>>


## 不支持的类型处理

对于不支持的类型，SlowJSON 在 debug 模式下会触发断言失败，并输出详细错误信息，帮助用户定位问题。

**示例：序列化不支持的类型**

```cpp
#include "slowjson.hpp"
#include <opencv2/core.hpp>

int main() {
    slow_json::Buffer buffer(1000);
    cv::Mat mat = (cv::Mat_<int>(3, 3) << 1, 2, 3, 4, 5, 6, 7, 8, 9);
    std::vector<cv::Mat> data{mat};
    slow_json::dumps(buffer, data);  // 触发断言
}
```

**输出（debug 模式）：**

```
程序断言失败,程序退出
断言表达式:SLOW_JSON_SUPPORTED=false
文件:/path/to/slowjson/dump_to_string_interface.hpp
行数:25
函数名称:static void slow_json::DumpToString<T>::dump_impl(slow_json::Buffer&, const T&) [with T = cv::Mat]
断言错误消息:无法将类型为'cv::Mat'的对象正确转换为字符串，找不到对应的DumpToString偏特化类
terminate called without an active exception
```

**示例：反序列化不支持的类型**

```cpp
#include "slowjson.hpp"
#include <opencv2/core.hpp>

int main() {
    std::string json_str = R"([[1,2,3],[4,5,6],[7,8,9]])";
    cv::Mat mat;
    slow_json::loads(mat, json_str);  // 触发断言
}
```

**输出（debug 模式）：**

```
程序断言失败,程序退出
断言表达式:SLOW_JSON_SUPPORTED=false
文件:/path/to/slowjson/load_from_dict_interface.hpp
行数:23
函数名称:static void slow_json::LoadFromDict<T>::load_impl(T&, const slow_json::dynamic_dict&) [with T = cv::Mat]
断言错误消息:无法将JSON字段转化为类型为'cv::Mat'的对象正确对象，找不到对应的LoadFromDict偏特化类
terminate called without an active exception
```


## 核心组件

### `static_dict`

- **描述**：编译期静态字典，用于序列化固定结构的 JSON。
- **特点**：利用模板元编程，性能优异。
- **限制**：含模板参数，无法在头文件和实现分离时使用。

### `dict`

- **描述**：运行时动态字典，用于序列化，支持声明和实现分离。
- **特点**：基于 `std::function`，适用于 `.hpp`/`.cpp` 分离场景。
- **限制**：仅用于序列化，不支持动态访问。

### `dynamic_dict`

- **描述**：动态解析 JSON，提供类似 Python 字典的访问方式。
- **特点**：支持运行时访问和类型转换，如 `json["key"].cast<T>()`。
- **用途**：解析 JSON 字符串并动态操作。


### `dynamic_dict` vs `loads`

- **`dynamic_dict`**：将 JSON 字符串解析为动态对象，支持类似 `json["a"].cast<std::vector<int>>()` 的访问，保留 JSON 结构。
- **`loads`**：将 JSON 字符串直接反序列化为具体的 C++ 对象，如 `std::vector<int>`，无需手动转换。

## 主要接口

- **描述**：字符串缓冲区，提供类似`std::string`的接口。
- **特点**：在对象序列化得到`JSON`中会频繁使用短字符串，可避免频繁的内存申请、释放和复制。
- **用途**：用于序列化得到 JSON 字符串的字符串缓冲区。


### `slow_json::Buffer`
```cpp
/**
* @brief 一个用于生成JSON的字符串缓区，支持类似std::vector的动态大小变化
* @details 该类的实现接口命名风格模仿std::string和std::vector，但部分功能上存在一些细微的差异
* 例如在resize和reserve的时候不会尝试去清0多多余的元素
*/
struct Buffer final;
```

### `slow_json::dumps`

```cpp
/**
 * 对 DumpToString 特化类做一个封装，根据不同类型去调用不同的偏特化函数
 * @tparam T 被转换为 JSON 的对象类型
 * @param buffer 存储转换之后的 JSON 的缓冲区对象
 * @param value 被转化为 JSON 的对象
 * @param indent 首行缩进长度，如果不需要可设置为空
 */
template<typename T>
static void dumps(Buffer &buffer, const T &value, std::optional<std::size_t> indent = std::nullopt);
```

- **功能**：将 C++ 对象序列化为 JSON 字符串。
- **参数**：
  - `buffer`：存储 JSON 的 `slow_json::Buffer` 对象。
  - `value`：待序列化的 C++ 对象。
  - `indent`（可选）：缩进长度，默认无缩进。
- **返回值**：无，直接修改 `buffer`。

### `slow_json::loads`

```cpp
/**
 * 从字符串中加载 JSON，并用其来反序列化对象
 * @tparam T 被反序列化的对象类型
 * @param value 被反序列化的对象
 * @param json JSON 字符串
 */
template<typename T>
static void loads(T &value, const std::string &json);
```

- **功能**：将 JSON 字符串反序列化为 C++ 对象。
- **参数**：
  - `value`：目标 C++ 对象。
  - `json`：JSON 字符串。
- **返回值**：无，直接修改 `value`。


### `dynamic_dict` 的主要接口

以下是 `dynamic_dict` 的核心接口及其声明：

- **`bool empty() const`**
  - **功能**：检查当前节点是否为 `null`。
  - **返回值**：`true` 表示为空，`false` 表示非空。
  - **示例**：
    ```cpp
    slow_json::dynamic_dict dict(R"({"key": null})");
    std::cout << dict["key"].empty() << std::endl;  // 输出：1
    ```

- **`std::size_t size() const`**
  - **功能**：如果当前节点是数组或对象，返回其元素个数；否则抛出异常。
  - **返回值**：元素个数。
  - **示例**：
    ```cpp
    slow_json::dynamic_dict dict(R"({"arr": [1, 2, 3]})");
    std::cout << dict["arr"].size() << std::endl;  // 输出：3
    ```

- **`template<typename T> T cast() const`**
  - **功能**：将当前节点的值转换为指定类型 `T`。
  - **返回值**：转换后的值，若类型不匹配则抛出异常。
  - **示例**：
    ```cpp
    slow_json::dynamic_dict dict(R"({"key": "value"})");
    std::string value = dict["key"].cast<std::string>();
    std::cout << value << std::endl;  // 输出：value
    ```

**完整示例：**

```cpp
#include "slowjson.hpp"
#include <iostream>

int main() {
    slow_json::dynamic_dict dict(R"({"key": "value", "arr": [1, 2, 3], "null_val": null})");
    std::cout << dict["key"].cast<std::string>() << std::endl;  // 输出：value
    std::cout << dict["arr"].size() << std::endl;               // 输出：3
    std::cout << dict["null_val"].empty() << std::endl;         // 输出：1
}
```

## 支持用户自定义类

### 侵入式支持

实际上对于`SlowJSON`而言，需要自定义对象提供如下的`get_config`成员函数，
根据使用场景选择返回`static_dict`(`auto`)或者`dict`，前者用于`header only`，后者用于需要声明和实现分离的场景。
```cpp
#include "slowjson.hpp"
#include <vector>

using namespace slow_json::static_string_literals;
struct Node{
    int x=1;
    std::vector<float> y={1.2,3.4};
    std::string z="STR";
    static constexpr auto get_config()noexcept{
        return slow_json::static_dict{
                std::pair{"x"_ss,&Node::x},
                std::pair{"y"_ss,&Node::y},
                std::pair{"z"_ss,&Node::z}
        };
    }
};

struct Node2{
    int x=1;
    std::vector<float> y={1.2,3.4};
    std::string z="STR";
    static slow_json::dict get_config()noexcept{
        return slow_json::dict{
                std::pair{"x"_ss,&Node::x},
                std::pair{"y"_ss,&Node::y},
                std::pair{"z"_ss,&Node::z}
        };
    }
};

int main() {
    Node node;
    slow_json::Buffer buffer(1000);
    slow_json::dumps(buffer, node);
    std::cout << buffer << std::endl;

    Node2 node2;
    buffer.clear();
    slow_json::dumps(buffer, node2);
    std::cout << buffer << std::endl;
}
```
该代码运行结果为
```json lines
{"x":1,"y":[1.2,3.4],"z":"STR"}
{"x":1,"y":[1.2,3.4],"z":"STR"}
```


### 侵入式支持（使用宏）

`SlowJSON` 提供了一系列宏（定义在 `macro.hpp` 中）来简化用户自定义类的序列化配置，无需手动编写 `get_config()`。

#### `$config`

- **功能**：为类定义一个返回 `static_dict`的静态成员函数`get_config`函数的声明和实现。
- **参数**：类名和成员变量列表。
- **示例**：
  ```cpp
  struct Example {
      int x;
      std::string y;
      $config(Example, x, y);
  };
  ```

#### `$config_decl` 和 `$config_impl`

- **功能**：支持声明/实现分离。
- **参数**：类名和成员变量列表。
- **示例**：

```cpp
//poly_example.hpp
struct PolyExample {
  int a;
  std::string b;
  $config_decl(PolyExample,a,b);  // 声明
};
//poly_example.cpp
$config_impl(PolyExample, a, b);  // 实现
```


### 非侵入式支持

很多类已经写好了，尤其是一些第三方库的类，例如一开始的例子`cv::Mat`，我们不可能去修改`OpenCV`源码，因此这种情况下，可以考虑非侵入式的方法来提供支持


通过特化 `DumpToString` 和 `LoadFromDict`，支持第三方库类型（如 OpenCV 的 `cv::Mat`）。

```cpp
#include <opencv2/core.hpp>
namespace slow_json {
    template<>
    struct DumpToString<cv::Mat> : public IDumpToString<DumpToString<cv::Mat>> {
        static void dump_impl(Buffer& buffer, const cv::Mat& value) {
            std::vector<std::vector<int>> vec(value.rows, std::vector<int>(value.cols));
            for (int i = 0; i < value.rows; ++i)
                for (int j = 0; j < value.cols; ++j)
                    vec[i][j] = value.at<int>(i, j);
            DumpToString<decltype(vec)>::dump(buffer, vec);
        }
    };
}
```


这样`slow_json`便可以处理`cv::Mat`类型了

```cpp
#include "slowjson.hpp"
#include "opencv4/opencv2/opencv.hpp"

#include <iostream>
namespace slow_json {
    template<>
    struct DumpToString<cv::Mat> : public IDumpToString<DumpToString<cv::Mat>> {
        static void dump_impl(Buffer& buffer, const cv::Mat& value) {
            std::vector<std::vector<int>> vec(value.rows, std::vector<int>(value.cols));
            for (int i = 0; i < value.rows; ++i)
                for (int j = 0; j < value.cols; ++j)
                    vec[i][j] = value.at<int>(i, j);
            DumpToString<decltype(vec)>::dump(buffer, vec);
        }
    };
}
struct ImageMerger{
    std::optional<int> x=100,y=120,w=1000,h=2000;
    cv::Mat transform_mat= (cv::Mat_<int>(3, 3) << 1, 2, 3, 4, 5, 6, 7, 8, 9);
    $config(ImageMerger,x,y,w,h,transform_mat);
};

int main() {
    slow_json::Buffer buffer(1000);
    ImageMerger merger;
    slow_json::dumps(buffer,merger,4);
    std::cout<<buffer<<std::endl;
}
```

这段代码的输出如下：

```text
{
    "x":100,
    "y":120,
    "w":1000,
    "h":2000,
    "transform_mat":[
        [
            1,
            2,
            3
        ],
        [
            4,
            5,
            6
        ],
        [
            7,
            8,
            9
        ]
    ]
}
```

## 扩展功能

### 访问和修改`static_dict`中的元素

待补充

### 何时使用`dict`来替代`static_dict`

待补充

`static_dict`是一个带有模板参数的模板类，无法构建一个类似`std::vector<static_dict>`的对象

### 对于继承与派生类的支持

`SlowJSON`支持对于派生类序列化，可以使用`$$config`,`$$config_decl`,`$$config_impl`来完成，相比于`$`版本，多了一个参数，即第一个参数提供
父类的信息（目前暂时还不支持多继承的情况，未来有时间补充）

- 宏函数定义:`$$config(<父类名列表>,类名,属性列表...)`
- 宏函数定义:`$$config_impl(<父类名列表>,类名,属性列表...)`
- 宏函数定义:`$$config_impl(<父类名列表>,类名,属性列表...)`
- **参数**：父类类名列表，类名和成员变量列表。


```cpp
#include "slowjson.hpp"
#include "test.hpp"
#include <iostream>

struct Base {
    int a;
    $config(Base, a);
};

struct Derived : Base {
    std::string b;
    $$config(<Base>,Derived,b);
};

int main() {
    Derived d{1, "derived"};
    slow_json::Buffer buffer(1000);
    slow_json::dumps(buffer, d);
    std::cout << buffer << std::endl;
}
```

**输出：**

```json
{"a":1,"b":"derived"}
```