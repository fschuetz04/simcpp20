// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include <iostream>
#include <string>

#include "simcpp20.hpp"
#include "units.h"

using namespace units::literals;

using time_type = units::time::second_t;
using event = simcpp20::event<time_type>;
using simulation = simcpp20::simulation<time_type>;

event clock_proc(simulation &sim, char const *name, time_type delay) {
  while (true) {
    std::cout << "[" << sim.now() << "] " << name << std::endl;
    co_await sim.timeout(delay);
  }
}

int main() {
  simulation sim;
  clock_proc(sim, "slow", 2_s);
  clock_proc(sim, "fast", 1_s);
  sim.run_until(5_s);
}
