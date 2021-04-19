// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <coroutine>  // std::coroutine_handle, std::suspend_never
#include <functional> // std::function
#include <memory>     // std::make_shared, std::shared_ptr
#include <vector>     // std::vector

namespace simcpp20 {
class simulation;

/// Can be awaited by processes.
class event {
public:
  /**
   * Construct a new pending event.
   *
   * @param simulation Reference to simulation.
   */
  explicit event(simulation &sim);

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
  void abort();

  /**
   * Add a callback to the event to be called when the event is processed.
   *
   * @param cb Callback to add to the event.
   */
  void add_callback(std::function<void(event &)> cb);

  /// @return Whether the event is pending.
  bool pending();

  /// @return Whether the event is triggered or processed.
  bool triggered();

  /// @return Whether the event is processed.
  bool processed();

  /// @return Whether the event is aborted.
  bool aborted();

  /**
   * @return Whether the event is already processed and a waiting coroutine must
   * not be paused.
   */
  bool await_ready();

  /**
   * Resume a waiting coroutine when the event is processed.
   *
   * @param handle Handle of the waiting coroutine.
   */
  void await_suspend(std::coroutine_handle<> handle);

  /// No-op.
  void await_resume();

  /// Promise type for process coroutines.
  class promise_type;

private:
  /**
   * Process the event.
   *
   * Set the event state to processed. Call all callbacks for this event.
   * Resume all coroutines waiting for this event.
   */
  void process();

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
    explicit data(simulation &sim);

    /// Destroy all coroutines still waiting for the event.
    ~data();
  };

  /// Shared data of the event.
  std::shared_ptr<data> shared;

  /// The simulation needs access to event::process.
  friend class simulation;
};

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
  event get_return_object();

  /**
   * Register the process to be started immediately via an initial event.
   *
   * @return Initial event.
   */
  event initial_suspend();

  /// @return Awaitable which is always ready.
  std::suspend_never final_suspend() noexcept;

  /// No-op.
  void unhandled_exception();

  /// Trigger the underlying event since the process finished.
  void return_void();

private:
  /// Refernece to the simulation.
  simulation &sim;

  /// Underlying event which is triggered when the process finishes.
  event ev;
};
} // namespace simcpp20
