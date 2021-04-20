# TODO include simcpp20 in subfolder, move target into namespace

cmake_minimum_required(VERSION 3.14)

if(DEFINED PROJECT_NAME)
    set(SimCpp20IsSubproject ON)
else()
    set(SimCpp20IsSubproject OFF)
endif()

project(SimCpp20
    VERSION      0.1.0
    DESCRIPTION  "Discrete-event simulation in C++20 using coroutines"
    HOMEPAGE_URL "https://github.com/fschuetz04/simcpp20"
    LANGUAGES    CXX)

if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "10"))
    message(FATAL_ERROR "SimCpp20 requires a g++ with version 10 or greater")
endif()

add_subdirectory(simcpp20)

if(NOT SimCpp20IsSubproject)
    add_subdirectory(examples)
    add_subdirectory(tests)
endif()
