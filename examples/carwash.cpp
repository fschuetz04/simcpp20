// See https://simpy.readthedocs.io/en/latest/examples/carwash.html

#include <cstdio>
#include <random>

#include "fschuetz04/simcpp20.hpp"

struct config {
  int initial_cars;
  double wash_time;
  simcpp20::resource<> machines;
  std::uniform_int_distribution<> arrival_time_dist;
  std::default_random_engine gen;
};

simcpp20::process<> wash(simcpp20::simulation<> &sim, config &conf, int id) {
  co_await sim.timeout(conf.wash_time);
  printf("[%4.1f] Car %d washed\n", sim.now(), id);
}

simcpp20::process<> car(simcpp20::simulation<> &sim, config &conf, int id) {
  printf("[%4.1f] Car %d arrives\n", sim.now(), id);

  auto request = conf.machines.request();
  co_await request;

  printf("[%4.1f] Car %d enters\n", sim.now(), id);

  co_await wash(sim, conf, id);

  printf("[%4.1f] Car %d leaves\n", sim.now(), id);
  conf.machines.release();
}

simcpp20::process<> car_source(simcpp20::simulation<> &sim, config &conf) {
  for (int id = 1;; ++id) {
    if (id > conf.initial_cars) {
      co_await sim.timeout(conf.arrival_time_dist(conf.gen));
    }

    car(sim, conf, id);
  }
}

int main() {
  simcpp20::simulation<> sim;

  std::random_device rd;
  config conf{
      .initial_cars = 4,
      .wash_time = 5,
      .machines = simcpp20::resource{sim, 2},
      .arrival_time_dist = std::uniform_int_distribution<>{3, 7},
      .gen = std::default_random_engine{rd()},
  };

  car_source(sim, conf);

  sim.run_until(20);
}
