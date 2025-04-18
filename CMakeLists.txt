cmake_minimum_required(VERSION 3.25)
project(SlowJson)

# Compiler flags
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug模式下不开启o3优化，并生成调式信息
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++2a -g -fconcepts -Wall -Wno-unused-function -Wno-unused-variable -Wno-deprecated-declarations -Wno-sign-compare -Wno-deprecated -Werror=return-local-addr -Werror=return-type")
    message("g++ using Debug mode:${CMAKE_CXX_FLAGS}")
else (CMAKE_BUILD_TYPE STREQUAL "Release")
    # Release模式关闭调式信息，并采用O3优化
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++2a -fconcepts -Wall -Wno-unused-function -Wno-unused-variable -Wno-deprecated-declarations -Wno-sign-compare -Wno-deprecated -Werror=return-local-addr -Werror=return-type")
    message("g++ using Release 模式:${CMAKE_CXX_FLAGS}")
endif ()

#set(OpenCV_DIR "/usr/local/opencv-4.5.5/build")
#find_package(OpenCV REQUIRED PATHS "/usr/local/opencv-4.5.5/build" NO_DEFAULT_PATH)


include_directories(
        "slowjson"
        "3rd_party/rapidjson"
        #        ${OpenCV_INCLUDE_DIRS}
)


set(CMAKE_CXX_STANDARD 20)

if (BUILD_TEST)
    message("编译单元测试代码")
    file(GLOB TEST_CPP "tests/*.cpp")
    add_executable(SlowJson
            ${TEST_CPP}
    )

elseif(BUILD_BENCHMARK)
    message("编译基准测试代码")
    file(GLOB BENCHMARK_CPP "benchmarks/*.cpp")
    add_executable(SlowJson
            ${BENCHMARK_CPP}
    )

else ()
    add_executable(SlowJson main.cpp)
endif ()

target_link_libraries(
        SlowJson
)