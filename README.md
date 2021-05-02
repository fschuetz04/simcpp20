# SimCpp20

SimCpp20 is a discrete-event simulation framework for C++20.
It is similar to SimPy and aims to be easy to set up and use.

Processes are defined as functions receiving `simcpp20::simulation<> &` as their first argument and returning `simcpp20::event<>`.
Each process is executed as a coroutine.
Thus, this framework requires C++20.
A short example simulating two clocks ticking in different time intervals looks like this:

```c++
#include <cstdio>

#include "simcpp20.hpp"

simcpp20::event<> clock_proc(simcpp20::simulation<> &sim, char const *name,
                             double delay) {
  while (true) {
    printf("[%.0f] %s\n", sim.now(), name);
    co_await sim.timeout(delay);
  }
}

int main() {
  simcpp20::simulation<> sim;
  clock_proc(sim, "slow", 2);
  clock_proc(sim, "fast", 1);
  sim.run_until(5);
}
```

When run, the following output is generated:

```text
[0] slow
[0] fast
[1] fast
[2] slow
[2] fast
[3] fast
[4] slow
[4] fast
```

Other examples can be found in the `examples/` folder.

This project uses CMake.
To build and execute the clocks example, run the following commands:

```shell
mkdir build
cd build
cmake ..
cmake --build .
examples/clocks
```

The CMake configuration has been tested with GCC and MSVC.
When using a GCC compiler, it must be of version 10 or later.
If such a version is available under a different name (for example `g++-10`), you can try `CXX=g++-10 cmake ..` instead of just `cmake ..` to set the C++ compiler command.
When using an MSVC compiler, it must be of version 19.28 or later (Visual Studio 2019 version 16.8 or later).
Clang probably does not work, since its coroutine implementation is contained in the `std::experimental` namespace.
Contributions to improve compiler support are welcome!

If you want to use SimCpp20 in your project, the easiest option is to use CMake with FetchContent.
A simple configuration looks like this:

```cmake
cmake_minimum_required(VERSION 3.14)

project(MyApp)

include(FetchContent)

FetchContent_Declare(SimCpp20
    GIT_REPOSITORY https://github.com/fschuetz04/simcpp20
    GIT_TAG        14fcc81ed5577b570a067a523914ec06b3527a1f)

FetchContent_MakeAvailable(SimCpp20)

add_executable(app app.cpp)
target_link_libraries(app PRIVATE simcpp20)
```

Replace the commit hash with the latest commit hash of SimCpp20 accordingly.

## Copyright and License

Copyright © 2021 Felix Schütz.

Licensed under the MIT License.
See the `LICENSE` file for details.
