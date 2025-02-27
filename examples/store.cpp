#include <cstdio>

#include "fschuetz04/simcpp20.hpp"
#include "fschuetz04/simcpp20/process.hpp"

simcpp20::process<> producer(simcpp20::simulation<> &sim,
                             simcpp20::store<int> &store) {
  for (int i = 0; i < 5; ++i) {
    co_await store.put(i);
    printf("[%2.0f] store <- %d\n", sim.now(), i);
  }
}

simcpp20::process<> consumer(simcpp20::simulation<> &sim,
                             simcpp20::store<int> &store) {
  for (int i = 0; i < 5; ++i) {
    co_await sim.timeout(5);
    auto value = co_await store.get();
    printf("[%2.0f] store -> %d\n", sim.now(), value);
  }
}

int main() {
  simcpp20::simulation<> sim;
  simcpp20::store<int> store{sim, 1};
  producer(sim, store);
  consumer(sim, store);
  sim.run();
}
