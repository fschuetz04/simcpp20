# SimCpp20

SimCpp20 is a discrete-event simulation framework for C++20.
It is similar to SimPy and aims to be easy to set up and use.

Processes are defined as functions receiving `simcpp20::simulation &` as their first argument and returning `simcpp20::event`.
Each process is executed as a coroutine.
Thus, this framework requires C++20.
To compile a simulation, use `g++ -Wall -std=c++20 -fcoroutines example.cpp simcpp20.cpp -o example`.
For this to work, `g++` must be on version 10 (you can try `g++-10` too).
A short example simulating two clocks ticking in different time intervals looks like this:

```c++
#include <cstdio>

#include "simcpp20.hpp"

simcpp20::event clock_proc(simcpp20::simulation &sim, char const *name,
                           double delay) {
  while (true) {
    printf("[%.0f] %s\n", sim.now(), name);
    co_await sim.timeout(delay);
  }
}

int main() {
  simcpp20::simulation sim;
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

This project uses CMake.
To build and execute the example, run the following commands:

```shell
mkdir build
cd build
cmake ..
cmake --build .
examples/clocks
```

Currently, the CMake files only support GCC with a minimum version of 10.
If version 10 of GCC later is available under a different name (for example `g++-10`), you can try `CXX=g++-10 cmake ..` instead of just `cmake ..` to set the C++ compiler command.
If you do not want to use CMake, you can use the GCC directly:

```shell
g++ -std=c++20 -fcoroutines -Isimcpp20 simcpp20/*.cpp examples/clocks.cpp -o example_clocks
./example_clocks
```

Again, you can try using `g++-10` instead.

More examples can be found in the `examples/` folder.

## Copyright and License

Copyright © 2021 Felix Schütz.

Licensed under the MIT License.
See the `LICENSE` file for details.
