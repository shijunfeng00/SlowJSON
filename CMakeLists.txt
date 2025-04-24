cmake_minimum_required(VERSION 3.16)
project(SlowJson)

# 定义 BUILD_TEST 选项，默认关闭
option(BUILD_TEST "Build test units" OFF)

# Compiler flags
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug模式下不开启O3优化，并生成调试信息
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++2a -g -fconcepts -Wall -Wno-unused-function -Wno-unused-variable -Wno-deprecated-declarations -Wno-sign-compare -Wno-deprecated -Werror=return-local-addr -Werror=return-type")
    message("g++ using Debug mode: ${CMAKE_CXX_FLAGS}")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    # Release模式关闭调试信息，并采用O3优化
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++2a -fconcepts -Wall -Wno-unused-function -Wno-unused-variable -Wno-deprecated-declarations -Wno-sign-compare -Wno-deprecated -Werror=return-local-addr -Werror=return-type")
    message("g++ using Release mode: ${CMAKE_CXX_FLAGS}")
endif ()

include_directories(
        "slowjson"
        "3rd_party/rapidjson"
)

set(CMAKE_CXX_STANDARD 20)

if (BUILD_TEST)
    message("编译单元测试代码")
    file(GLOB TEST_CPP "tests/*.cpp")
    add_executable(SlowJson
            ${TEST_CPP}
    )
    # 为测试目标定义 BUILD_TEST_UNIT 宏
    target_compile_definitions(SlowJson PRIVATE BUILD_TEST_UNIT)

elseif(BUILD_BENCHMARK)
    message("编译基准测试代码")
    file(GLOB BENCHMARK_CPP "benchmarks/*.cpp")
    add_executable(SlowJson
            ${BENCHMARK_CPP}
    )

else ()
    add_executable(SlowJson main.cpp)
endif ()

target_link_libraries(SlowJson)