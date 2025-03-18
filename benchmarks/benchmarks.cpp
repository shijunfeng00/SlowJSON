//
// Created by hyzh on 2025/3/18.
//

#include "benchmarks.hpp"
int main(){
    benchmark_simple();
    benchmark_nested();
    benchmark_simple_serialization_deserialization();
    benchmark_nested_serialization_deserialization();
}