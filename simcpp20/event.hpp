// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <coroutine>
#include <functional>
#include <memory>

#include "event_state.hpp"

namespace simcpp20 {
class simulation;

using simtime = double;

/// Can be awaited by processes
class event : public std::enable_shared_from_this<event> {
public:
  /**
   * Construct a new pending event.
   *
   * @param simulation Reference to simulation instance. Required for scheduling
   * the event.
   */
  event(simulation &sim);

  /// Destroy the event and all processes waiting for it.
  ~event();

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
  void add_callback(std::function<void(std::shared_ptr<event>)> cb);

  /// @return Whether the event is pending.
  bool pending();

  /// @return Whether the event is triggered or processed.
  bool triggered();

  /// @return Whether the event is processed.
  bool processed();

private:
  /// Coroutine handles of processes waiting for this event.
  std::vector<std::coroutine_handle<>> handles = {};

  /// Callbacks for this event.
  std::vector<std::function<void(std::shared_ptr<event>)>> cbs = {};

  /// State of the event.
  event_state state = event_state::pending;

  /// Reference to the simulation.
  simulation &sim;

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
