add_library(fschuetz04_simcpp20 INTERFACE)
add_library(fschuetz04::simcpp20 ALIAS fschuetz04_simcpp20)

target_include_directories(fschuetz04_simcpp20 INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_compile_features(fschuetz04_simcpp20 INTERFACE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10")
    message(WARNING "SimCpp20 requires GCC 10 or later")
  endif()
  target_compile_options(fschuetz04_simcpp20 INTERFACE -fcoroutines)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.28")
    message(WARNING "SimCpp20 requires MSVC 19.28 or later")
  endif()
endif()
