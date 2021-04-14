// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <coroutine>
#include <functional>
#include <memory>

#include "types.hpp"

namespace simcpp20 {
class simulation;

/// State of an event.
enum class event_state {
  /**
   * Event has not yet been triggered or processed. This is the initial state of
   * a new event.
   */
  pending,

  /**
   * Event has been triggered and will be processed at the current simulation
   * time.
   */
  triggered,

  /// Event is currently being processed or has been processed.
  processed
};

/// Can be awaited by processes
class event {
public:
  /**
   * Construct a new pending event.
   *
   * @param simulation Reference to simulation instance. Required for scheduling
   * the event.
   */
  event(simulation &sim);

  /**
   * Schedule the event to be processed immediately.
   *
   * Set the event state to triggered.
   */
  void trigger();

  /**
   * Schedule the event to be processed after the given delay.
   *
   * @param delay Delay after which to process the event.
   */
  void trigger_delayed(simtime delay);

  /**
   * Add a callback to the event.
   *
   * The callback will be called when the event is processed. If the event is
   * already processed, the callback is not stored, as it will never be called.
   *
   * @param cb Callback to add to the event. The callback receives the event,
   * so one function can differentiate between multiple events.
   */
  void add_callback(std::function<void(event &)> cb);

  /// @return Whether the event is pending.
  bool pending();

  /// @return Whether the event is triggered or processed.
  bool triggered();

  /// @return Whether the event is processed.
  bool processed();

private:
  /// Shared state of the event.
  class shared_state {
  public:
    /// Construct a new shared state.
    shared_state(simulation &sim);

    /// Destroy all processes waiting for the event if it has no been processed.
    ~shared_state();

    /// Coroutine handles of processes waiting for this event.
    std::vector<std::coroutine_handle<>> handles{};

    /// Callbacks for this event.
    std::vector<std::function<void(event &)>> cbs{};

    /// State of the event.
    event_state state = event_state::pending;

    /// Reference to the simulation.
    simulation &sim;
  };

  /// Shared state of the event.
  std::shared_ptr<shared_state> shared;

  /**
   * Add a coroutine handle to the event.
   *
   * The coroutine will be resumed when the event is processed. If the event is
   * already processed, the handle is not stored, as the coroutine will never
   * be resumed.
   *
   * @param handle Handle of the coroutine to resume when the event is
   * processed.
   */
  void add_handle(std::coroutine_handle<> handle);

  /**
   * Process the event.
   *
   * Set the event state to processed. Call all callbacks for this event.
   * Resume all coroutines waiting for this event.
   */
  void process();

  // simulation needs access to event::process
  friend class simulation;

  // await_event needs access to event::add_handle
  friend class await_event;
};
} // namespace simcpp20
