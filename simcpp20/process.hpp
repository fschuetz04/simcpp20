// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <coroutine>
#include <memory>

#include "await_event.hpp"
#include "event.hpp"
#include "simulation.hpp"

namespace simcpp20 {
/// Used to simulate an actor.
class process {
public:
  /// Promise type for processes.
  class promise_type {
  public:
    /**
     * Construct a new promise_type.
     *
     * @tparam Args Additional arguments passed to the process function. These
     * arguments are ignored.
     * @param sim Reference to the simulation.
     */
    template <typename... Args>
    promise_type(simulation &sim, Args &&...)
        : sim(sim), proc_ev(sim.event()) {}

    /**
     * @return process instance containing an event which will be triggered when
     * the process finishes.
     */
    process get_return_object();

    /**
     * Register the process to be started immediately.
     *
     * Create an event, which is scheduled to be processed immediately. When
     * this event is processed, the process is started.
     *
     * @return Awaitable for the initial event.
     */
    await_event initial_suspend();

    /**
     * Trigger the underlying event since the process finished.
     *
     * @return Awaitable which is always ready.
     */
    std::suspend_never final_suspend() noexcept;

    /// Does nothing.
    void unhandled_exception();

    /**
     * Transform the given event to an await_event awaitable.
     *
     * Called when using co_await on an event.
     *
     * @param ev Event to wait for.
     * @return Awaitable for the given event.
     */
    await_event await_transform(std::shared_ptr<event> ev);

    /**
     * Transform the given process to an await_event awaitable.
     *
     * Called when using co_await on a process.
     *
     * @param proc Process to wait for.
     * @return Awaitable for the underlying event of the process, which is
     * triggered when the process finishes.
     */
    await_event await_transform(process proc);

  private:
    /// Refernece to the simulation.
    simulation &sim;

    /// Underlying event which is triggered when the process finishes.
    std::shared_ptr<event> proc_ev;
  };

private:
  /// Underlying event which is triggered when the process finishes.
  std::shared_ptr<event> ev;

  /**
   * Construct a new process object
   *
   * @param ev Underlying event which is triggered when the process finishes.
   */
  process(std::shared_ptr<event> ev);
};
} // namespace simcpp20
