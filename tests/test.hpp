//
// Created by hy-20 on 2024/8/12.
//

#ifndef SLOWJSON_RUN_TEST_HPP
#define SLOWJSON_RUN_TEST_HPP

/**
 * slow_json::Buffer
 */
void test_buffer();

/**
 * 基于slow_json::loads和slow_json::dynamic_dict的反序列化
 */
void test_deserialization();

/**
 * 基于slow_json::loads和slow_json::dumps的序列化反序列化枚举型变量
 */
void test_enum_serialization_deserialization();

/**
 * 基于slow_json::loads和slow_json::static_dict对自定义类（派生类）进行反序列化
 */
void test_static_inherit_deserialization();

/**
 * 基于slow_json::dumps和slow_json::static_dict对自定义类（派生类）进行序列化
 */
void test_static_inherit_serialization();

/**
 * 测试合并两个dict，包括slow_json::static_dict和slow_json::polymorphic_dict
 */
void test_merge_dict();

/**
 * 采用非侵入式接口实现自定义类型的序列化、反序列化
 */
void test_non_intrusive_serialization_deserialization();

/**
 * 基于slow_json::polymorphic的反序列化
 */
void test_polymorphic_deserialization();

/**
 * 采用slow_json::dumps来序列化slow_json::static_dict
 */
void test_static_dict_dumps();

/**
 * 采用slow_json::dumps来序列化slow_json::polymorphic_dict
 */
void test_polymorphic_dict_dumps();

/**
 * 测试slow_json::polymorphic对自定义类进行序列化
 */
void test_polymorphic_serialization();

/**
 * 成员变量是C风格数组的情况
 */
void test_polymorphic_field_array();

/**
 * 测试采用面向对象，继承ISerializable的方式来使得slow_json支持自定义类
 */
void test_serializable_oop();

/**
 * 测试嵌套序列化（链表）
 */
void test_serialization_nested();

/**
 * 测试对于STL容器和容器适配器的序列化支持
 */
void test_stl_dumps();

/**
 * 测试对于STL容器和容器适配器的反序列化支持
 */
void test_stl_loads();

/**
 * 测试浮点数的序列化
 */
void test_floating_point_serialization();

/**
 * 测试整数的序列化
 */
void test_integral_serialization();

/**
 * 测试对象禁用拷贝构造函数的情况下能否正确使用
 */
void test_non_copy_constructible();


#endif //SLOWJSON_RUN_TEST_HPP
