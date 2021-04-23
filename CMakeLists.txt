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

add_subdirectory(simcpp20)

# TODO add option to enable tests
if(NOT SimCpp20IsSubproject)
  enable_testing()
  add_subdirectory(tests)
  add_subdirectory(examples)
endif()
