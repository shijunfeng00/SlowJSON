/**
 * @file assert_with_message.hpp
 * @brief 可配置的运行时断言宏，支持异常抛出或程序终止
 * @author shijunfeng
 * @date 2024/7/12
 * 
 * ======================== 宏开关行为说明 ========================
 * 断言行为由两个关键宏控制：
 * 1. NDEBUG       - 定义表示Release模式，未定义表示Debug模式
 * 2. SLOW_JSON_ASSERT_AS_EXCEPTION - 控制断言失败时是否抛出异常
 * 
 * ┌──────────────────────┬──────────────────────┬──────────────────────┐
 * │        模式          │ SLOW_JSON_ASSERT_AS  │       行为描述       │
 * │                      │ _EXCEPTION状态       │                      │
 * ├──────────────────────┼──────────────────────┼──────────────────────┤
 * │                      │      开启            │ 抛出包含详细信息的   │
 * │   Debug              │ (默认关闭)           │ std::runtime_error  │
 * │ (未定义NDEBUG)       ├──────────────────────┼──────────────────────┤
 * │                      │      关闭            │ 打印错误信息到stderr│
 * │                      │ (默认)              │ 并调用std::terminate│
 * ├──────────────────────┼──────────────────────┼──────────────────────┤
 * │                      │      开启            │ 抛出包含详细信息的   │
 * │   Release            │ (默认关闭)           │ std::runtime_error  │
 * │ (定义NDEBUG)         ├──────────────────────┼──────────────────────┤
 * │                      │      关闭            │ 断言完全被移除       │
 * │                      │ (默认)              │ (无任何运行时开销)   │
 * └──────────────────────┴──────────────────────┴──────────────────────┘
 * 
 * 特殊模式：
 * - 测试模式(BUILD_TEST_UNIT):
 *   强制开启 SLOW_JSON_ASSERT_AS_EXCEPTION，
 *   断言失败时始终抛出异常
 * 
 * 使用示例：
 *   assert_with_message(ptr != nullptr, "空指针异常，预期值:%d", 42);
 * 
 * 输出示例：
 *   断言失败,程序退出
 *   断言表达式:(ptr != nullptr) 的值为 false
 *   文件:example.cpp
 *   行数:25
 *   函数名称:main
 *   断言错误消息:空指针异常，预期值:42
 */

#ifndef SLOWJSON_ASSERT_WITH_MESSAGE_HPP
#define SLOWJSON_ASSERT_WITH_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>

#ifdef BUILD_TEST_UNIT //测试模式下，把断言变为异常
#define SLOW_JSON_NOEXCEPT
#define SLOW_JSON_ASSERT_AS_EXCEPTION
#else
#ifdef SLOW_JSON_ASSERT_AS_EXCEPTION
#define SLOW_JSON_NOEXCEPT
#else
#define SLOW_JSON_NOEXCEPT noexcept
#endif
#endif


#ifdef NDEBUG
#ifdef  SLOW_JSON_ASSERT_AS_EXCEPTION
#define assert_with_message(expr, message, ...) {                                                                                                                      \
       if(!(expr)){                                                                                                                                                  \
           char str[1000]={0};                                                                                                                                          \
           memset(str,0,sizeof(str));\
           snprintf(str,1000,"断言失败,程序退出\n断言表达式:(%s) 的值为 %s\n文件:%s\n行数:%d\n函数名称:%s\n断言错误消息:"#message"\n",#expr,expr?"true":"false",__FILE__,__LINE__,__PRETTY_FUNCTION__,##__VA_ARGS__);           \
           throw std::runtime_error(str);                                                                                                                                                  \
       }     
#define SLOW_JSON_NOEXCEPT
#else
#define assert_with_message(...)
#endif
#else
#ifdef SLOW_JSON_ASSERT_AS_EXCEPTION
#define assert_with_message(expr, message, ...) {                                                                                                                      \
       if(!(expr)){                                                                                                                                                  \
           char str[1000]={0};                                                                                                                                          \
           memset(str,0,sizeof(str));\
           snprintf(str,1000,"断言失败,程序退出\n断言表达式:(%s) 的值为 %s\n文件:%s\n行数:%d\n函数名称:%s\n断言错误消息:"#message"\n",#expr,expr?"true":"false",__FILE__,__LINE__,__PRETTY_FUNCTION__,##__VA_ARGS__);           \
           throw std::runtime_error(str);                                                                                                                                                  \
       }                                                                                                                                                                 \
}
#else
#define assert_with_message(expr, message, ...) {                                                                                                                      \
       if(!(expr)){                                                                                                                                                  \
           fprintf(stderr,"断言失败,程序退出\n断言表达式:(%s) 的值为 %s\n文件:%s\n行数:%d\n函数名称:%s\n断言错误消息:",#expr,expr?"true":"false",__FILE__,__LINE__,__PRETTY_FUNCTION__);\
           fprintf(stderr,message"\n",##__VA_ARGS__);                                                                                                                    \
           std::terminate();                                                                                                                                                  \
       }                                                                                                                                                             \
}
#endif
#endif
#endif //SLOWJSON_ASSERT_WITH_MESSAGE_HPP
