// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include <cstdio>

#include "simcpp20.hpp"

simcpp20::event consumer(simcpp20::simulation &sim,
                         simcpp20::value_event<int> ev) {
  auto val = co_await ev;
  printf("[%.0f] val = %d\n", sim.now(), val);
}

int main() {
  simcpp20::simulation sim;
  auto ev = sim.timeout<int>(1, 42);
  consumer(sim, ev);
  sim.run();
}
