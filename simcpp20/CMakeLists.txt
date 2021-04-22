add_library(simcpp20
    event.cpp
    simulation.cpp)

target_include_directories(simcpp20 PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_compile_features(simcpp20 PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10")
        message(FATAL_ERROR "SimCpp20 requires g++ 10 or later")
    endif()
    target_compile_options(simcpp20 PUBLIC -fcoroutines)
endif()
