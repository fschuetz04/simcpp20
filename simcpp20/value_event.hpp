// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <cassert>   // assert
#include <coroutine> // std::suspend_never
#include <memory>    // std::make_shared, std::shared_ptr
#include <optional>  // std::optional

namespace simcpp20 {
/**
 * Can be awaited by processes and contains a value after it is triggered.
 *
 * @tparam TValue Type of the contained value.
 * @tparam TTime Type used for simulation time.
 */
template <class TValue, class TTime = double>
class value_event : public event<TTime> {
public:
  /**
   * Construct a new pending value event.
   *
   * @param simulation Reference to simulation.
   */
  explicit value_event(simulation<TTime> &sim) : event<TTime>{sim} {}

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
  void trigger(TValue value) const {
    if (!event<TTime>::pending()) {
      return;
    }

    *value_ = value;
    event<TTime>::trigger();
  }

  /// @return Value of the event.
  TValue await_resume() const {
    event<TTime>::await_resume();
    return value();
  }

  /// @return Value of the event.
  TValue value() const {
    assert(value_->has_value());
    return value_->value();
  }

  /// Promise type for value process coroutines.
  class promise_type {
  public:
    /**
     * Construct a new promise type instance.
     *
     * @tparam TArgs Additional arguments passed to the process function. These
     * arguments are ignored.
     * @param sim Reference to the simulation.
     */
    template <class... TArgs>
    explicit promise_type(simulation<TTime> &sim, TArgs &&...)
        : sim{sim}, ev{sim} {}

    /**
     * Construct a new promise type instance.
     *
     * @tparam TClass Class instance if the process function is a lambda or a
     * member function of a class.
     * @tparam TArgs Additional arguments passed to the process function. These
     * arguments are ignored.
     * @param sim Reference to the simulation.
     */
    template <class TClass, class... TArgs>
    explicit promise_type(TClass &&, simulation<TTime> &sim, TArgs &&...)
        : sim{sim}, ev{sim} {}

    /**
     * Construct a new promise type instance.
     *
     * @tparam TClass Class instance if the process function is a lambda or a
     * member function of a class. Must contain a member variable sim
     * referencing the simulation.
     * @tparam TArgs Additional arguments passed to the process function. These
     * arguments are ignored.
     * @param c Class instance.
     */
    template <class TClass, class... TArgs>
    explicit promise_type(TClass &&c, TArgs &&...) : sim{c.sim}, ev{c.sim} {}

#ifdef __INTELLISENSE__
    /**
     * Fix IntelliSense complaining about missing default constructor for the
     * promise type.
     */
    promise_type();
#endif

    /// @return Event which will be triggered when the process finishes.
    value_event<TValue, TTime> get_return_object() const { return ev; }

    /**
     * Register the process to be started immediately via an initial event.
     *
     * @return Initial event.
     */
    event<TTime> initial_suspend() const { return sim.timeout(TTime{0}); }

    /// @return Awaitable which is always ready.
    std::suspend_never final_suspend() const noexcept { return {}; }

    /// No-op.
    void unhandled_exception() const { assert(false); }

    /// Trigger the underlying event since the process finished.
    void return_value(TValue value) const { ev.trigger(value); }

  private:
    /// Reference to the simulation.
    simulation<TTime> &sim;

    /// Underlying event which is triggered when the process finishes.
    const value_event<TValue, TTime> ev;
  };

private:
  /// Value of the event.
  std::shared_ptr<std::optional<TValue>> value_ =
      std::make_shared<std::optional<TValue>>();

  /// The simulation needs access to value_event<T>::value_.
  friend class simulation<TTime>;
};
} // namespace simcpp20
