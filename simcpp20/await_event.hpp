// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <coroutine>
#include <memory>

#include "event.hpp"

namespace simcpp20 {
/**
 * Awaitable to wait for an event inside a coroutine.
 *
 * TODO(fschuetz04): Combine with event?
 */
class await_event {
public:
  /**
   * Construct a new await_event.
   *
   * @param ev Event to wait for.
   */
  await_event(std::shared_ptr<event> ev);

  /**
   * @return Whether the event is already processed and the coroutine must
   * not be paused.
   */
  bool await_ready();

  /**
   * Resume the waiting coroutine when the event is processed.
   *
   * @param handle Handle of the coroutine. This handle is added to the awaited
   * event.
   */
  void await_suspend(std::coroutine_handle<> handle);

  /// Called when the coroutine is resumed.
  void await_resume();

private:
  /// Event to wait for.
  std::shared_ptr<event> ev;
};
} // namespace simcpp20
