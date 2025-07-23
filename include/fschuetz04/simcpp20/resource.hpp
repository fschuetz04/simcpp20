#pragma once

#include <cstdint> // uint64_t
#include <queue>   // std::queue

#include "event.hpp"
#include "simulation.hpp"

namespace simcpp20 {

/**
 * A shared resource holding a number of instances.
 *
 * @tparam Time Type used for simulation time.
 */
template <typename Time = double> class resource {
public:
  /**
   * Constructor.
   *
   * @param sim Reference to the simulation.
   * @param available Number of available instances. Defaults to 0.
   */
  explicit resource(simulation<Time> &sim, uint64_t available = 0)
      : sim_{sim}, available_{available} {}

  /**
   * Request an instance of the resource.
   *
   * @return An event that will be triggered once an instance is available,
   * which may be immediately.
   */
  event<Time> request() {
    auto ev = sim_.event();
    requests_.push(ev);
    trigger_requests();
    return ev;
  }

  /// Release an instance of the resource.
  void release() {
    ++available_;
    trigger_requests();
  }

  /// @return Number of available instance.
  uint64_t available() { return available_; }

private:
  /// Pending request events.
  std::queue<event<Time>> requests_ = {};

  /// Reference to the simulation.
  simulation<Time> &sim_;

  /// Number of available instances.
  uint64_t available_;

  /// Triggger pending request events until the resource is empty.
  void trigger_requests() {
    while (available() > 0 && requests_.size() > 0) {
      auto ev = requests_.front();
      requests_.pop();
      if (ev.aborted()) {
        continue;
      }

      ev.trigger();
      --available_;
    }
  }
};

} // namespace simcpp20
