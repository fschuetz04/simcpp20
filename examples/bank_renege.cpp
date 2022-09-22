// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

// See https://simpy.readthedocs.io/en/latest/examples/bank_renege.html

#include <cstdio>
#include <random>

#include "fschuetz04/simcpp20.hpp"
#include "resource.hpp"

struct config {
  int n_customers;
  resource counters;
  std::uniform_real_distribution<> max_wait_time_dist;
  std::exponential_distribution<> arrival_interval_dist;
  std::exponential_distribution<> service_time_dist;
  std::default_random_engine gen;
};

simcpp20::event<> customer(simcpp20::simulation<> &sim, config &conf, int id) {
  printf("[%5.1f] Customer %d arrives\n", sim.now(), id);

  auto request = conf.counters.request();
  auto max_wait_time = conf.max_wait_time_dist(conf.gen);
  co_await (request | sim.timeout(max_wait_time));

  if (!request.triggered()) {
    request.abort();
    printf("[%5.1f] Customer %d RENEGES\n", sim.now(), id);
    co_return;
  }

  printf("[%5.1f] Customer %d gets to the counter\n", sim.now(), id);

  co_await sim.timeout(conf.service_time_dist(conf.gen));

  printf("[%5.1f] Customer %d leaves\n", sim.now(), id);
  conf.counters.release();
}

simcpp20::event<> customer_source(simcpp20::simulation<> &sim, config &conf) {
  for (int id = 1; id <= conf.n_customers; ++id) {
    customer(sim, conf, id);
    co_await sim.timeout(conf.arrival_interval_dist(conf.gen));
  }
}

int main() {
  simcpp20::simulation<> sim;

  std::random_device rd;
  config conf{
      .n_customers = 5,
      .counters = resource{sim, 1},
      .max_wait_time_dist = std::uniform_real_distribution<>{1., 3.},
      .arrival_interval_dist = std::exponential_distribution<>{1. / 10},
      .service_time_dist = std::exponential_distribution<>{1. / 12},
      .gen = std::default_random_engine{rd()},
  };

  customer_source(sim, conf);

  sim.run();
}
