// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

// See https://simpy.readthedocs.io/en/latest/examples/machine_shop.html

#include "resource.hpp"
#include "simcpp20.hpp"

#include <cstdio>
#include <random>

struct config {
  double repair_time;
  resource repair_man;
  std::normal_distribution<> time_for_part_dist;
  std::exponential_distribution<> time_to_failure_dist;
  std::default_random_engine gen;
};

class machine {
public:
  machine(simcpp20::simulation<> &sim, config &conf)
      : sim{sim}, conf{conf}, failure{sim.event()} {
    produce();
    fail();
  }

  int n_parts_made = 0;
  simcpp20::simulation<> &sim;

private:
  simcpp20::event<> produce() {
    while (true) {
      double time_for_part = conf.time_for_part_dist(conf.gen);

      while (true) {
        double start = sim.now();
        auto timeout = sim.timeout(time_for_part);
        co_await(timeout | failure);

        if (timeout.triggered()) {
          // part is finished
          ++n_parts_made;
          break;
        }

        // machine failed, calculate remaining time for part and wait for repair
        time_for_part -= sim.now() - start;
        co_await conf.repair_man.request();
        co_await sim.timeout(conf.repair_time);
        conf.repair_man.release();
      }
    }
  }

  simcpp20::event<> fail() {
    while (true) {
      co_await sim.timeout(conf.time_to_failure_dist(conf.gen));
      failure.trigger();
      failure = sim.event();
    }
  }

  config &conf;
  simcpp20::event<> failure;
};

int main() {
  simcpp20::simulation<> sim;

  std::random_device rd;
  config conf{
      .repair_time = 30,
      .repair_man = resource{sim, 1},
      .time_for_part_dist = std::normal_distribution<>{10, 2},
      .time_to_failure_dist = std::exponential_distribution<>{1. / 300},
      .gen = std::default_random_engine{rd()},
  };

  int n_machines = 10;
  std::vector<machine> machines;
  machines.reserve(n_machines);
  for (int i = 0; i < n_machines; ++i) {
    machines.emplace_back(sim, conf);
  }

  int n_weeks = 4;
  sim.run_until(n_weeks * 7 * 24 * 60);

  printf("Machine shop results after %d weeks:\n", n_weeks);
  for (int i = 0; i < n_machines; ++i) {
    printf("- Machine %d made %d parts\n", i, machines[i].n_parts_made);
  }
}
