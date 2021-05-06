# TODO rename library

add_library(simcpp20 INTERFACE)

target_include_directories(simcpp20 INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_compile_features(simcpp20 INTERFACE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10")
    message(WARNING "SimCpp20 requires GCC 10 or later")
  endif()
  target_compile_options(simcpp20 INTERFACE -fcoroutines)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.28")
    message(WARNING "SimCpp20 requires MSVC 19.28 or later")
  endif()
endif()
