cmake_minimum_required(VERSION 3.16)

project(dlio-perf LANGUAGES CXX)

# 只有当前项目是主项目时，才会执行以下操作
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_EXTENSIONS OFF)

endif ()

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

option(BUILD_TESTS "build test case" ON)

find_package(Threads REQUIRED)

add_subdirectory(third_party)

add_subdirectory(src)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()

add_subdirectory(apps)