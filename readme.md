# SlowJson：一个简单通用的C++JSON处理库

本框架的目的是提供一套非常简单的的JSON处理库（当然主要是用于对象的序列化和反序列化，对于JSON数据的修改支持很弱，目前只有一些非常有限的接口）

该项目大量采用模板元编程和模板特化技巧优化性能并使得框架更加通用，使用者可以像写`Python`一样非常方便的去构造和解析JSON数据

# 更新日志：

- 2024/8/9

对于 `static_dict` 添加修改键值对数据的支持

# 使用说明

该库采用header only的设计，因此只需要在`CMakeLists.txt`中导入头文件即可

该库在`Linux/gcc9.4`下完成编写，没有对非`gcc`环境做适配，后面有时间再说

```cmake
include_directories(
        "slowjson"
        "3rd_party/rapidjson"
)
```

# 主要接口

## `slow_json::Buffer`

```cpp
    /**
     * @brief 一个用于生成JSON的字符串缓区，支持类似std::vector的动态大小变化
     * @details 该类的实现接口模仿std::string，几本可以平替，但是存在一些细微的差异，例如在resize和reserve的时候不会尝试去清0多多余的元素
     */
struct Buffer final;
```

## `slow_json::static_dict`

```cpp
    /**
     * 编译期静态字典
     * @tparam Args 字典的简直对类型，为std::pair
     */
template<typename ...Args>
struct static_dict :
public std::tuple<Args...>;
```

## `slow_json::dynamic_dict`

```cpp
    /**
     * @brief 一个可以动态访问和转换 JSON 数据的结构体
     * @details 这个结构体使用了 rapidjson 库来存储和操作 JSON 数据，提供了方便的重载运算符和类型转换方法，可以根据不同的键或索引来获取 JSON 对象或数组中的元素，并将其转换为所需的类型。
     * @note 这个结构体不负责内存管理，如果是从 JSON 字符串创建的对象，则会在析构时释放内存，如果是从已有的 Value 引用创建的对象，则不会释放内存。
     */
class dynamic_dict;
```

## `slow_json::polymorphic_dict`

```cpp
    /**
     * @brief 基于多态(std::function)实现的多态字典，避免模板参数
     * @details 不支持数据查寻，只能支持将其转化为JSON，或者配合get_config()将对象与JSON相互转换
     */
struct polymorphic_dict;
```

## `slow_json::ISerializable`

```cpp
    /**
     * @brief 规定一个类型是可以与JSON相互转换的接口类
     * @details 该接口定义了两个纯虚函数，get_config和from_config，分别用于将对象转换为JSON，和将JSON转换为对象
     *          因此实现了这两个接口的类，都是可序列化的类
     */
struct ISerializable;
```

## `slow_json::loads`

```cpp
    /**
     * 从字符串中加载JSON，并用其来反序列化对象
     * @tparam T 被反序列化的对象类型
     * @param value 被反序列化的对象
     * @param json JSON字符串
     */
template<typename T>
static void loads(T &value, const std::string &json);
```

## `slow_json::dumps`

```cpp
    /**
     * 对DumpToString特化类做一个封装，根据不同类型去调用不同的偏特化函数
     * @tparam T 被转换为JSON的对象类型
     * @param buffer 存储转换之后的JSON的缓冲区对象
     * @param value 被转化为JSON的对象
     * @param indent 首行缩进长度，如果不需要可设置为空
     */
template<typename T>
static void dumps(Buffer &buffer, const T &value, std::optional<std::size_t> indent = std::nullopt);
```

# 类型支持

该库支持的数据类型十分丰富，基本涵盖大部分C++常用类型，主要可以描述为下面的类型：

- `int`、`float`、`bool`、`std::string`、`std::string_view`、`const char*` 等基本类型

- `std::vector`、`std::map`、,`std::tuple`、`std::deque` 等STL容器和容器适配器类型。

- `T*`,`std::shared_ptr<T>`,`std::optional<T>`，并支持处理`nullptr`和`std::nullopt`

- `C++`风格的`std::array<T,N>`数组和`C`语言风格的`int[N]`数组

- `std::tuple`,`std::pair`,`slow_json::static_dict`

- 实现特定接口的用户自定义类型（侵入式或非侵入式）

- 上述类型的任意组合出来的复杂类型

# 将C++对象序列化为JSON字符串

## 处理不支持的类型

开篇先说说本框架如果遇到不支持的类型会怎么样，幸运的是，本框架（通常）不会像大多数模板库一样得到又臭有长的编译期错误，如果某个类型不能被序列化，则会得到一个简单的断言失败，并输出一些错误信息。

```cpp
int main(){
    slow_json::Buffer buffer(1000);
    cv::Mat mat = (cv::Mat_<int>(3, 3) << 1, 2, 3, 4, 5, 6, 7, 8, 9);
    std::vector<cv::Mat>data{mat};
    slow_json::dumps(buffer,data); //显然框架肯定不能直接支持cv::Mat这种无法预料的第三方库
}
```

这段代码会正常编译通过，并输出如下的信息（目前仅在debug模式产生）

```text
程序断言失败,程序退出
断言表达式:SLOW_JSON_SUPPORTED=false
文件:/project/石峻峰-实验性项目/SlowJson重构/slowjson/dump_to_string_interface.hpp
行数:25
函数名称:static void slow_json::DumpToString<T>::dump_impl(slow_json::Buffer&, const T&) [with T = cv::Mat]
断言错误消息:无法将类型为'cv::Mat'的对象正确转换为字符串，找不到对应的DumpToString偏特化类
terminate called without an active exception
```

从这段错误信息中可以很容易知道具体是什么类型不被支持(`cv::Mat`)
，从而帮助用户去查找原因，或编写对应的类型支持代码。我们将在稍后的章节介绍如何编写代码使得`slow_json`可以支持`cv::Mat`。

## C++基本类型、STL容器和容器适配器

该框架可以非常方便的把C++基本类型以及STL容器和部分容器适配器直接转化为对应的`JSON`
字符串数据，也支持将这些类型组合的嵌套类型直接转换为对应的`JSON`字符串，而无需自己写相应的代码

```cpp
#include "slowjson.hpp"
#include <unordered_set>
#include <iostream>

int main(){
    int x=123;
    double y=123.4567890123;
    std::string str="cpp_string";
    const char* c_str="c_string";
    int a[]={1,2,3,4};
    std::vector<float>vec{1.1,2.2,3.3,4.4};
    std::deque<int>deq{5,6,7,8};
    std::unordered_set<std::string>u_set{"A","B","C"};
    std::tuple tp{
        std::pair{"A",5},
        std::tuple{
            "string",
            std::vector{1,2,3},
            std::nullopt,
            std::make_shared<int>(4)
        }
    };
    slow_json::Buffer buffer{1000};
    slow_json::dumps(buffer,x);
    std::cout<<buffer<<std::endl;
    buffer.clear();

    slow_json::dumps(buffer,y);
    std::cout<<buffer<<std::endl;
    buffer.clear();

    slow_json::dumps(buffer,str);
    std::cout<<buffer<<std::endl;
    buffer.clear();

    slow_json::dumps(buffer,c_str);
    std::cout<<buffer<<std::endl;
    buffer.clear();

    slow_json::dumps(buffer,a);
    std::cout<<buffer<<std::endl;
    buffer.clear();

    slow_json::dumps(buffer,vec);
    std::cout<<buffer<<std::endl;
    buffer.clear();

    slow_json::dumps(buffer,deq);
    std::cout<<buffer<<std::endl;
    buffer.clear();

    slow_json::dumps(buffer,u_set);
    std::cout<<buffer<<std::endl;
    buffer.clear();

    slow_json::dumps(buffer,tp,4);
    std::cout<<buffer<<std::endl;
    buffer.clear();
}
```

该代码的输出结果为

```text
123
123.4567890123
"cpp_string"
"c_string"
[1,2,3,4]
[1.1,2.2,3.3,4.4]
[5,6,7,8]
["C","B","A"]
[
    [
        "A",
        5
    ],
    [
        "string",
        [
            1,
            2,
            3
        ],
        null,
        4
    ]
]
```

## 枚举变量

特殊的，`slowjson`提供了对于`enum`的支持

```cpp
#include "slowjson.hpp"
#include <iostream>
enum Color {
    RED,
    GREEN,
    BLUE,
    BLACK
};
int main(){
    slow_json::Buffer buffer{1000};
    slow_json::dumps(buffer,RED,4);
    std::cout<<buffer<<std::endl;
}
```

这段代码执行结果为

```text
RED
```
## 静态JSON字典类（slow_json::static_dict）

对于JSON中，多数情况下其实每一个`key`是具有不同类型的`value`，那么这个时候采用`std::map`等方法就不够用了。
因此本库中提供了`slow_json::static_dict`类型，借助`std::pair`，可以像`Python`的`dict`那样去快速构建一个`JSON`字符串

```cpp
int main(){
    slow_json::Buffer buffer(1000);
    constexpr slow_json::static_dict dict{
            std::pair{"test", 123},
            std::pair{"name", "不知道取什么名字好"},
            std::pair{"tuple", slow_json::static_dict{
                    std::pair{"haha", "wawa"},
                    std::pair{"single", "boy"}
            }}
    };
    slow_json::dumps(buffer,dict,4);
    std::cout<<buffer<<std::endl;
    buffer.clear();
}
```

该代码运行得到的结果为

```text
{
    "test":123,
    "name":"不知道取什么名字好",
    "tuple":{
        "haha":"wawa",
        "single":"boy"
    }
}
```

对于`slow_json::static_dict`，如果`key`采用`slow_json::static_string`，则可以非常微弱的支持编译期静态访问数据(
感觉好像用处不大)

```cpp
using namespace slow_json::static_string_literals; //为了支持_ss后缀，用来获取编译期静态字符串
int main(){
    slow_json::Buffer buffer(1000);
    constexpr slow_json::static_dict dict{
            std::pair{"test"_ss, 123},
            std::pair{"name"_ss, "ABC"},
            std::pair{"tuple"_ss, slow_json::static_dict{
                    std::pair{"haha"_ss, "wawa"},
                    std::pair{"single"_ss, "boy"}
            }}
    };
    constexpr auto value=dict["name"_ss];
    constexpr auto value2=dict["tuple"_ss]["haha"_ss];
    std::cout<<value<<" "<<value2<<std::endl;
}
```

该代码的输出为

```text
ABC wawa
```

## 添加自定义类型的支持（侵入式）

很多时候其实我们更加喜欢将自己定义的C++类型转化为`JSON`字符串，同时又能将`JSON`字符串转化为C++对象，可是C++并不支持序列化，也不支持反射

不过对于本库来说，想要实现将C++自定义类转化为JSON其实非常方便，尤其是在已经支持将`STL`容器和容器适配器转化为`JSON`字符串的情况

例如如下的`Node`类型

```cpp
struct Node{
    int x=1;
    std::vector<float> y={1.2,3.4};
    std::string z="STR";
};
```

如果想让其可以通过本库进行序列化，只需要实现一个如下面代码中的`get_config`函数即可

```cpp
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
int main(){
    Node node;
    slow_json::Buffer buffer(1000);
    slow_json::dumps(buffer,node);
    std::cout<<buffer<<std::endl;
}
```

该代码的输出结果为

```text
{"x":1,"y":[1.2,3.4],"z":"STR"}
```

该函数依然是可以嵌套使用的，例如

```cpp
struct Node;

struct NodeList{
    Node nodes[3];
    static constexpr auto get_config()noexcept{
        return slow_json::static_dict{
            std::pair{"nodes"_ss,&NodeList::nodes}
        };
    }
};
int main(){
    slow_json::Buffer buffer(1000);
    NodeList node_list;
    slow_json::dumps(buffer,node_list);
    std::cout<<buffer<<std::endl;
}
```

## 添加自定义类型的支持（非侵入式）

很多类已经写好了，尤其是一些第三方库的类，例如一开始的例子`cv::Mat`，我们不可能去修改`OpenCV`源码，因此这种情况下，可以考虑非侵入式的方法来提供支持

具体来说，是实现一个偏特化子类`struct DumpToString<T>:public IDumpToString<DumpToString<T>>`
的静态方法`static void dump_impl(Buffer&buffer,const T&value)noexcept;`

对于`Opencv`的`cv::Mat`，可以如下进行实现，只需要将其定义在使用`slow_json::dumps`之前即可：

```cpp
namespace slow_json {
    template<>
    struct DumpToString<cv::Mat> : public IDumpToString<DumpToString<cv::Mat>> {
        static void dump_impl(Buffer &buffer, const cv::Mat &value) noexcept {
            std::vector<std::vector<int>>vec; //将cv::Mat转化为已知的可以处理的类型，然后调用对应类型的偏特化类的静态方法即可
            for(int i=0;i<value.cols;i++){
                std::vector<int>line;
                for(int j=0;j<value.rows;j++){
                    line.emplace_back(value.at<int>(i,j));
                }
                vec.emplace_back(std::move(line));
            }
            // slow_json::dumps(buffer,vec);
            DumpToString<decltype(vec)>::dump(buffer,vec);
        }
    };
}
```

这样`slow_json`便可以处理`cv::Mat`类型了

```cpp
struct ImageMerger{
    std::optional<int> x=100,y=120,w=1000,h=2000;
    cv::Mat transform_mat= (cv::Mat_<int>(3, 3) << 1, 2, 3, 4, 5, 6, 7, 8, 9);
    static constexpr auto get_config()noexcept{
        return slow_json::static_dict{
            std::pair{"x"_ss,&ImageMerger::x},
            std::pair{"y"_ss,&ImageMerger::y},
            std::pair{"w"_ss,&ImageMerger::w},
            std::pair{"h"_ss,&ImageMerger::h},
            std::pair{"transform_mat"_ss,&ImageMerger::transform_mat}
        };
    }
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

# 将字符串序列化为JSON

## 处理不支持的类型

同样对于不支持的类型，在`debug`模式下会产生一个`assertion`断言失败的错误信息

```cpp
int main(){
    std::string json_str=R"([[1,2,3],[4,5,6],[7,8,9]])";
    cv::Mat mat;
    slow_json::loads(mat,json_str);
    std::cout<<mat<<std::endl;
}
```

该代码的运行输出如下

```text
程序断言失败,程序退出
断言表达式:SLOW_JSON_SUPPORTED=false
文件:/project/石峻峰-实验性项目/SlowJson重构/slowjson/load_from_dict_interface.hpp
行数:23
函数名称:static void slow_json::LoadFromDict<T>::load_impl(T&, const slow_json::dynamic_dict&) [with T = cv::Mat]
断言错误消息:无法将JSON字段转化为类型为'cv::Mat'的对象正确对象，找不到对应的LoadFromDict偏特化类
terminate called without an active exception
```

## 枚举变量

同样的，`slowjson`也支持将字符串还原为对应的`enum`类型的变量

```cpp
#include "slowjson.hpp"
#include<iostream>
enum Color {
    RED,
    GREEN,
    BLUE,
    BLACK
};

int main(){
    Color color2;
    std::cout<<color2<<std::endl;
    slow_json::loads(color2,"\"BLUE\"");
    std::cout<<color2<<std::endl;
    std::cout<<(color2==BLUE?"true":"false")<<std::endl;
}

```

该代码执行得到的结果为

```text
22066
2
true
```
## C++基本类型、STL容器和容器适配器

- 对于C++的基本类型，采用`slow_json::loads`可以完成所有的解析通过

- `slow_json::loads`也支持STL容器和容器适配器，以及复杂的组合类型，如`std::unordered_map<std::string,std::vector<int>>`

- 注意，对于指针数据的处理（如字符串指针），需要手动管理内存，需要手动delete数据

```cpp
#include "slowjson.hpp"
#include <unordered_set>
#include <iostream>

int main() {
    {
        int x;
        slow_json::loads(x, "123");
        std::cout << x << std::endl;
        double y;
        slow_json::loads(y, "123.456789");
        printf("%.6f\n", y);
        std::string str;
        slow_json::loads(str, "\"hahahah\"");
        std::cout << str << std::endl;

        const char *ptr = nullptr;
        slow_json::loads(ptr, "\"shijunfeng\""); //注意，指针需要手动释放，否则会有内存泄漏问题，其实不推荐这么做
        std::cout << "data2:" << ptr << std::endl;
        delete ptr;
    }
    {
        slow_json::dynamic_dict dict("\"null\"");
        auto result = dict.cast<std::optional<std::string>>();
        if (result) {
            std::cout << result.value() << std::endl;
        } else {
            std::cout << "空数据" << std::endl;
        }
    }
    {
        slow_json::dynamic_dict dict("[1,2,3,4,5,6,7]");
        int fuck[7];
        //std::vector<int>fuck;
        dict.fit(fuck);
        for(auto&it:fuck){
            std::cout<<it<<" ";
        }
        std::cout<<std::endl;
    }
    {
        slow_json::dynamic_dict dict("[1,2,3,4,5,6,7]");
        std::deque<int>fuck;
        dict.fit(fuck);
        for(auto&it:fuck){
            std::cout<<it<<" ";
        }
        std::cout<<std::endl;
    }
    {
        slow_json::dynamic_dict dict("[1,2,3,4,5,6,7]");
        std::unordered_set<int>fuck;
        dict.fit(fuck);
        for(auto&it:fuck){
            std::cout<<it<<" ";
        }
        std::cout<<std::endl;
    }
    {
        std::string json_str=R"({
        "x":[4],
        "y":[1],
        "z":[2,3,4,5,6]
    })";
        std::unordered_map<std::string,std::vector<int>>fuck;
        slow_json::loads(fuck,json_str);
        for(auto&it:fuck){
            std::cout<<it.first<<" "<<it.second.back()<<" "<<it.second.size()<<std::endl;
        }
        std::cout<<std::endl;
    }
}
```

这段代码的输出结果为

```text
123
123.456789
hahahah
data2:shijunfeng
null
1 2 3 4 5 6 7 
1 2 3 4 5 6 7 
7 6 5 4 3 2 1 
z 6 5
y 1 1
x 4 1
```

## 动态JSON解析类（slow_json::dynamic_dict）

本库提供了`slow_json::dynamic_dict`，可以用来解析动态JSON。
相比于`slow_json::static_dict`，`slow_json::dynamic_dict`提供了更加方便的动态JSON访问能力

```cpp
int main() {
    std::string json_str=R"({
        "x":[4],
        "y":[1],
        "z":[2,3,4,5,6],
        "t":null,
        "object":{
            "name":"zhihu"
        }
    })";
    slow_json::dynamic_dict dict(json_str);
    std::cout<<dict["object"]["name"].cast<std::string>()<<std::endl; //直接访问数据
    std::cout<<dict["t"].empty()<<std::endl;                          //是否为null
    std::cout<<dict["z"].size()<<std::endl;                           //获取数组大小（如果是一个数组的话）
}
```

该代码的输出结果为

```cpp
zhihu
1
5
```

## 添加自定义类型的支持（侵入式）

参考关于JSON生成中的自定义类型，实际上当写下那个`get_config`函数后，不仅可以实现对于自定义类型的JSON生成，还可以从JSON反序列化对象

```cpp

struct Node{
int x;
float y;
std::string z;
static constexpr auto get_config()noexcept{
return slow_json::static_dict{
std::pair{
"x"_ss, &Node::x
},
std::pair{
"y"_ss, &Node::y
},
std::pair{
"z"_ss, &Node::z}
};
}
};

int main(){

std::vector<Node> p;
std::string json_str=R"([{
        "x":4,
        "y":1.2,
        "z":"strings"
    },{
        "x":41,
        "y":12.23,
        "z":"STR"
    }])";
slow_json::loads(p, json_str);
std::cout<<p.front().x<<" "<<p.front().y<<" "<<p.front().z<<std::endl;
std::cout<<p.back().x<<" "<<p.back().y<<" "<<p.back().z<<std::endl;
}
```

这段代码运行得到的结果为

```text
4 1.2 strings
41 12.23 STR
```

## 添加自定义类型的支持（非侵入式）

同样的，对于JSON反序列化，该库也支持非侵入是的，还是以`opencv`的`cv::Mat`为例，
实现一个类似的偏特化子类`struct LoadFromDict<T>::ILoadFromDict<LoadFromDict<T>>`
的静态成员函数`static void load_impl(T&value,const slow_json::dynamic_dict&dict)`

```cpp
namespace slow_json {
    template<>
    struct LoadFromDict<cv::Mat>:public ILoadFromDict<LoadFromDict<cv::Mat>>{
        static void load_impl(cv::Mat&value,const slow_json::dynamic_dict&dict){
            value=cv::Mat(3,3,CV_8UC1);
            for(int i=0;i<dict.size();i++){
                for(int j=0;j<dict[i].size();j++){
                    value.at<uint8_t>(i,j)=dict[i][j].cast<int32_t>();
                }
            }
        }
    };
}
```

这样编写好代码，放在调用`slow_json::loads`之前的地方，这样本库便可以支持`cv::Mat`的反序列化

```cpp
int main(){
    std::string json_str=R"([[1,2,3],[4,5,6],[7,8,9]])";
    cv::Mat mat;
    slow_json::loads(mat,json_str);
    std::cout<<mat<<std::endl;
}
```

该代码的输出结果为

```text
[  1,   2,   3;
   4,   5,   6;
   7,   8,   9]
```

# 面向对象的支持

这部分其实是为了兼容上一套代码（这套代码是重构之后的代码），但是确实也是一个不错的方案，具体而言就是继承`slow_json::ISerializable`
，并实现`get_config`和`from_config`两个方法。
通常用于序列化和反序列化并不对等的情况，例如成元属性为`std::unordered_map<int,std::string>`
，但是又希望对应的JSON为`list[str]`，又不知道把偏特化的`DumpToString`和`LoadFromDict`写在哪里，那不如直接和类写成一起。具体用法可以参考如下的实现代码：

```cpp
#include "slowjson.hpp"
#include <iostream>
struct Data:public slow_json::ISerializable{
    int x=1;
    float y=1.2;
    std::string z="sjf";
    slow_json::polymorphic_dict get_config()const noexcept override{
        return slow_json::polymorphic_dict{
            std::pair{"x",x},
            std::pair{"y",y},
            std::pair{"z",z}
        };
    }
    void from_config(const slow_json::dynamic_dict&data)noexcept override{
        x=data["x"].cast<int>();
        y=data["y"].cast<float>();
        z=data["z"].cast<std::string>();
    }
};

int main(){
    Data data;
    Data data2;
    data.x=123;
    data.y=345.678;
    data.z="haha";
    slow_json::Buffer buffer{100};
    slow_json::dumps(buffer,data);
    std::cout<<buffer<<std::endl;
    slow_json::loads(data2,buffer.string());
    std::cout<<data2.x<<std::endl;
    std::cout<<data2.y<<std::endl;
    std::cout<<data2.z<<std::endl;
}
```

这段代码运行得到的结果为

```text
{"x":123,"y":345.678,"z":"haha"}
123
345.678
haha
```

# 补充信息

## 多态JSON字典类型（slow_json::polymorphic_dict）

`slow_json::static_dict<Args...>`是一个模板类型，无法事先确定类型，因此`get_config`的返回值类型只能是`auto`
但是这对于一些喜欢头文件和实现分离的人来说，并不是很友好。
因此本库中还提供了`slow_json::polymorphic_dict`，该类型并不是一个模板类型，故可以如下编写代码以分离声明和实现到对应的hpp和cpp中

- 这个类型仅仅只是用于`JSON`序列化中（目前只能用于在不需要反序列化的场景下）用来替待`slow_json::static_dict`。

- 这个类型只能用于提供生成和解析JSON所需要的一些信息，无法去访问具体的数据，哪怕是编译期确定的数据

- **由于采用了`std::function`，因此可能会带来一些额外的的性能损失**

```cpp
struct Node{
    int xxx=1;
    float yyy=1.2345;
    std::string zzz="shijunfeng";
    std::deque<std::string>dq{"a","b","c","d"};
    static slow_json::polymorphic_dict get_config()noexcept;
};

slow_json::polymorphic_dict Node::get_config() noexcept {
    return slow_json::polymorphic_dict{
            std::pair{"xxx"_ss, &Node::xxx},
            std::pair{"yyy"_ss, &Node::yyy},
            std::pair{"zzz"_ss, &Node::zzz},
            std::pair{"dq"_ss,&Node::dq}
    };
}
int main(){
    Node p;
    slow_json::Buffer buffer;
    slow_json::dumps(buffer,p,4);
    std::cout<<buffer<<std::endl;
}
```

## 派生类的JSON处理

采用`slowjson::inherit`可以很方便的进行处理，并同时支持派生类的序列化与反序列化

```cpp
struct Node{
    int xxx=1;
    float yyy=1.2345;
    std::string zzz="shijunfeng";
    std::deque<std::string>dq{"a","b","c","d"};
    static constexpr auto get_config()noexcept{
        return slow_json::static_dict{
                std::pair{"xxx"_ss, &Node::xxx},
                std::pair{"yyy"_ss, &Node::yyy},
                std::pair{"zzz"_ss, &Node::zzz},
                std::pair{"dq"_ss,&Node::dq}
        };
    }
};

struct Node2:public Node{
    int hahaha=2333;
    static constexpr auto get_config()noexcept{

        return slow_json::inherit<Node>(slow_json::static_dict{
                std::pair{"hahaha"_ss, &Node2::hahaha}
        });
    }
};

int main(){
    Node2 p;
    slow_json::Buffer buffer{1000};
    slow_json::dumps(buffer,p,4);
    std::cout<<buffer<<std::endl;
    slow_json::static_dict d1{std::pair{"a","b"}};
    slow_json::static_dict d2{std::pair{"c","d"}};
    slow_json::merge(d1,slow_json::static_dict{std::pair{"a","b"}});
}
```

代码执行结果为：

```text
{
    "xxx":1,
    "yyy":1.2345,
    "zzz":"shijunfeng",
    "test":{
        "value":123.456
    },
    "dq":[
        "a",
        "b",
        "c",
        "d"
    ],
    "hahaha":2333
}

```