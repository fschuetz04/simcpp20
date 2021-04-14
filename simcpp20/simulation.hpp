// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <functional>
#include <queue>
#include <vector>

#include "scheduled_event.hpp"
#include "types.hpp"

namespace simcpp20 {
class event;

/**
 * Used to run a discrete-event simulation.
 *
 * To create a new instance, default-initialize the class:
 *
 *     simcpp20::simulation sim;
 */
class simulation {
public:
  /**
   * Create a pending event which will be processed after the given delay.
   *
   * @tparam T Event class. Must be a subclass of simcpp20::event.
   * @param delay Delay after which the event will be processed.
   * @return Created event.
   */
  template <std::derived_from<simcpp20::event> T = simcpp20::event>
  T timeout(simtime delay) {
    auto ev = event<T>();
    schedule(delay, ev);
    return ev;
  }

  /**
   * Create a pending event.
   *
   * @tparam T Event class. Must be a subclass of simcpp20::event.
   * @return Created event.
   */
  template <std::derived_from<simcpp20::event> T = simcpp20::event> T event() {
    return simcpp20::event{*this};
  }

  /**
   * Create a pending event which is triggered when any of the given events is
   * processed.
   *
   * If any of the given events is already processed or the list of given events
   * is empty, the created event is immediately triggered.
   *
   * @param evs List of events.
   * @return Created event.
   */
  simcpp20::event any_of(std::vector<simcpp20::event> evs);

  /**
   * Create a pending event which is triggered when all of the given events are
   * processed.
   *
   * If all of the given events are already processed or the list of given
   * events if empty, the created event is immediately triggered.
   *
   * @param evs List of events.
   * @return Created event.
   */
  simcpp20::event all_of(std::vector<simcpp20::event> evs);

  /**
   * Extract the next scheduled event from the event queue and process it.
   *
   * @throw When the event queue is empty.
   */
  void step();

  /**
   * Run the simulation until the event queue is empty.
   */
  void run();

  /**
   * Run the simulation until the next scheduled event is scheduled at or after
   * the given target time.
   *
   * @param target Target time.
   */
  void run_until(simtime target);

  /// @return Current simulation time.
  simtime now();

  /// @return Whether the event queue is empty.
  bool empty();

private:
  /// Current simulation time.
  simtime now_ = 0;

  /// Event queue.
  std::priority_queue<scheduled_event, std::vector<scheduled_event>,
                      std::greater<scheduled_event>>
      scheduled_evs = {};

  /// Next ID for scheduling an event.
  id_type next_id = 0;

  /**
   * Schedule the given event to be processed after the given delay.
   *
   * @param delay Delay after which to process the event.
   * @param ev Event to be processed.
   */
  void schedule(simtime delay, simcpp20::event ev);

  // event needs access to simulation::schedule
  friend class event;
};

} // namespace simcpp20
