include(FetchContent)

FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG        6e79e682b726f524310d55dec8ddac4e9c52fb5f) # v3.4.0
FetchContent_MakeAvailable(Catch2)

add_executable(tests tests.cpp resource_tests.cpp store_tests.cpp)
target_link_libraries(tests PRIVATE
  fschuetz04::simcpp20
  Catch2::Catch2WithMain)
target_compile_options(tests PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)
