#pragma once

#include <cassert>    // assert
#include <coroutine>  // std::coroutine_handle, std::suspend_never
#include <cstddef>    // std::size_t
#include <functional> // std::function, std::hash
#include <memory>     // std::shared_ptr, std::make_shared
#include <utility>    // std::exchange, std::move
#include <vector>     // std::vector

#include "promise_type.hpp"

namespace simcpp20 {

template <typename Time> class simulation;
template <typename Time> class process;

/**
 * An event that can be awaited, triggered, and aborted.
 *
 * @tparam Time Type used for simulation time.
 */
template <typename Time = double> class event {
public:
  /**
   * Constructor.
   *
   * @param simulation Reference to the simulation.
   */
  explicit event(simulation<Time> &sim) : data_{std::make_shared<data>(sim)} {}

  /// Destructor.
  virtual ~event() = default;

  /**
   * Copy constructor.
   *
   * @param other Event to copy.
   */
  event(const event &other) : data_{other.data_} { assert(data_); }

  /**
   * Move constructor.
   *
   * @param other Event to move.
   */
  event(event &&other) noexcept : data_{std::move(other.data_)} {
    assert(data_);
  }

  /**
   * Copy assignment operator.
   *
   * @param other Event to replace this event with.
   * @return Reference to this instance.
   */
  event &operator=(const event &other) {
    data_ = other.data_;
    assert(data_);
    return *this;
  }

  /**
   * Move assignment operator.
   *
   * @param other Event to replace this event with.
   * @return Reference to this instance.
   */
  event &operator=(event &&other) noexcept {
    data_ = std::move(other.data_);
    assert(data_);
    return *this;
  }

  /**
   * Set the event state to triggered and schedule it to be processed
   * immediately. If the event is not pending, nothing is done.
   */
  void trigger() const {
    assert(data_);

    if (!pending()) {
      return;
    }

    data_->sim_.schedule(*this);
    data_->state_ = state::triggered;
  }

  /**
   * Set the event state to aborted. If the event is not pending, nothing is
   * done.
   */
  void abort() const {
    assert(data_);

    if (!pending()) {
      return;
    }

    data_->state_ = state::aborted;

    data_->cbs_.clear();

    std::vector<internal::generic_promise_type<Time> *> temp_promises;
    data_->promises_.swap(temp_promises);
    for (auto &promise : temp_promises) {
      promise->process_handle().destroy();
    }
  }

  /// @param cb Callback to be called when the event is processed.
  void add_callback(std::function<void(const event<Time> &)> cb) const {
    assert(data_);

    if (processed() || aborted()) {
      return;
    }

    data_->cbs_.emplace_back(cb);
  }

  /// @return Whether the event is pending.
  bool pending() const {
    assert(data_);
    return data_->state_ == state::pending;
  }

  /// @return Whether the event is triggered or processed.
  bool triggered() const {
    assert(data_);
    return data_->state_ == state::triggered || processed();
  }

  /// @return Whether the event is processed.
  bool processed() const {
    assert(data_);
    return data_->state_ == state::processed;
  }

  /// @return Whether the event is aborted.
  bool aborted() const {
    assert(data_);
    return data_->state_ == state::aborted;
  }

  /**
   * Called when using co_await on the event. The calling coroutine is only
   * suspended if this method returns false.
   *
   * @return Whether the event is processed.
   */
  bool await_ready() const {
    assert(data_);
    return processed();
  }

  /**
   * Called when a coroutine is suspended after using co_await on the event.
   *
   * @tparam Promise Promise type of the coroutine.
   * @param handle Coroutine handle.
   */
  template <typename Promise>
  void await_suspend(std::coroutine_handle<Promise> handle) {
    assert(data_);

    if (aborted()) {
      handle.destroy();
      return;
    }

    data_->promises_.push_back(&handle.promise());
  }

  /**
   * Called when a coroutine is resumed after using co_await on the event or if
   * the coroutine did not need to be suspended.
   */
  void await_resume() { assert(data_); }

  /**
   * Alias for simulation::any_of.
   *
   * @param other Other event.
   * @return New pending event which is triggered when this event or the other
   * event is processed.
   */
  event<Time> operator|(const event<Time> &other) const {
    assert(data_);
    return data_->sim_.any_of(*this, other);
  }

  /**
   * Alias for simulation::all_of.
   *
   * @param other Other event.
   * @return New pending event which is triggered when this event and the other
   * event are processed.
   */
  event<Time> operator&(const event<Time> &other) const {
    assert(data_);
    return data_->sim_.all_of(*this, other);
  }

  /** Comparison operator.
   *
   * @param other Other event.
   * @return Whether this event is equal to the other event.
   */
  bool operator==(const event<Time> &other) const {
    return data_ == other.data_;
  }

protected:
  /**
   * Set the event state to processed, resume all coroutines awaiting this
   * event, and call all callbacks added to the event.
   */
  void process() const {
    assert(data_);

    if (processed() || aborted()) {
      return;
    }

    data_->state_ = state::processed;

    std::vector<internal::generic_promise_type<Time> *> temp_promises;
    data_->promises_.swap(temp_promises);
    for (auto &promise : temp_promises) {
      if (promise->is_aborted()) {
        promise->process_handle().destroy();
      } else {
        promise->process_handle().resume();
      }
    }

    for (auto &cb : data_->cbs_) {
      cb(*this);
    }
    data_->cbs_.clear();
  }

  /// State of the event.
  enum class state {
    /// Event is not yet triggered or aborted.
    pending,

    /// Event is triggered and will be processed at the current simulation time.
    triggered,

    /// Event is processed.
    processed,

    /// Event is aborted.
    aborted
  };

  /// Shared data of the event.
  class data {
  public:
    /**
     * Constructor.
     *
     * @param sim Reference to the simulation.
     */
    explicit data(simulation<Time> &sim) : sim_{sim} {}

    /// Destructor.
    virtual ~data() = default;

    /// State of the event.
    state state_ = state::pending;

    /// Promises awaiting the event.
    std::vector<internal::generic_promise_type<Time> *> promises_ = {};

    /// Callbacks added to the event.
    std::vector<std::function<void(const event<Time> &)>> cbs_ = {};

    /// Reference to the simulation.
    simulation<Time> &sim_;
  };

  /**
   * Constructor.
   *
   * @param data Shared data.
   */
  explicit event(const std::shared_ptr<data> &data_) : data_{data_} {
    assert(data_);
  }

  /// Shared data of the event.
  std::shared_ptr<data> data_;

  friend class simulation<Time>;
  friend struct std::hash<event<Time>>;
};

} // namespace simcpp20

namespace std {

/// Specialization of std::hash for simcpp20::event.
template <typename Time> struct hash<simcpp20::event<Time>> {
  /**
   * @param ev Event.
   * @return Hash of the event.
   */
  std::size_t operator()(const simcpp20::event<Time> &ev) const {
    return std::hash<decltype(ev.data_.get())>()(ev.data_.get());
  }
};

} // namespace std
