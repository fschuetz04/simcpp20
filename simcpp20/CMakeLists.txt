add_library(simcpp20 INTERFACE)

target_include_directories(simcpp20 INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_compile_options(simcpp20 INTERFACE -fcoroutines -std=c++20)
