// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "simulation.hpp"

namespace simcpp20 {
std::shared_ptr<simcpp20::event> simulation::any_of(
    std::initializer_list<std::shared_ptr<simcpp20::event>> evs) {
  for (auto &ev : evs) {
    if (ev->processed()) {
      return timeout(0);
    }
  }

  auto any_of_ev = event();

  for (auto &ev : evs) {
    ev->add_callback([any_of_ev](auto) { any_of_ev->trigger(); });
  }

  return any_of_ev;
}

std::shared_ptr<simcpp20::event> simulation::all_of(
    std::initializer_list<std::shared_ptr<simcpp20::event>> evs) {
  int n = evs.size();

  for (auto &ev : evs) {
    if (ev->processed()) {
      --n;
    }
  }

  if (n == 0) {
    return timeout(0);
  }

  auto all_of_ev = event();
  auto n_ptr = std::make_shared<int>(n);

  for (auto &ev : evs) {
    ev->add_callback([all_of_ev, n_ptr](auto) {
      --*n_ptr;
      if (*n_ptr == 0) {
        all_of_ev->trigger();
      }
    });
  }

  return all_of_ev;
}

void simulation::step() {
  auto scheduled_ev = scheduled_evs.top();
  scheduled_evs.pop();

  now_ = scheduled_ev.time();
  scheduled_ev.ev()->process();
}

void simulation::run() {
  while (!empty()) {
    step();
  }
}

void simulation::run_until(simtime target) {
  while (!empty() && scheduled_evs.top().time() < target) {
    step();
  }

  now_ = target;
}

simtime simulation::now() { return now_; }

bool simulation::empty() { return scheduled_evs.empty(); }

void simulation::schedule(simtime delay, std::shared_ptr<simcpp20::event> ev) {
  scheduled_evs.emplace(now() + delay, next_id_, ev);
  next_id_++;
}
} // namespace simcpp20
