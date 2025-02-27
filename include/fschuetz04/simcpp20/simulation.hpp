#pragma once

#include <cassert>    // assert
#include <coroutine>  // std::coroutine_handle
#include <cstddef>    // std::size_t
#include <cstdint>    // std::uint64_t
#include <functional> // std::greater
#include <memory>     // std::make_shared, std::make_unique
#include <queue>      // std::priority_queue
#include <set>        // std::set
#include <utility>    // std::forward
#include <vector>     // std::vector

#include "event.hpp"
#include "process.hpp"
#include "promise_type.hpp"
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
 * @tparam Time Type used for simulation time.
 */
template <typename Time = double> class simulation {
private:
  using event_type = simcpp20::event<Time>;
  using process_type = simcpp20::process<Time>;

public:
  /// Destructor.
  ~simulation() {
    std::set<std::coroutine_handle<>> temp_handles;
    handles_.swap(temp_handles);
    for (auto &handle : temp_handles) {
      handle.destroy();
    }
  }

  /// @return New pending event.
  event_type event() { return event_type{*this}; }

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
  template <typename... Args> event_type any_of(Args &&...evs) {
    return any_of_internal(event(), std::forward<Args>(evs)...);
  }

  /**
   * @tparam Value Value type of the event.
   * @param evs List of events.
   * @return New pending value event which is triggered when any of the given
   * events is processed.
   */
  template <typename Value, typename... Args>
  value_event<Value, Time> any_of(Args &&...evs) {
    return any_of_internal(event<Value>(), std::forward<Args>(evs)...);
  }

  /**
   * @param evs List of events.
   * @return New pending event which is triggered when all of the given events
   * are processed.
   */
  template <typename... Args> event_type all_of(Args &&...evs) {
    auto n_ptr = std::make_shared<std::size_t>(0);
    return all_of_internal(event(), n_ptr, std::forward<Args>(evs)...);
  }

  /**
   * @param ev Event to be processed.
   * @param delay Delay after which to process the event.
   */
  void schedule(const event_type &ev, Time delay = Time{0}) {
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

private:
  /**
   * Consumes one event for `any_of` and forwards the remaining events.
   *
   * @tparam Event Type of the current event.
   * @tparam Args Type of the remaining events.
   * @param any_of_ev Event to trigger when any one of the events is processed.
   * @param current_ev Event to consume.
   * @param next_evs Remaining events to forward.
   * @return Event that is triggered when any one of the events is processed.
   */
  template <typename Event, typename... Args>
  Event any_of_internal(Event any_of_ev, Event current_ev, Args &&...next_evs) {
    any_of_internal(any_of_ev, current_ev);
    return any_of_internal(any_of_ev, std::forward<Args>(next_evs)...);
  }

  /**
   * Consumes the final event for `any_of`.
   *
   * @param any_of_ev Event to trigger when any one of the events is processed.
   * @param current_ev Event to consume.
   * @return Event that is triggered when any one of the events is processed.
   */
  event_type any_of_internal(event_type any_of_ev, event_type current_ev) {
    if (current_ev.processed()) {
      any_of_ev.trigger();
    } else {
      current_ev.add_callback(
          [any_of_ev](const auto &) { any_of_ev.trigger(); });
    }

    return any_of_ev;
  }

  /**
   * Consumes the final event for `any_of` with `value_event`.
   *
   * @tparam Value Value type of `value_event`.
   * @param any_of_ev Event to trigger when any one of the events is processed.
   * @param current_ev Event to consume.
   * @return Event that is triggered when any one of the events is processed.
   */
  template <typename Value>
  value_event<Value, Time>
  any_of_internal(value_event<Value, Time> any_of_ev,
                  value_event<Value, Time> current_ev) {
    if (current_ev.processed()) {
      any_of_ev.trigger(current_ev.value());
    } else {
      current_ev.add_callback([any_of_ev, current_ev](const auto &) {
        any_of_ev.trigger(current_ev.value());
      });
    }

    return any_of_ev;
  }

  /**
   * Consumes one event for `all_of` and forwards the remaining events.
   *
   * @tparam Args Type of the remaining events.
   * @param all_of_ev Event to trigger when all of the events are processed.
   * @param n_ptr Number of events not yet processed.
   * @param current_ev Event to consume.
   * @param next_evs Remaining events to forward.
   * @return Event that is triggered when all of the events are processed.
   */
  template <typename... Args>
  event_type all_of_internal(event_type all_of_ev,
                             std::shared_ptr<size_t> n_ptr,
                             event_type current_ev, Args &&...next_evs) {
    all_of_internal(all_of_ev, n_ptr, current_ev);
    return all_of_internal(all_of_ev, n_ptr, std::forward<Args>(next_evs)...);
  }

  /**
   * Consumes the final event for `all_of`.
   *
   * @param all_of_ev Event to trigger when all of the events are processed.
   * @param n_ptr Number of events not yet processed.
   * @param current_ev Event to consume.
   * @return Event that is triggered when all of the events are processed.
   */
  event_type all_of_internal(event_type all_of_ev,
                             std::shared_ptr<size_t> n_ptr,
                             event_type current_ev) {
    if (!current_ev.processed()) {
      ++*n_ptr;

      current_ev.add_callback([all_of_ev, n_ptr](const auto &) {
        if (--*n_ptr == 0) {
          all_of_ev.trigger();
        }
      });
    }

    return all_of_ev;
  }

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
    explicit scheduled_event(Time time, id_type id, const event_type &ev)
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
      scheduled_evs_;

  /// Current simulation time.
  Time now_ = Time{0};

  /// Next ID for scheduling an event.
  id_type next_id_ = 0;

  /// Set of coroutine handles belonging to pending processes.
  std::set<std::coroutine_handle<>> handles_;

  friend class internal::generic_promise_type<Time>;
};

} // namespace simcpp20
