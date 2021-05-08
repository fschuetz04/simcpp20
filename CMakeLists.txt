cmake_minimum_required(VERSION 3.14)

if(DEFINED PROJECT_NAME)
  set(MAIN_PROJECT OFF)
else()
  set(MAIN_PROJECT ON)
endif()

project(simcpp20
  VERSION      0.1.0
  DESCRIPTION  "Discrete-event simulation in C++20 using coroutines"
  HOMEPAGE_URL "https://github.com/fschuetz04/simcpp20"
  LANGUAGES    CXX)

add_subdirectory(include/fschuetz04/simcpp20)

option(FSCHUETZ04_SIMCPP20_BUILD_TESTS "Build tests" ${MAIN_PROJECT})
if(FSCHUETZ04_SIMCPP20_BUILD_TESTS)
  add_subdirectory(tests)
endif()

option(FSCHUETZ04_SIMCPP20_BUILD_EXAMPLES "Build examples" ${MAIN_PROJECT})
if(FSCHUETZ04_SIMCPP20_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
