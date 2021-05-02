// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <cassert>    // assert
#include <coroutine>  // std::coroutine_handle, std::suspend_never
#include <functional> // std::function
#include <memory>     // std::make_shared, std::shared_ptr
#include <vector>     // std::vector

namespace simcpp20 {
template <class TTime> class simulation;

/**
 * Can be awaited by processes.
 *
 * @tparam TTime Type used for simulation time.
 */
template <class TTime = double> class event {
public:
  /**
   * Construct a new pending event.
   *
   * @param simulation Reference to simulation.
   */
  explicit event(simulation<TTime> &sim) : data_{std::make_shared<data>(sim)} {}

  /**
   * Set the event state to triggered and schedule it to be processed
   * immediately.
   *
   * If the event is not pending, nothing is done.
   *
   * TODO(fschuetz04): Check whether used on a process?
   */
  void trigger() const {
    if (!pending()) {
      return;
    }

    data_->sim.schedule(*this);
    data_->state_ = state::triggered;
  }

  /**
   * Set the event state to aborted and destroy all coroutines waiting for it.
   *
   * If the event is not pending, nothing is done.
   *
   * TODO(fschuetz04): Check whether used on a process? Destroy process
   * coroutine?
   */
  void abort() const {
    if (!pending()) {
      return;
    }

    data_->state_ = state::aborted;

    for (auto &handle : data_->handles) {
      handle.destroy();
    }
    data_->handles.clear();

    data_->cbs.clear();
  }

  /**
   * Add a callback to the event to be called when the event is processed.
   *
   * @param cb Callback to add to the event.
   */
  void add_callback(std::function<void(const event<TTime> &)> cb) const {
    if (processed() || aborted()) {
      return;
    }

    data_->cbs.emplace_back(cb);
  }

  /// @return Whether the event is pending.
  bool pending() const { return data_->state_ == state::pending; }

  /// @return Whether the event is triggered or processed.
  bool triggered() const {
    return data_->state_ == state::triggered || processed();
  }

  /// @return Whether the event is processed.
  bool processed() const { return data_->state_ == state::processed; }

  /// @return Whether the event is aborted.
  bool aborted() const { return data_->state_ == state::aborted; }

  /**
   * @return Whether the event is already processed and a waiting coroutine must
   * not be paused.
   */
  bool await_ready() const { return processed(); }

  /**
   * Resume a waiting coroutine when the event is processed.
   *
   * @param handle Handle of the waiting coroutine.
   */
  void await_suspend(std::coroutine_handle<> handle) {
    assert(!processed());

    if (!aborted() && !processed()) {
      data_->handles.push_back(handle);
    }

    data_ = nullptr;
  }

  /// No-op.
  void await_resume() const {}

  /**
   * Alias for sim.any_of. Create a pending event which is triggered when this
   * event or the given event is processed.
   *
   * @param other Given event.
   * @return Created event.
   */
  event<TTime> operator|(const event<TTime> &other) const {
    return data_->sim.any_of({*this, other});
  }

  /**
   * Alias for sim.all_of. Create a pending event which is triggered when this
   * event and the given event is processed.
   *
   * @param other Given event.
   * @return Created event.
   */
  event<TTime> operator&(const event<TTime> &other) const {
    return data_->sim.all_of({*this, other});
  }

  /// Promise type for process coroutines.
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
     * member function of a class. This argument is ignored.
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
    event<TTime> get_return_object() const { return ev; }

    /**
     * Register the process to be started immediately via an initial event.
     *
     * @return Initial event.
     */
    event<TTime> initial_suspend() const { return sim.timeout(TTime{0}); }

    /// @return Awaitable which is always ready.
    std::suspend_never final_suspend() const noexcept { return {}; }

    /// No-op.
    void unhandled_exception() const {}

    /// Trigger the underlying event since the process finished.
    void return_void() const { ev.trigger(); }

  private:
    /// Refernece to the simulation.
    simulation<TTime> &sim;

    /// Underlying event which is triggered when the process finishes.
    const event<TTime> ev;
  };

private:
  /**
   * Process the event.
   *
   * Set the event state to processed. Call all callbacks for this event.
   * Resume all coroutines waiting for this event.
   */
  void process() const {
    if (processed() || aborted()) {
      return;
    }

    data_->state_ = state::processed;

    for (auto &handle : data_->handles) {
      handle.resume();
    }
    data_->handles.clear();

    for (auto &cb : data_->cbs) {
      cb(*this);
    }
    data_->cbs.clear();
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
    /// Construct a new shared data instance.
    explicit data(simulation<TTime> &sim) : sim{sim} {}

    /// Destroy all coroutines still waiting for the event.
    ~data() {
      for (auto &handle : handles) {
        handle.destroy();
      }
    }

    /// State of the event.
    state state_ = state::pending;

    /// Coroutine handles of processes waiting for this event.
    std::vector<std::coroutine_handle<>> handles{};

    /// Callbacks for this event.
    std::vector<std::function<void(const event<TTime> &)>> cbs{};

    /// Reference to the simulation.
    simulation<TTime> &sim;
  };

  /// Shared data of the event.
  std::shared_ptr<data> data_;

  /// The simulation needs access to event<TTime>::process.
  friend class simulation<TTime>;
};

} // namespace simcpp20
