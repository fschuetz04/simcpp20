cmake_minimum_required(VERSION 3.10)

project(simcpp20)

if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "10"))
    message(FATAL_ERROR "SimCpp20 requires at GCC with version 10 or greater")
endif()

add_subdirectory(simcpp20)
target_include_directories(simcpp20 PUBLIC "${PROJECT_SOURCE_DIR}/simcpp20")
target_compile_options(simcpp20 PUBLIC -std=c++20 -fcoroutines)

add_subdirectory(examples)
