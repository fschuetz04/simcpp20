# SimCpp20

SimCpp20 is a discrete-event simulation framework for C++20.
It is similar to SimPy and aims to be easy to set up and use.

Processes are defined as functions receiving `simcpp20::simulation &` as their first argument and returning `simcpp20::process`.
Each process is executed as a coroutine.
Thus, this framework requires C++20.
To compile a simulation, use `g++ -Wall -std=c++20 -fcoroutines example.cpp simcpp20.cpp -o example`.
For this to work, `g++` must be on version 10 (you can try `g++-10` too).
A short example simulating two clocks ticking in different time intervals looks like this:

```c++
#include <coroutine>
#include <iostream>

#include "simcpp20.hpp"

simcpp20::process clock_proc(simcpp20::simulation &sim, std::string name,
                            double delay) {
  while (true) {
    std::cout << name << " " << sim.now() << std::endl;
    co_await sim.timeout(delay);
  }
}

int main() {
  simcpp20::simulation sim;
  clock_proc(sim, "fast", 1);
  clock_proc(sim, "slow", 2);
  sim.run_until(10);
}
```

## Copyright and License

Copyright © 2021 Felix Schütz.

Licensed under the MIT License.
See the LICENSE file for details.