//
// Created by hyzh on 2025/2/28.
// 提供宏支持，在类中使用简化代码编写
//

#ifndef SLOWJSON_MACRO_HPP
#define SLOWJSON_MACRO_HPP

#define SJ_CONCAT_(a, b) a##b
#define SJ_CONCAT(a, b) SJ_CONCAT_(a, b)


#define SJ_ARG_COUNT(...) SJ_ARG_COUNT_IMPL( \
    __VA_ARGS__,                             \
    30,29,28,27,26,25,24,23,22,21,         \
    20,19,18,17,16,15,14,13,12,11,         \
    10,9,8,7,6,5,4,3,2,1,0)

#define SJ_ARG_COUNT_IMPL(                   \
    _1,_2,_3,_4,_5,_6,_7,_8,_9,_10,         \
    _11,_12,_13,_14,_15,_16,_17,_18,_19,_20,\
    _21,_22,_23,_24,_25,_26,_27,_28,_29,_30,\
    N,...) N


#define SJ_FOREACH(macro, arg, ...) \
  SJ_CONCAT(SJ_FOREACH_, SJ_ARG_COUNT(__VA_ARGS__))(macro, arg, __VA_ARGS__)


#define SJ_FOREACH_1(m, a, x) m(a, x)
#define SJ_FOREACH_2(m, a, x, ...) m(a, x) SJ_FOREACH_1(m, a, __VA_ARGS__)
#define SJ_FOREACH_3(m, a, x, ...) m(a, x) SJ_FOREACH_2(m, a, __VA_ARGS__)
#define SJ_FOREACH_4(m, a, x, ...) m(a, x) SJ_FOREACH_3(m, a, __VA_ARGS__)
#define SJ_FOREACH_5(m, a, x, ...) m(a, x) SJ_FOREACH_4(m, a, __VA_ARGS__)
#define SJ_FOREACH_6(m, a, x, ...) m(a, x) SJ_FOREACH_5(m, a, __VA_ARGS__)
#define SJ_FOREACH_7(m, a, x, ...) m(a, x) SJ_FOREACH_6(m, a, __VA_ARGS__)
#define SJ_FOREACH_8(m, a, x, ...) m(a, x) SJ_FOREACH_7(m, a, __VA_ARGS__)
#define SJ_FOREACH_9(m, a, x, ...) m(a, x) SJ_FOREACH_8(m, a, __VA_ARGS__)
#define SJ_FOREACH_10(m, a, x, ...) m(a, x) SJ_FOREACH_9(m, a, __VA_ARGS__)
#define SJ_FOREACH_11(m, a, x, ...) m(a, x) SJ_FOREACH_10(m, a, __VA_ARGS__)
#define SJ_FOREACH_12(m, a, x, ...) m(a, x) SJ_FOREACH_11(m, a, __VA_ARGS__)
#define SJ_FOREACH_13(m, a, x, ...) m(a, x) SJ_FOREACH_12(m, a, __VA_ARGS__)
#define SJ_FOREACH_14(m, a, x, ...) m(a, x) SJ_FOREACH_13(m, a, __VA_ARGS__)
#define SJ_FOREACH_15(m, a, x, ...) m(a, x) SJ_FOREACH_14(m, a, __VA_ARGS__)
#define SJ_FOREACH_16(m, a, x, ...) m(a, x) SJ_FOREACH_15(m, a, __VA_ARGS__)
#define SJ_FOREACH_17(m, a, x, ...) m(a, x) SJ_FOREACH_16(m, a, __VA_ARGS__)
#define SJ_FOREACH_18(m, a, x, ...) m(a, x) SJ_FOREACH_17(m, a, __VA_ARGS__)
#define SJ_FOREACH_19(m, a, x, ...) m(a, x) SJ_FOREACH_18(m, a, __VA_ARGS__)
#define SJ_FOREACH_20(m, a, x, ...) m(a, x) SJ_FOREACH_19(m, a, __VA_ARGS__)
#define SJ_FOREACH_21(m, a, x, ...) m(a, x) SJ_FOREACH_20(m, a, __VA_ARGS__)
#define SJ_FOREACH_22(m, a, x, ...) m(a, x) SJ_FOREACH_21(m, a, __VA_ARGS__)
#define SJ_FOREACH_23(m, a, x, ...) m(a, x) SJ_FOREACH_22(m, a, __VA_ARGS__)
#define SJ_FOREACH_24(m, a, x, ...) m(a, x) SJ_FOREACH_23(m, a, __VA_ARGS__)
#define SJ_FOREACH_25(m, a, x, ...) m(a, x) SJ_FOREACH_24(m, a, __VA_ARGS__)
#define SJ_FOREACH_26(m, a, x, ...) m(a, x) SJ_FOREACH_25(m, a, __VA_ARGS__)
#define SJ_FOREACH_27(m, a, x, ...) m(a, x) SJ_FOREACH_26(m, a, __VA_ARGS__)
#define SJ_FOREACH_28(m, a, x, ...) m(a, x) SJ_FOREACH_27(m, a, __VA_ARGS__)
#define SJ_FOREACH_29(m, a, x, ...) m(a, x) SJ_FOREACH_28(m, a, __VA_ARGS__)
#define SJ_FOREACH_30(m, a, x, ...) m(a, x) SJ_FOREACH_29(m, a, __VA_ARGS__)

#define PROCESS_FIELD(arg, elem) \
   std::pair{#elem##_ss,&__this::elem},


#define $$config(supers,class_name,...)                               \
/**                                                              \
 * 通过这个函数，使得slowJSON支持将该对象序列化为JSON，或从JSON进行反序列化 \
 * @return 一个可序列化的对象                                        \
 */                                                               \
static auto get_config()noexcept{                                 \
    using namespace slow_json::static_string_literals;            \
    using __this=class_name;                                      \
    return slow_json::inherit supers(slow_json::static_dict{      \
        SJ_FOREACH(PROCESS_FIELD, 0, ##__VA_ARGS__)               \
    });                                                            \
}

#define $$config_decl(supers,class_name,...)                               \
/**                                                              \
 * 通过这个函数，使得slowJSON支持将该对象序列化为JSON，或从JSON进行反序列化 \
 * @return 一个可序列化的对象                                        \
 */                                                               \
static decltype([](){                                         \
    using namespace slow_json::static_string_literals;            \
    using __this=class_name;                                      \
    return slow_json::inherit supers(slow_json::static_dict{      \
        SJ_FOREACH(PROCESS_FIELD, 0, ##__VA_ARGS__)               \
    });                                                      \
}()) get_config()noexcept;


#define $$config_impl(supers,class_name,...)                               \
/**                                                              \
 * 通过这个函数，使得slowJSON支持将该对象序列化为JSON，或从JSON进行反序列化 \
 * @return 一个可序列化的对象                                        \
 */                                                               \
decltype([](){                                         \
    using namespace slow_json::static_string_literals;            \
    using __this=class_name;                                      \
    return  slow_json::inherit supers(slow_json::static_dict{      \
        SJ_FOREACH(PROCESS_FIELD, 0, ##__VA_ARGS__)               \
    });                                                       \
}()) class_name::get_config()noexcept{                                 \
    using namespace slow_json::static_string_literals;            \
    using __this=class_name;                                      \
    return slow_json::inherit supers(slow_json::static_dict{      \
        SJ_FOREACH(PROCESS_FIELD, 0, ##__VA_ARGS__)               \
    });                                                            \
}

#define $config(class_name,...) $$config(<>,class_name,##__VA_ARGS__)
#define $config_decl(class_name,...) $$config_decl(<>,class_name,##__VA_ARGS__)
#define $config_impl(class_name,...) $$config_impl(<>,class_name,##__VA_ARGS__)

#endif //SLOWJSON_MACRO_HPP
