#pragma once

#include <cstddef> // size_t
#include <limits>  // std::numeric_limits
#include <queue>   // std::queue

#include "event.hpp"
#include "simulation.hpp"
#include "value_event.hpp"

namespace simcpp20 {

/**
 * A shared store holding objects.
 *
 * @tparam T Type of objects to store.
 */
template <typename T> class store {
public:
  /**
   * Constructor.
   *
   * @param sim Reference to the simulation.
   * @param capacity Maximum number of items the store can hold. Defaults to
   * unlimited.
   */
  explicit store(simulation<> &sim,
                 size_t capacity = std::numeric_limits<size_t>::max())
      : sim_{sim}, capacity_{capacity} {}

  /**
   * Retrieve a value from the store.
   *
   * @return An event that will be triggered with the next value
   * from the store once it is available, which may be immediately.
   */
  value_event<T> get() {
    auto ev = sim_.event<T>();

    // add callback to trigger puts when this get is processed
    ev.add_callback([this](const auto &) { trigger_puts(); });

    // queue the get event
    gets_.push(ev);

    // try to match this get with an available value
    trigger_gets();

    return ev;
  }

  /**
   * Add a value to the store.
   *
   * @param value The value to add.
   * @return An event that will be triggered when the store has capacity and the
   * value is added to the store, which may be immediately.
   */
  event<> put(const T &value) {
    T copy = value;
    return put(std::move(copy));
  }

  /**
   * Add a value (via move) to the store.
   *
   * @param value The value to add.
   * @return An event that will be triggered when the store has capacity and the
   * value is added to the store, which may be immediately.
   */
  event<> put(T &&value) {
    auto ev = sim_.event();

    // add callback to trigger gets when this put is processed
    ev.add_callback([this](const auto &) { trigger_gets(); });

    // queue the put event
    puts_.emplace(ev, std::move(value));

    // try to process this put if there is capacity
    trigger_puts();

    return ev;
  }

private:
  /// Reference to the simulation.
  simulation<> &sim_;

  /// Values currently in the store.
  std::queue<T> values_;

  /// Capacity of the store.
  size_t capacity_;

  /// Pending get events.
  std::queue<value_event<T>> gets_;

  /// Pairs of pending put events and the values to be put into the store.
  std::queue<std::pair<event<>, T>> puts_;

  /// Trigger pending get events until the store is empty.
  void trigger_gets() {
    while (!values_.empty() && !gets_.empty()) {
      auto ev = std::move(gets_.front());
      gets_.pop();
      if (ev.aborted()) {
        continue;
      }

      ev.trigger(std::move(values_.front()));
      values_.pop();
    }
  }

  /// Trigger pending put events until the store is full.
  void trigger_puts() {
    while (values_.size() < capacity_ && !puts_.empty()) {
      auto [ev, value] = std::move(puts_.front());
      puts_.pop();
      if (ev.aborted()) {
        continue;
      }

      values_.emplace(std::move(value));
      ev.trigger();
    }
  }
};

} // namespace simcpp20
