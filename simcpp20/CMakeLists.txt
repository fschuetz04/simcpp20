add_library(simcpp20
    await_event.cpp
    event.cpp
    process.cpp
    scheduled_event.cpp
    simulation.cpp)
target_include_directories(simcpp20 PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_compile_options(simcpp20 PUBLIC -fcoroutines -std=c++20)
