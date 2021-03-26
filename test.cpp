#include <coroutine>
#include <iostream>

#include "simcpp2.hpp"

simcpp2::process clock1(simcpp2::simulation &sim) {
  while (true) {
    std::cout << "clock1: " << sim.now() << std::endl;
    co_await sim.timeout(1);
  }
}

simcpp2::process clock2(simcpp2::simulation &sim) {
  while (true) {
    std::cout << "clock2: " << sim.now() << std::endl;
    co_await sim.timeout(2);
  }
}

int main() {
  simcpp2::simulation sim;
  clock1(sim);
  clock2(sim);
  sim.run_until(10);
}
