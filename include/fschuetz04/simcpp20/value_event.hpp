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
  explicit value_event(simulation<Time> &sim) : event<Time> {
    std::make_shared<data>(sim)
  }
  {}

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
    assert(event<Time>::data_);

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
  const Value &await_resume() {
    assert(event<Time>::data_);

    event<Time>::await_resume();
    return value();
  }

  /// @return Value of the event.
  const Value &value() const {
    assert(event<Time>::data_);

    auto casted_data = static_pointer_cast<data>(event<Time>::data_);
    return *casted_data->value_;
  }

  /**
   * Alias for simulation::any_of.
   *
   * @param other Other event.
   * @return New pending event which is triggered when this event or the other
   * event is processed.
   */
  value_event<Value, Time>
  operator|(const value_event<Value, Time> &other) const {
    assert(event<Time>::data_);
    return event<Time>::data_->sim_.template any_of<Value>(*this, other);
  }

  /// Promise type for a coroutine returning a value event.
  class promise_type : public event<Time>::generic_promise_type {
  public:
    using handle_type = std::coroutine_handle<promise_type>;

    /**
     * Constructor.
     *
     * @tparam Args Types of additional arguments passed to the coroutine
     * function.
     * @param sim Reference to the simulation.
     */
    template <typename... Args>
    explicit promise_type(simulation<Time> &sim, Args &&...)
        : event<Time>::generic_promise_type{sim,
                                            handle_type::from_promise(*this)},
          ev_{sim} {}

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
        : event<Time>::generic_promise_type{sim,
                                            handle_type::from_promise(*this)},
          ev_{sim} {}

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
    explicit promise_type(Class &&c, Args &&...)
        : event<Time>::generic_promise_type{c.sim,
                                            handle_type::from_promise(*this)},
          ev_{c.sim} {}

#ifdef __INTELLISENSE__
    // IntelliSense fix. See https://stackoverflow.com/q/67209981.
    promise_type();
#endif

    /// @return Event associated with the process.
    const event<Time> &process_event() const override { return ev_; }

    /**
     * Called to get the return value of the coroutine function.
     *
     * @return Value event associated with the coroutine. This event is
     * triggered when the coroutine returns with the value it returns.
     */
    value_event<Value, Time> get_return_object() const { return ev_; }

    /**
     * Called when the coroutine returns. Trigger the event associated with the
     * coroutine with the value returned by the coroutine.
     *
     * @tparam Types of arguments to construct the return value with.
     * @param Arguments to construct the return value with.
     */
    template <typename... Args> void return_value(Args &&...args) const {
      ev_.trigger(std::forward<Args>(args)...);
    }

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
    ~data() override {}

    /// Value of the event.
    std::shared_ptr<Value> value_;
  };

  /**
   * @tparam Args Types of arguments to construct the event value with.
   * @param args Arguments to construct the event value with.
   */
  template <typename... Args> void set_value(Args &&...args) const {
    auto casted_data = static_pointer_cast<data>(event<Time>::data_);
    casted_data->value_ = std::make_shared<Value>(std::forward<Args>(args)...);
  }

  friend class simulation<Time>;
};
} // namespace simcpp20
