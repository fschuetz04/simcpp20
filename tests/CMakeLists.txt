include(FetchContent)

FetchContent_Declare(Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG        b9853b4b356b83bb580c746c3a1f11101f9af54f) # v3.0.0-preview3
FetchContent_MakeAvailable(Catch2)

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE
  simcpp20
  Catch2::Catch2WithMain)

if(MSVC)
  target_compile_options(tests PRIVATE /W4)
else()
  target_compile_options(tests PRIVATE -Wall -Wextra)
endif()

add_test(tests tests)
