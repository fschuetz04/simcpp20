cmake_minimum_required(VERSION 3.10)

project(simcpp20)

if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "10"))
    message(FATAL_ERROR "SimCpp20 requires a g++ with version 10 or greater")
endif()

add_subdirectory(simcpp20)
add_subdirectory(examples)
