#pragma once

#include <cstdint>
#include <queue>

#include "simulation.hpp"

namespace simcpp20 {

class resource {
public:
  resource(simcpp20::simulation<> &sim, uint64_t available)
      : sim{sim}, available_{available} {}

  simcpp20::event<> request() {
    auto ev = sim.event();
    evs.push(ev);
    trigger_evs();
    return ev;
  }

  void release() {
    ++available_;
    trigger_evs();
  }

  uint64_t available() { return available_; }

private:
  std::queue<simcpp20::event<>> evs{};
  simcpp20::simulation<> &sim;
  uint64_t available_;

  void trigger_evs() {
    while (available() > 0 && evs.size() > 0) {
      auto ev = evs.front();
      evs.pop();
      if (ev.aborted()) {
        continue;
      }

      ev.trigger();
      --available_;
    }
  }
};

} // namespace simcpp20
