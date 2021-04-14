// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include <cstdint>
#include <queue>

#include "simcpp20.hpp"

class resource {
public:
  resource(simcpp20::simulation &sim, uint64_t capacity)
      : sim{sim}, cap{capacity} {}

  simcpp20::event request() {
    auto ev = sim.event();
    evs.push(ev);
    trigger_evs();
    return ev;
  }

  void release() {
    ++cap;
    trigger_evs();
  }

private:
  std::queue<simcpp20::event> evs{};
  simcpp20::simulation &sim;
  uint64_t cap;

  void trigger_evs() {
    while (cap > 0 && evs.size() > 0) {
      auto ev = evs.front();
      evs.pop();
      if (ev.aborted()) {
        continue;
      }

      ev.trigger();
      --cap;
    }
  }
};
