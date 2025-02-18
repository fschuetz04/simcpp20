#include <cstdio>

#include "fschuetz04/simcpp20.hpp"

simcpp20::value_event<int> producer(simcpp20::simulation<> &sim) {
  co_await sim.timeout(1);
  co_return 42;
}

simcpp20::event<> consumer(simcpp20::simulation<> &sim) {
  auto val = co_await producer(sim);
  printf("[%.0f] val = %d\n", sim.now(), val);
}

int main() {
  simcpp20::simulation<> sim;
  consumer(sim);
  sim.run();
}
