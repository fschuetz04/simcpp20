// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <cassert>
#include <coroutine>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <tuple>
#include <vector>

namespace simcpp20 {
class simulation;

using time_type = double;
using id_type = uint64_t;

/* event */

/// Can be awaited by processes.
class event {
public:
  /**
   * Construct a new pending event.
   *
   * @param simulation Reference to simulation.
   */
  explicit event(simulation &sim) : shared{std::make_shared<data>(sim)} {}

  /**
   * Set the event state to triggered and schedule it to be processed
   * immediately.
   *
   * If the event is not pending, nothing is done.
   *
   * TODO(fschuetz04): Check whether used on a process?
   * TODO(fschuetz04): trigger_delayed()?
   */
  void trigger();

  /**
   * Set the event state to aborted and destroy all coroutines waiting for it.
   *
   * If the event is not pending, nothing is done.
   *
   * TODO(fschuetz04): Check whether used on a process? Destroy process
   * coroutine?
   */
  void abort() {
    if (!pending()) {
      return;
    }

    shared->state = state::aborted;

    for (auto &handle : shared->handles) {
      handle.destroy();
    }
    shared->handles.clear();

    shared->cbs.clear();
  }

  /**
   * Add a callback to the event to be called when the event is processed.
   *
   * @param cb Callback to add to the event.
   */
  void add_callback(std::function<void(event &)> cb) {
    if (processed() || aborted()) {
      return;
    }

    shared->cbs.emplace_back(cb);
  }

  /// @return Whether the event is pending.
  bool pending() { return shared->state == state::pending; }

  /// @return Whether the event is triggered or processed.
  bool triggered() { return shared->state == state::triggered || processed(); }

  /// @return Whether the event is processed.
  bool processed() { return shared->state == state::processed; }

  /// @return Whether the event is aborted.
  bool aborted() { return shared->state == state::aborted; }

  /**
   * @return Whether the event is already processed and a waiting coroutine must
   * not be paused.
   */
  bool await_ready() { return processed(); }

  /**
   * Resume a waiting coroutine when the event is processed.
   *
   * @param handle Handle of the waiting coroutine.
   */
  void await_suspend(std::coroutine_handle<> handle) {
    assert(!processed());

    if (!aborted() && !processed()) {
      shared->handles.push_back(handle);
    }

    shared = nullptr;
  }

  /// No-op.
  void await_resume() {}

  /// Promise type for process coroutines.
  class promise_type;

private:
  /**
   * Process the event.
   *
   * Set the event state to processed. Call all callbacks for this event.
   * Resume all coroutines waiting for this event.
   */
  void process() {
    if (processed() || aborted()) {
      return;
    }

    shared->state = state::processed;

    for (auto &handle : shared->handles) {
      handle.resume();
    }
    shared->handles.clear();

    for (auto &cb : shared->cbs) {
      cb(*this);
    }
    shared->cbs.clear();
  }

  /// State of an event.
  enum class state {
    /**
     * Event is not yet triggered or aborted. This is the initial state of a new
     * event.
     */
    pending,

    /**
     * Event is triggered and will be processed at the current simulation time.
     */
    triggered,

    /// Event is processed.
    processed,

    /// Event is aborted.
    aborted
  };

  /// Shared data of the event.
  class data {
  public:
    /// State of the event.
    event::state state = event::state::pending;

    /// Coroutine handles of processes waiting for this event.
    std::vector<std::coroutine_handle<>> handles{};

    /// Callbacks for this event.
    std::vector<std::function<void(event &)>> cbs{};

    /// Reference to the simulation.
    simulation &sim;

    /// Construct a new shared data instance.
    explicit data(simulation &sim) : sim{sim} {}

    /// Destroy all coroutines still waiting for the event.
    ~data() {
      for (auto &handle : handles) {
        handle.destroy();
      }
    }
  };

  /// Shared data of the event.
  std::shared_ptr<data> shared;

  /// The simulation needs access to event::process.
  friend class simulation;
};

/* event::promise_type */

/// Promise type for process coroutines.
class event::promise_type {
public:
  /**
   * Construct a new promise type instance.
   *
   * @tparam Args Additional arguments passed to the process function. These
   * arguments are ignored.
   * @param sim Reference to the simulation.
   */
  template <typename... Args>
  explicit promise_type(simulation &sim, Args &&...) : sim{sim}, ev{sim} {}

  /// @return Event which will be triggered when the process finishes.
  event get_return_object() { return ev; }

  /**
   * Register the process to be started immediately via an initial event.
   *
   * @return Initial event.
   */
  event initial_suspend();

  /// @return Awaitable which is always ready.
  std::suspend_never final_suspend() noexcept { return {}; }

  /// No-op.
  void unhandled_exception() {}

  /// Trigger the underlying event since the process finished.
  void return_void() { ev.trigger(); }

private:
  /// Refernece to the simulation.
  simulation &sim;

  /// Underlying event which is triggered when the process finishes.
  event ev;
};

/* value_event */

/// Can be awaited by processes and contains a value after it is triggered.
template <class T> class value_event : public event {
public:
  /**
   * Construct a new pending value event.
   *
   * @param simulation Reference to simulation.
   */
  explicit value_event(simulation &sim) : event{sim} {}

  /**
   * Set the event state to triggered and schedule it to be processed
   * immediately.
   *
   * If the event is not pending, nothing is done.
   *
   * TODO(fschuetz04): Check whether used on a value process?
   *
   * @param value Value of the event.
   */
  void trigger(T value) {
    if (!pending()) {
      return;
    }

    *value_ = value;
    event::trigger();
  }

  /// @return Value of the event.
  T await_resume() { return value(); }

  /// @return Value of the event.
  T value() {
    assert(*value_);
    return **value_;
  }

  /// Promise type for value process coroutines.
  class promise_type {
  public:
    /**
     * Construct a new promise type instance.
     *
     * @tparam Args Additional arguments passed to the process function. These
     * arguments are ignored.
     * @param sim Reference to the simulation.
     */
    template <typename... Args>
    explicit promise_type(simulation &sim, Args &&...) : sim{sim}, ev{sim} {}

    /// @return Event which will be triggered when the process finishes.
    value_event<T> get_return_object() { return ev; }

    /**
     * Register the process to be started immediately via an initial event.
     *
     * @return Initial event.
     */
    event initial_suspend();

    /// @return Awaitable which is always ready.
    std::suspend_never final_suspend() noexcept { return {}; }

    /// No-op.
    void unhandled_exception() {}

    /// No-op. Should never be called.
    void return_void() { assert(false); }

    /// Trigger the underlying event since the process finished.
    void return_value(T value) { ev.trigger(value); }

  private:
    /// Reference to the simulation.
    simulation &sim;

    /// Underlying event which is triggered when the process finishes.
    value_event<T> ev;
  };

private:
  /// Value of the event.
  std::shared_ptr<std::optional<T>> value_ =
      std::make_shared<std::optional<T>>();

  /// The simulation needs access to value_event<T>::value_.
  friend class simulation;
};

/* simulation */

using event_alias = event;

/**
 * Used to run a discrete-event simulation.
 *
 * To create a new instance, default-initialize the class:
 *
 *     simcpp20::simulation sim;
 */
class simulation {
public:
  /// @return Pending event.
  event_alias event() { return event_alias{*this}; }

  /**
   * @tparam T Value type of the event.
   * @return Pending value event.
   */
  template <class T> value_event<T> event() { return value_event<T>{*this}; }

  /**
   * @param delay Delay after which to process the event.
   * @return Pending event.
   */
  event_alias timeout(time_type delay) {
    auto ev = event();
    schedule(ev, delay);
    return ev;
  }

  /**
   * @tparam T Value type of the event.
   * @param delay Delay after which to process the event.
   * @param value Value of the event.
   * @return Pending value event.
   */
  template <class T> value_event<T> timeout(time_type delay, T value) {
    value_event<T> ev{*this};
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
  simcpp20::event any_of(std::vector<simcpp20::event> evs) {
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

  /**
   * Create a pending event which is triggered when all of the given events are
   * processed.
   *
   * TODO(fschuetz04): Check behavior.
   *
   * @param evs List of events.
   * @return Created event.
   */
  simcpp20::event all_of(std::vector<simcpp20::event> evs) {
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

  /**
   * Schedule the given event to be processed after the given delay.
   *
   * @param delay Delay after which to process the event.
   * @param ev Event to be processed.
   */
  void schedule(event_alias ev, time_type delay = 0) {
    if (delay < 0) {
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
  void run_until(time_type target) {
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
  bool empty() { return scheduled_evs.empty(); }

  /// @return Current simulation time.
  time_type now() { return now_; }

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
    scheduled_event(time_type time, id_type id, simcpp20::event ev)
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
    time_type time() const { return time_; }

    /// @return Event to process.
    simcpp20::event ev() { return ev_; }

  private:
    /// Time at which to process the event.
    time_type time_;

    /**
     * Incremental ID to sort events scheduled at the same time by insertion
     * order.
     */
    id_type id;

    /// Event to process.
    simcpp20::event ev_;
  };

  /// Event queue.
  std::priority_queue<scheduled_event, std::vector<scheduled_event>,
                      std::greater<scheduled_event>>
      scheduled_evs{};

  /// Current simulation time.
  time_type now_ = 0;

  /// Next ID for scheduling an event.
  id_type next_id = 0;
};

/* event */

void event::trigger() {
  if (!pending()) {
    return;
  }

  shared->sim.schedule(*this);
  shared->state = state::triggered;
}

event event::promise_type::initial_suspend() {
  event ev{sim};
  sim.schedule(ev);
  return ev;
}

template <class T> event value_event<T>::promise_type::initial_suspend() {
  event ev{sim};
  sim.schedule(ev);
  return ev;
}

} // namespace simcpp20
