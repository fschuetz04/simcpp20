// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include <cstdio>

#include "simcpp20.hpp"

simcpp20::event process(simcpp20::simulation &sim) {
  printf("[%.0f] 1\n", sim.now());

  // internal compiler error in some GCC versions when awaiting sim.any_of
  // directly
  auto ev = sim.all_of({sim.timeout(1), sim.timeout(2)});
  co_await ev;
  printf("[%.0f] 2\n", sim.now());

  ev = sim.all_of({sim.timeout(1), sim.event()});
  co_await ev;
  printf("[%.0f] 3\n", sim.now());
}

int main() {
  simcpp20::simulation sim;
  process(sim);
  sim.run();
}
