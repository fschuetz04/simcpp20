include(FetchContent)

OPTION(BUILD_TESTS OFF)
FetchContent_Declare(units
  GIT_REPOSITORY https://github.com/nholthaus/units
  GIT_TAG        1e2c7ed7cd8e61db7a82b8827caef7ed49b01831)
FetchContent_MakeAvailable(units)

set(TARGETS
  all_of
  any_of
  bank_renege
  carwash
  clocks
  clocks_units
  machine_shop
  ping_pong
  store
  value_event
  value_process)

foreach(TARGET ${TARGETS})
  add_executable(${TARGET} ${TARGET}.cpp)
  target_link_libraries(${TARGET} PRIVATE fschuetz04::simcpp20)
  target_compile_options(${TARGET} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
  )
endforeach()

target_link_libraries(clocks_units PRIVATE units)
