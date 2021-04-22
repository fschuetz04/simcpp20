add_library(simcpp20
    event.cpp
    simulation.cpp)

target_include_directories(simcpp20 PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_compile_features(simcpp20 PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10")
        message(WARNING "SimCpp20 requires GCC 10 or later")
    endif()
    target_compile_options(simcpp20 PUBLIC -fcoroutines)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.28")
        message(WARNING "SimCpp20 requires MSVC 19.28 or later")
    endif()
endif()
