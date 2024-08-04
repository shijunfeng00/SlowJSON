//
// Created by hy-20 on 2024/7/12.
//

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
           fprintf(stderr,"程序断言失败,程序被迫结束\n断言表达式:(%s)=%s\n文件:%s\n行数:%d\n函数完整签名:%s\n断言错误消息:",#expr,expr?"true":"false",__FILE__,__LINE__,__PRETTY_FUNCTION__);\
           fprintf(stderr,message"\n",##__VA_ARGS__);                                                                                                                    \
           std::terminate();                                                                                                                                                  \
       }                                                                                                                                                             \
}
#endif
#endif //SLOWJSON_ASSERT_WITH_MESSAGE_HPP
