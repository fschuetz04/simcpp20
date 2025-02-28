#include <cstdio>

#include "fschuetz04/simcpp20.hpp"

simcpp20::process<> producer(simcpp20::simulation<> &sim, int id,
                             double delay) {
  printf("[%.0f] Producer %d starting\n", sim.now(), id);
  co_await sim.timeout(delay);
  printf("[%.0f] Producer %d finished\n", sim.now(), id);
}

simcpp20::process<> consumer(simcpp20::simulation<> &sim) {
  printf("[%.0f] Consumer starting\n", sim.now());

  // Create two producer processes
  auto p1 = producer(sim, 1, 5);
  auto p2 = producer(sim, 2, 10);

  // Wait for any producer to finish
  co_await (p1 | p2);
  printf("[%.0f] First producer finished\n", sim.now());

  // Wait for all producers to finish
  co_await (p1 & p2);
  printf("[%.0f] All producers finished\n", sim.now());
}

int main() {
  simcpp20::simulation<> sim;
  consumer(sim);
  sim.run();
}
