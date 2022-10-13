// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <cassert>    // assert
#include <cstddef>    // std::size_t
#include <cstdint>    // std::uint64_t
#include <functional> // std::greater
#include <iostream>
#include <memory>     // std::make_shared, std::make_unique
#include <queue>      // std::priority_queue
#include <utility>    // std::forward
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
 * TODO(fschuetz04): Keep list of pending events to clear them up?
 *
 * @tparam Time Type used for simulation time.
 */
template <typename Time = double> class simulation {
private:
  using event_type = simcpp20::event<Time>;
  std::vector<event_type> pending_events = std::vector<event_type>();

public:
  /// @return New pending event.
  event_type event() { return pending_events.emplace_back(*this); }

  /**
   * @tparam Value Value type of the event.
   * @return New pending value event.
   */
  template <typename Value> value_event<Value, Time> event() {
    return value_event<Value, Time>{*this};
  }

  /**
   * @param delay Delay after which to process the event.
   * @return New pending event.
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
   * @return New pending value event.
   */
  template <typename Value, typename... Args>
  value_event<Value, Time> timeout(Time delay, Args &&...args) {
    auto ev = event<Value>();
    ev.set_value(std::forward<Args>(args)...);
    schedule(ev, delay);
    return ev;
  }

  /**
   * @param evs List of events.
   * @return New pending event which is triggered when any of the given events
   * is processed.
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
   * @param evs List of events.
   * @return New pending event which is triggered when all of the given events
   * are processed.
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
   * @param ev Event to be processed.
   * @param delay Delay after which to process the event.
   */
  void schedule(event_type ev, Time delay = Time{0}) {
    assert(delay >= Time{0});

    scheduled_evs_.emplace(now() + delay, next_id_, ev);
    ++next_id_;
  }

  /// Process the next scheduled event.
  void step() {
    auto sev = scheduled_evs_.top();
    scheduled_evs_.pop();
    now_ = sev.time_;
    sev.ev_.process();
  }

  /// Run the simulation until no more events are scheduled.
  void run() {
    while (!empty()) {
      step();
    }
  }

  /**
   * Run the simulation until the target time is reached or no more events are
   * scheduled. The target time is reached when the next scheduled event is
   * scheduled at or after the target time.
   *
   * @param target Target time.
   */
  void run_until(Time target) {
    assert(target >= now());

    while (!empty() && scheduled_evs_.top().time_ < target) {
      step();
    }

    now_ = target;
  }

  /// @return Whether no events are scheduled.
  bool empty() const { return scheduled_evs_.empty(); }

  /// @return Current simulation time.
  Time now() const { return now_; }

  /**
   * Clear up all events awaiting by aborting them to 
   * destroy the coroutine frame and free its resources.
   */  
  ~simulation<Time>() {
    while (!empty()) {
      auto sev = scheduled_evs_.top();
      scheduled_evs_.pop();
      sev.ev_.abort();
    }

    while (!pending_events.empty()) {
      event_type pev = pending_events.back();
      pending_events.pop_back();
      pev.abort();
    }
    pending_events.clear();
  }



private:
  /// One event scheduled to be processed.
  class scheduled_event {
  public:
    /**
     * Constructor.
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
      if (time_ != other.time_) {
        return time_ > other.time_;
      }

      return id_ > other.id_;
    }

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

  /// Scheduled events.
  std::priority_queue<scheduled_event, std::vector<scheduled_event>,
                      std::greater<scheduled_event>>
      scheduled_evs_{};

  /// Current simulation time.
  Time now_ = Time{0};

  /// Next ID for scheduling an event.
  id_type next_id_ = 0;
};
} // namespace simcpp20
