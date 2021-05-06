// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <cassert>   // assert
#include <coroutine> // std::suspend_never
#include <memory>    // std::make_shared, std::shared_ptr, std::make_unique, ...
#include <optional>  // std::optional
#include <utility>   // std::forward

namespace simcpp20 {
template <typename Value, class Time = double>
class value_event : public event<Time> {
public:
  explicit value_event(simulation<Time> &sim) : event<Time>{sim} {}

  template <typename... Args> void trigger(Args &&...args) const {
    if (!event<Time>::pending()) {
      return;
    }

    *value_ = std::make_unique<Value>(std::forward<Args>(args)...);
    event<Time>::trigger();
  }

  Value await_resume() {
    event<Time>::await_resume();
    return value();
  }

  Value value() const {
    assert(*value_);
    return **value_;
  }

  class promise_type {
  public:
    template <typename... Args>
    explicit promise_type(simulation<Time> &sim, Args &&...)
        : sim_{sim}, ev_{sim} {}

    template <typename Class, typename... Args>
    explicit promise_type(Class &&, simulation<Time> &sim, Args &&...)
        : sim_{sim}, ev_{sim} {}

    template <typename Class, typename... Args>
    explicit promise_type(Class &&c, Args &&...) : sim_{c.sim}, ev_{c.sim} {}

#ifdef __INTELLISENSE__
    // IntelliSense fix. See https://stackoverflow.com/q/67209981.
    promise_type();
#endif

    value_event<Value, Time> get_return_object() const { return ev_; }

    event<Time> initial_suspend() const { return sim_.timeout(Time{0}); }

    std::suspend_never final_suspend() const noexcept { return {}; }

    void unhandled_exception() const { assert(false); }

    template <typename... Args> void return_value(Args &&...args) const {
      ev_.trigger(std::forward<Args>(args)...);
    }

    simulation<Time> &sim_;

    value_event<Value, Time> ev_;
  };

private:
  std::shared_ptr<std::unique_ptr<Value>> value_ =
      std::make_shared<std::unique_ptr<Value>>();

  friend class simulation<Time>;
};
} // namespace simcpp20
