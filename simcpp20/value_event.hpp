// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <cassert>   // assert
#include <coroutine> // std::suspend_never
#include <memory>    // std::make_shared, std::shared_ptr
#include <optional>  // std::optional

#include "event.hpp"

namespace simcpp20 {
class simulation;

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
} // namespace simcpp20
