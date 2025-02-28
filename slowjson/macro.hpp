//
// Created by hyzh on 2025/2/28.
// 提供宏支持，在类中使用简化代码编写
//

#ifndef SLOWJSON_MACRO_HPP
#define SLOWJSON_MACRO_HPP

#define SJ_CONCAT_(a, b) a##b
#define SJ_CONCAT(a, b) SJ_CONCAT_(a, b)


#define SJ_ARG_COUNT(...) SJ_ARG_COUNT_IMPL(__VA_ARGS__, 5,4,3,2,1,0)
#define SJ_ARG_COUNT_IMPL(_1,_2,_3,_4,_5,N,...) N


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

#define $polymorphic(class_name,...)                     \
/**                                                                \
 * 通过这个函数，使得slowJSON支持将该对象序列化为JSON，或从JSON进行反序列化   \
 * @return 一个可序列化的对象                                         \
 */                                                                \
static slow_json::polymorphic_dict get_config()noexcept{           \
    using namespace slow_json::static_string_literals;             \
    using __this=class_name;                                       \
    return slow_json::polymorphic_dict{                            \
        SJ_FOREACH(PROCESS_FIELD, 0, ##__VA_ARGS__)                \
    };                                                             \
}

#define $static(class_name,...)                               \
/**                                                              \
 * 通过这个函数，使得slowJSON支持将该对象序列化为JSON，或从JSON进行反序列化 \
 * @return 一个可序列化的对象                                        \
 */                                                               \
static auto get_config()noexcept{                                 \
    using namespace slow_json::static_string_literals;            \
    using __this=class_name;                                      \
    return slow_json::static_dict{                                \
        SJ_FOREACH(PROCESS_FIELD, 0, ##__VA_ARGS__)               \
    };                                                            \
}

#define $polymorphic_decl(class_name) static slow_json::polymorphic_dict get_config()noexcept;
#define $polymorphic_impl(class_name,...)  \
/**                                                                 \
 * 通过这个函数，使得slowJSON支持将该对象序列化为JSON，或从JSON进行反序列化    \
 * @return 一个可序列化的对象                                           \
 */                                                                  \
slow_json::polymorphic_dict class_name::get_config()noexcept{ \
    using namespace slow_json::static_string_literals;               \
    using __this=class_name;                                         \
    return slow_json::polymorphic_dict{                              \
        SJ_FOREACH(PROCESS_FIELD, 0, ##__VA_ARGS__)                  \
    };                                                               \
}
#endif //SLOWJSON_MACRO_HPP
