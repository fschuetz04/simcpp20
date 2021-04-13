// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <memory>
#include <queue>

namespace simcpp20 {
class event;
class scheduled_event;

using id_type = uint64_t;
using simtime = double;

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
  std::shared_ptr<T> timeout(simtime delay) {
    auto ev = event<T>();
    ev->trigger_delayed(delay);
    return ev;
  }

  /**
   * Create a pending event.
   *
   * @tparam T Event class. Must be a subclass of simcpp20::event.
   * @return Created event.
   */
  template <std::derived_from<simcpp20::event> T = simcpp20::event>
  std::shared_ptr<T> event() {
    return std::make_shared<T>(*this);
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
  std::shared_ptr<simcpp20::event>
  any_of(std::initializer_list<std::shared_ptr<simcpp20::event>> evs);

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
  std::shared_ptr<simcpp20::event>
  all_of(std::initializer_list<std::shared_ptr<simcpp20::event>> evs);

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

  /// ID of the next scheduled event.
  id_type next_id_ = 0;

  /**
   * Schedule the given event to be processed after the given delay.
   *
   * @param delay Delay after which to process the event.
   * @param ev Event to be processed.
   */
  void schedule(simtime delay, std::shared_ptr<simcpp20::event> ev);

  // event needs access to simulation::schedule
  friend class event;
};

} // namespace simcpp20
