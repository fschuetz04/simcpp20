// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

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
