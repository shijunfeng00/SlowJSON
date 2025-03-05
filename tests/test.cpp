//
// Created by hy-20 on 2024/8/12.
//
#include "test.hpp"
#include <iostream>

int main() {
    printf("Start running test unit.\n");
#ifdef NDEBUG
    printf("Running on release mode.");
#else
    printf("running on debug mode.");
#endif
    printf("\n--------------------\n");
    test_buffer();
    test_deserialization();
    test_enum_serialization_deserialization();
    test_static_inherit_deserialization();
    test_static_inherit_serialization();
    test_merge_dict();
    test_non_intrusive_serialization_deserialization();
    test_polymorphic_deserialization();
    test_static_dict_dumps();
    test_polymorphic_dict_dumps();
    test_polymorphic_serialization();
    test_polymorphic_field_array();
    test_serializable_oop();
    test_serialization_nested();
    test_stl_dumps();
    test_stl_loads();
    test_floating_point_serialization();
    test_integral_serialization();
    test_non_copy_constructible();
    test_pair_tuple_deserialization();
    test_function();
    test_list_c_ptr_serialization_deserialization();
    test_list_shared_ptr_serialization_deserialization();
    test_binary_search_tree();
    test_macro_serialization();
    printf("\n--------------------\n");
    printf("All done correctly!\n");
}