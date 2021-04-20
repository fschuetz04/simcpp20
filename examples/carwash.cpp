// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

// See https://simpy.readthedocs.io/en/latest/examples/carwash.html

#include <cstdio>
#include <random>

#include "resource.hpp"
#include "simcpp20.hpp"

struct config {
  int initial_cars;
  double wash_time;
  resource machines;
  std::default_random_engine gen;
  std::uniform_real_distribution<double> arrival_time_dist;
};

simcpp20::event wash(simcpp20::simulation &sim, config &conf, int id) {
  co_await sim.timeout(conf.wash_time);
  printf("[%5.1f] Car %d washed\n", sim.now(), id);
}

simcpp20::event car(simcpp20::simulation &sim, config &conf, int id) {
  printf("[%5.1f] Car %d arrives\n", sim.now(), id);

  auto request = conf.machines.request();
  co_await request;

  printf("[%5.1f] Car %d enters\n", sim.now(), id);

  co_await wash(sim, conf, id);

  printf("[%5.1f] Car %d leaves\n", sim.now(), id);
  conf.machines.release();
}

simcpp20::event car_source(simcpp20::simulation &sim, config &conf) {
  for (int id = 1;; ++id) {
    if (id > conf.initial_cars) {
      co_await sim.timeout(conf.arrival_time_dist(conf.gen));
    }

    car(sim, conf, id);
  }
}

int main() {
  simcpp20::simulation sim;

  std::random_device rd;
  config conf{
      .initial_cars = 4,
      .wash_time = 5,
      .machines = resource{sim, 2},
      .gen = std::default_random_engine{rd()},
      .arrival_time_dist = std::uniform_real_distribution<double>{3, 7},
  };

  car_source(sim, conf);

  sim.run_until(20);
}
