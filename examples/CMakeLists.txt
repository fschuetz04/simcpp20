include(FetchContent)

OPTION(BUILD_TESTS OFF)
FetchContent_Declare(units
  GIT_REPOSITORY https://github.com/nholthaus/units
  GIT_TAG        1e2c7ed7cd8e61db7a82b8827caef7ed49b01831)
FetchContent_MakeAvailable(units)

set(Targets
  all_of
  any_of
  bank_renege
  carwash
  clocks
  clocks_units
  machine_shop
  ping_pong
  value_event
  value_process)

foreach(Target ${Targets})
  add_executable(${Target} ${Target}.cpp)
  target_link_libraries(${Target} PRIVATE simcpp20)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${Target} PRIVATE -Wall -Wextra)
  endif()
endforeach()

target_link_libraries(clocks_units PRIVATE units)
