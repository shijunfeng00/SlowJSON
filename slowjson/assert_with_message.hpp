/**
 * @author shijunfeng
 * @date 2024/7/12
 * @details 实现一个运行时断言，并在断言失败的时候输出一个失败原因和失败的位置
 *          仅仅在DEBUG模式起效，非DEBUG模式不会开启
 */

#ifndef SLOWJSON_ASSERT_WITH_MESSAGE_HPP
#define SLOWJSON_ASSERT_WITH_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>

#ifdef NDEBUG
#define assert_with_message(...)
#else
#define assert_with_message(expr, message, ...) {                                                                                                                      \
       if(!(expr)){                                                                                                                                                  \
           fprintf(stderr,"断言失败,程序退出\n断言表达式:(%s) 的值为 %s\n文件:%s\n行数:%d\n函数名称:%s\n断言错误消息:",#expr,expr?"true":"false",__FILE__,__LINE__,__PRETTY_FUNCTION__);\
           fprintf(stderr,message"\n",##__VA_ARGS__);                                                                                                                    \
           std::terminate();                                                                                                                                                  \
       }                                                                                                                                                             \
}
#endif
#endif //SLOWJSON_ASSERT_WITH_MESSAGE_HPP
