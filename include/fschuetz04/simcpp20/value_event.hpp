#pragma once

#include <cassert> // assert
#include <memory>  // std::make_shared, std::shared_ptr
#include <utility> // std::forward, std::exchange

#include "event.hpp"

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
  explicit value_event(simulation<Time> &sim)
      : event<Time>{std::make_shared<data>(sim)} {}

  /// Destructor.
  ~value_event() override = default;

  /**
   * Set the event state to triggered, and schedule it to be processed
   * immediately. If the event is not pending, nothing is done.
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

private:
  /// Shared data of the event.
  class data : public event<Time>::data {
  public:
    using event<Time>::data::data;

    /// Destructor.
    ~data() override = default;

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
