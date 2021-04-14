// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include <random>

#include "resource.hpp"
#include "simcpp20.hpp"

constexpr int n_counters = 1;
constexpr int n_customers = 10;
constexpr double mean_arrival_interval = 10;
constexpr double max_wait_time = 16;
constexpr double mean_service_time = 12;

std::random_device rd;
std::default_random_engine gen{rd()};
std::exponential_distribution<double> exp_dist{};

simcpp20::process customer(simcpp20::simulation &sim, int id,
                           resource &counters) {
  printf("[%5.1f] Customer %d arrives\n", sim.now(), id);

  auto request = counters.request();
  auto timeout = sim.timeout(max_wait_time);
  // internal compiler when directly awaiting return value of sim.any_of
  auto any_of = sim.any_of({request, timeout});
  co_await any_of;

  if (!request.triggered()) {
    request.abort();
    printf("[%5.1f] Customer %d leaves unhappy\n", sim.now(), id);
  }

  printf("[%5.1f] Customer %d gets to the counter\n", sim.now(), id);

  auto service_time = exp_dist(gen) * mean_service_time;
  co_await sim.timeout(service_time);

  printf("[%5.1f] Customer %d gets to the counter\n", sim.now(), id);
  counters.release();
}

simcpp20::process customer_source(simcpp20::simulation &sim) {
  resource counters{sim, n_counters};
  std::vector<simcpp20::process> customers{};

  for (int id = 1; id <= n_customers; ++id) {
    customers.push_back(customer(sim, id, counters));

    auto arrival_interval = exp_dist(gen) * mean_arrival_interval;
    co_await sim.timeout(arrival_interval);
  }

  for (auto &customer : customers) {
    co_await customer;
  }

  printf("[%5.1f] Customer source finished\n", sim.now());
}

int main() {
  simcpp20::simulation sim;
  customer_source(sim);
  sim.run();
}
