#include <cstdio>

#include "fschuetz04/simcpp20.hpp"

simcpp20::process<> process(simcpp20::simulation<> &sim) {
  printf("[%.0f] 1\n", sim.now());

  co_await (sim.timeout(1) & sim.timeout(2));
  printf("[%.0f] 2\n", sim.now());

  // sim.event() will never be triggered -> the resulting event will never
  // be triggered too
  co_await (sim.timeout(1) & sim.event());
  printf("[%.0f] 3\n", sim.now());
}

int main() {
  simcpp20::simulation<> sim;
  process(sim);
  sim.run();
}
