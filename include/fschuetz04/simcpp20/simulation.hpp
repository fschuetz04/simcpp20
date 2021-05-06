// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <cassert>    // assert
#include <cstddef>    // std::size_t
#include <cstdint>    // std::uint64_t
#include <functional> // std::greater
#include <memory>     // std::make_shared
#include <queue>      // std::priority_queue
#include <vector>     // std::vector

#include "event.hpp"
#include "value_event.hpp"

namespace simcpp20 {
using id_type = std::uint64_t;

/**
 * Used to run a discrete-event simulation.
 *
 * To create a new instance, default-initialize the class:
 *
 *     simcpp20::simulation<> sim;
 *
 * TODO(fschuetz04): Keep list of pending events to clear them up? Own all
 * events?
 *
 * @tparam Time Type used for simulation time.
 */
template <typename Time = double> class simulation {
private:
  using event_type = simcpp20::event<Time>;

public:
  /// Destroy all coroutines of this simulation.
  ~simulation() {
    // TODO(fschuetz04): Destroy coroutines?
  }

  /// @return Pending event.
  event_type event() { return event_type{*this}; }

  /**
   * @tparam Value Value type of the event.
   * @return Pending value event.
   */
  template <typename Value> value_event<Value, Time> event() {
    return value_event<Value, Time>{*this};
  }

  /**
   * @param delay Delay after which to process the event.
   * @return Pending event.
   */
  event_type timeout(Time delay) {
    auto ev = event();
    schedule(ev, delay);
    return ev;
  }

  /**
   * @tparam Value Value type of the event.
   * @param delay Delay after which to process the event.
   * @param value Value of the event.
   * @return Pending value event.
   */
  template <typename Value>
  value_event<Value, Time> timeout(Time delay, Value value) {
    auto ev = event<Value>();
    *ev.value_ = value;
    schedule(ev, delay);
    return ev;
  }

  /**
   * Create a pending event which is triggered when any of the given events is
   * processed.
   *
   * TODO(fschuetz04): Check behavior. Check whether events are aborted /
   * destroyed?
   *
   * @param evs List of events.
   * @return Created event.
   */
  event_type any_of(std::vector<event_type> evs) {
    if (evs.size() == 0) {
      return timeout(0);
    }

    for (const auto &ev : evs) {
      if (ev.processed()) {
        return timeout(0);
      }
    }

    auto any_of_ev = event();

    for (const auto &ev : evs) {
      ev.add_callback(
          [any_of_ev](const auto &) mutable { any_of_ev.trigger(); });
    }

    return any_of_ev;
  }

  /**
   * Create a pending event which is triggered when all of the given events are
   * processed.
   *
   * TODO(fschuetz04): Check behavior. Check whether events are aborted /
   * destroyed?
   *
   * @param evs List of events.
   * @return Created event.
   */
  event_type all_of(std::vector<event_type> evs) {
    size_t n = evs.size();

    for (const auto &ev : evs) {
      if (ev.processed()) {
        --n;
      }
    }

    if (n == 0) {
      return timeout(0);
    }

    auto all_of_ev = event();
    auto n_ptr = std::make_shared<std::size_t>(n);

    for (const auto &ev : evs) {
      ev.add_callback([all_of_ev, n_ptr](const auto &) mutable {
        --*n_ptr;
        if (*n_ptr == 0) {
          all_of_ev.trigger();
        }
      });
    }

    return all_of_ev;
  }

  /**
   * Schedule the given event to be processed after the given delay.
   *
   * @param delay Delay after which to process the event.
   * @param ev Event to be processed.
   */
  void schedule(event_type ev, Time delay = Time{0}) {
    if (delay < Time{0}) {
      assert(false);
      return;
    }

    scheduled_evs_.emplace(now() + delay, next_id_, ev);
    ++next_id_;
  }

  /**
   * Extract the next scheduled event from the event queue and process it.
   *
   * @throw When the event queue is empty.
   */
  void step() {
    auto scheduled_ev = scheduled_evs_.top();
    scheduled_evs_.pop();
    now_ = scheduled_ev.time();
    scheduled_ev.ev().process();
  }

  /// Run the simulation until the event queue is empty.
  void run() {
    while (!empty()) {
      step();
    }
  }

  /**
   * Run the simulation until the next scheduled event is scheduled at or after
   * the given target time or the event queue is empty.
   *
   * @param target Target time.
   */
  void run_until(Time target) {
    if (target < now()) {
      assert(false);
      return;
    }

    while (!empty() && scheduled_evs_.top().time() < target) {
      step();
    }

    now_ = target;
  }

  /// @return Whether the event queue is empty.
  bool empty() const { return scheduled_evs_.empty(); }

  /// @return Current simulation time.
  Time now() const { return now_; }

private:
  /// One event in the event queue.
  class scheduled_event {
  public:
    /**
     * Construct a new scheduled event.
     *
     * @param time Time at which to process the event.
     * @param id_ Incremental ID to sort events scheduled at the same time by
     * insertion order.
     * @param ev Event to process.
     */
    scheduled_event(Time time, id_type id, event_type ev)
        : time_{time}, id_{id}, ev_{ev} {}

    /**
     * @param other Scheduled event to compare to.
     * @return Whether this event is scheduled before the given event.
     */
    bool operator>(const scheduled_event &other) const {
      if (time() != other.time()) {
        return time() > other.time();
      }

      return id_ > other.id_;
    }

    /// @return Time at which to process the event.
    Time time() const { return time_; }

    /// @return Event to process.
    event_type ev() const { return ev_; }

    /// Time at which to process the event.
    Time time_;

    /**
     * Incremental ID to sort events scheduled at the same time by insertion
     * order.
     */
    id_type id_;

    /// Event to process.
    event_type ev_;
  };

  /// Event queue.
  std::priority_queue<scheduled_event, std::vector<scheduled_event>,
                      std::greater<scheduled_event>>
      scheduled_evs_{};

  /// Current simulation time.
  Time now_ = Time{0};

  /// Next ID for scheduling an event.
  id_type next_id_ = 0;
};
} // namespace simcpp20
