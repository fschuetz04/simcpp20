// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <cassert>   // assert
#include <coroutine> // std::suspend_never
#include <utility>   // std::forward, std::exchange

namespace simcpp20 {
/**
 * One event with a value.
 *
 * @tparam Time Type used for simulation time.
 * @tparam Value Type of the value.
 */
template <typename Value, class Time = double>
class value_event : public event<Time> {
public:
  /**
   * Constructor.
   *
   * @param simulation Reference to the simulation.
   */
  explicit value_event(simulation<Time> &sim) : event<Time>{new data(sim)} {}

  /**
   * Set the event state to triggered, and schedule it to be processed
   * immediately. If the event is not pending, nothing is done.
   *
   * TODO(fschuetz04): Check whether used on a process?
   *
   * @tparam Types of arguments to construct the event value with.
   * @param args Arguments to construct the event value with.
   */
  template <typename... Args> void trigger(Args &&...args) const {
    assert(event<Time>::awaiting_ev_ == nullptr);
    assert(event<Time>::data_ != nullptr);

    if (!event<Time>::pending()) {
      return;
    }

    set_value(std::forward<Args>(args)...);
    event<Time>::trigger();
  }

  /**
   * Called when a coroutine is resumed after using co_await on the event or if
   * the coroutine did not need to be suspended. The return value is the return
   * value of the co_await expression.
   *
   * @return Value of the event.
   */
  Value &await_resume() {
    assert(event<Time>::data_ != nullptr);

    event<Time>::await_resume();
    return value();
  }

  /// @return Value of the event.
  Value &value() const {
    assert(event<Time>::awaiting_ev_ == nullptr);
    assert(event<Time>::data_ != nullptr);

    auto casted_data = static_cast<data *>(event<Time>::data_);
    return *casted_data->value_;
  }

  /// Promise type for a coroutine returning a value event.
  class promise_type {
  public:
    /**
     * Constructor.
     *
     * @tparam Args Types of additional arguments passed to the coroutine
     * function.
     * @param sim Reference to the simulation.
     */
    template <typename... Args>
    explicit promise_type(simulation<Time> &sim, Args &&...)
        : sim_{sim}, ev_{sim} {}

    /**
     * Constructor.
     *
     * @tparam Class Class type if the coroutine function is a lambda or a
     * member function of a class.
     * @tparam Args Types of additional arguments passed to the coroutine
     * function.
     * @param sim Reference to the simulation.
     */
    template <typename Class, typename... Args>
    explicit promise_type(Class &&, simulation<Time> &sim, Args &&...)
        : sim_{sim}, ev_{sim} {}

    /**
     * Constructor.
     *
     * @tparam Class Class type if the coroutine function is a member function
     * of a class. Must contain a member variable sim referencing the simulation
     * instance.
     * @tparam Args Types of additional arguments passed to the coroutine
     * function.
     * @param c Class instance.
     */
    template <typename Class, typename... Args>
    explicit promise_type(Class &&c, Args &&...) : sim_{c.sim}, ev_{c.sim} {}

#ifdef __INTELLISENSE__
    // IntelliSense fix. See https://stackoverflow.com/q/67209981.
    promise_type();
#endif

    /**
     * Called to get the return value of the coroutine function.
     *
     * @return Value event associated with the coroutine. This event is
     * triggered when the coroutine returns with the value it returns.
     */
    value_event<Value, Time> get_return_object() const { return ev_; }

    /**
     * Called when the coroutine is started. The coroutine awaits the return
     * value before running.
     *
     * @return Event which will be processed at the current simulation time.
     */
    event<Time> initial_suspend() const { return sim_.timeout(Time{0}); }

    /// Called when an exception is thrown inside the coroutine and not handled.
    void unhandled_exception() const { assert(false); }

    /**
     * Called when the coroutine returns. Trigger the event associated with the
     * coroutine with the value returned by the coroutine.
     *
     * @tparam Types of arguments to construct the return value with.
     * @parma Arguments to construct the return value with.
     */
    template <typename... Args> void return_value(Args &&...args) const {
      ev_.trigger(std::forward<Args>(args)...);
    }

    /**
     * Called after the coroutine returns.
     *
     * @return Awaitable which is always ready.
     */
    std::suspend_never final_suspend() const noexcept { return {}; }

    /// Reference to the simulation.
    simulation<Time> &sim_;

    /**
     * Value event associated with the coroutine. This event is triggered when
     * the coroutine returns with the value the coroutine returns.
     */
    value_event<Value, Time> ev_;
  };

private:
  /// Shared data of the event.
  class data : public event<Time>::data {
  public:
    using event<Time>::data::data;

    /// Destructor.
    ~data() override {
      if (value_ != nullptr) {
        delete std::exchange(value_, nullptr);
      }
    }

    /// Value of the event.
    Value *value_ = nullptr;
  };

  /**
   * @tparam Args Types of arguments to construct the event value with.
   * @param args Arguments to construct the event value with.
   */
  template <typename... Args> void set_value(Args &&...args) const {
    auto casted_data = static_cast<data *>(event<Time>::data_);
    casted_data->value_ = new Value(std::forward<Args>(args)...);
  }

  friend class simulation<Time>;
};
} // namespace simcpp20
