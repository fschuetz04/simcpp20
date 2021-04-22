// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <cassert>    // assert
#include <cstdint>    // uint64_t
#include <functional> // std::greater
#include <memory>     // std::make_shared
#include <queue>      // std::priority_queue
#include <vector>     // std::vector

#include "event.hpp"
#include "value_event.hpp"

namespace simcpp20 {
template <class TTime = double> using event_alias = event<TTime>;
using id_type = uint64_t;

/**
 * Used to run a discrete-event simulation.
 *
 * To create a new instance, default-initialize the class:
 *
 *     simcpp20::simulation<> sim;
 *
 * @tparam TTime Type used for simulation time.
 */
template <class TTime = double> class simulation {
public:
  /// @return Pending event.
  event_alias<TTime> event() { return event_alias<TTime>{*this}; }

  /**
   * @tparam TValue Value type of the event.
   * @return Pending value event.
   */
  template <class TValue> value_event<TValue, TTime> event() {
    return value_event<TValue, TTime>{*this};
  }

  /**
   * @param delay Delay after which to process the event.
   * @return Pending event.
   */
  event_alias<TTime> timeout(TTime delay) {
    auto ev = event();
    schedule(ev, delay);
    return ev;
  }

  /**
   * @tparam TValue Value type of the event.
   * @param delay Delay after which to process the event.
   * @param value Value of the event.
   * @return Pending value event.
   */
  template <class TValue>
  value_event<TValue, TTime> timeout(TTime delay, TValue value) {
    auto ev = event<TValue>();
    *ev.value_ = value;
    schedule(ev, delay);
    return ev;
  }

  /**
   * Create a pending event which is triggered when any of the given events is
   * processed.
   *
   * TODO(fschuetz04): Check behavior.
   *
   * @param evs List of events.
   * @return Created event.
   */
  event_alias<TTime> any_of(std::vector<event_alias<TTime>> evs) {
    for (const auto &ev : evs) {
      if (ev.processed()) {
        return timeout(0);
      }
    }

    auto any_of_ev = event();

    for (const auto &ev : evs) {
      ev.add_callback([any_of_ev](auto) mutable { any_of_ev.trigger(); });
    }

    return any_of_ev;
  }

  /**
   * Create a pending event which is triggered when all of the given events are
   * processed.
   *
   * TODO(fschuetz04): Check behavior.
   *
   * @param evs List of events.
   * @return Created event.
   */
  event_alias<TTime> all_of(std::vector<event_alias<TTime>> evs) {
    int n = evs.size();

    for (const auto &ev : evs) {
      if (ev.processed()) {
        --n;
      }
    }

    if (n == 0) {
      return timeout(0);
    }

    auto all_of_ev = event();
    auto n_ptr = std::make_shared<int>(n);

    for (const auto &ev : evs) {
      ev.add_callback([all_of_ev, n_ptr](auto) mutable {
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
  void schedule(event_alias<TTime> ev, TTime delay = TTime{0}) {
    if (delay < TTime{0}) {
      assert(false);
      return;
    }

    scheduled_evs.emplace(now() + delay, next_id, ev);
    ++next_id;
  }

  /**
   * Extract the next scheduled event from the event queue and process it.
   *
   * @throw When the event queue is empty.
   */
  void step() {
    auto scheduled_ev = scheduled_evs.top();
    scheduled_evs.pop();
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
  void run_until(TTime target) {
    if (target < now()) {
      assert(false);
      return;
    }

    while (!empty() && scheduled_evs.top().time() < target) {
      step();
    }

    now_ = target;
  }

  /// @return Whether the event queue is empty.
  bool empty() const { return scheduled_evs.empty(); }

  /// @return Current simulation time.
  TTime now() const { return now_; }

private:
  /// One event in the event queue.
  class scheduled_event {
  public:
    /**
     * Construct a new scheduled event.
     *
     * @param time Time at which to process the event.
     * @param id Incremental ID to sort events scheduled at the same time by
     * insertion order.
     * @param ev Event to process.
     */
    scheduled_event(TTime time, id_type id, event_alias<TTime> ev)
        : time_{time}, id{id}, ev_{ev} {}

    /**
     * @param other Scheduled event to compare to.
     * @return Whether this event is scheduled before the given event.
     */
    bool operator>(const scheduled_event &other) const {
      if (time() != other.time()) {
        return time() > other.time();
      }

      return id > other.id;
    }

    /// @return Time at which to process the event.
    TTime time() const { return time_; }

    /// @return Event to process.
    event_alias<TTime> ev() const { return ev_; }

  private:
    /// Time at which to process the event.
    TTime time_;

    /**
     * Incremental ID to sort events scheduled at the same time by insertion
     * order.
     */
    id_type id;

    /// Event to process.
    event_alias<TTime> ev_;
  };

  /// Event queue.
  std::priority_queue<scheduled_event, std::vector<scheduled_event>,
                      std::greater<scheduled_event>>
      scheduled_evs{};

  /// Current simulation time.
  TTime now_ = TTime{0};

  /// Next ID for scheduling an event.
  id_type next_id = 0;
};
} // namespace simcpp20
