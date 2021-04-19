// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "simulation.hpp"

#include <cassert> // assert
#include <memory>  // std::make_shared

namespace simcpp20 {
event_alias simulation::event() {
  return event_alias{*this};
}

event_alias simulation::timeout(time_type delay) {
  auto ev = event();
  schedule(ev, delay);
  return ev;
}

event_alias simulation::any_of(std::vector<event_alias> evs) {
  for (auto &ev : evs) {
    if (ev.processed()) {
      return timeout(0);
    }
  }

  auto any_of_ev = event();

  for (auto &ev : evs) {
    ev.add_callback([any_of_ev](auto) mutable { any_of_ev.trigger(); });
  }

  return any_of_ev;
}

event_alias simulation::all_of(std::vector<event_alias> evs) {
  int n = evs.size();

  for (auto &ev : evs) {
    if (ev.processed()) {
      --n;
    }
  }

  if (n == 0) {
    return timeout(0);
  }

  auto all_of_ev = event();
  auto n_ptr = std::make_shared<int>(n);

  for (auto &ev : evs) {
    ev.add_callback([all_of_ev, n_ptr](auto) mutable {
      --*n_ptr;
      if (*n_ptr == 0) {
        all_of_ev.trigger();
      }
    });
  }

  return all_of_ev;
}

void simulation::schedule(event_alias ev, time_type delay /* = 0 */) {
  if (delay < 0) {
    assert(false);
    return;
  }

  scheduled_evs.emplace(now() + delay, next_id, ev);
  ++next_id;
}

void simulation::step() {
  auto scheduled_ev = scheduled_evs.top();
  scheduled_evs.pop();
  now_ = scheduled_ev.time();
  scheduled_ev.ev().process();
}

void simulation::run() {
  while (!empty()) {
    step();
  }
}

void simulation::run_until(time_type target) {
  if (target < now()) {
    assert(false);
    return;
  }

  while (!empty() && scheduled_evs.top().time() < target) {
    step();
  }

  now_ = target;
}

bool simulation::empty() {
  return scheduled_evs.empty();
}

time_type simulation::now() {
  return now_;
}

simulation::scheduled_event::scheduled_event(time_type time, id_type id,
                                             event_alias ev)
    : time_{time}, id{id}, ev_{ev} {}

bool simulation::scheduled_event::operator>(
    const scheduled_event &other) const {
  if (time() != other.time()) {
    return time() > other.time();
  }

  return id > other.id;
}

time_type simulation::scheduled_event::time() const {
  return time_;
}

event_alias simulation::scheduled_event::ev() {
  return ev_;
}
} // namespace simcpp20
