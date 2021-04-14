// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <coroutine>

#include "simulation.hpp"

namespace simcpp20 {
class event;

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
    explicit promise_type(simulation &sim, Args &&...)
        : sim(sim), proc_ev(sim.event()) {}

    /**
     * @return process instance containing an event which will be triggered when
     * the process finishes.
     */
    process get_return_object();

    /**
     * Register the process to be started immediately via an initial event.
     *
     * @return Initial event.
     */
    event initial_suspend();

    /**
     * Trigger the underlying event since the process finished.
     *
     * @return Awaitable which is always ready.
     */
    std::suspend_never final_suspend() noexcept;

    /// No-op.
    void unhandled_exception();

  private:
    /// Refernece to the simulation.
    simulation &sim;

    /// Underlying event which is triggered when the process finishes.
    event proc_ev;
  };

  event operator co_await();

private:
  /// Underlying event which is triggered when the process finishes.
  event ev;

  /**
   * Construct a new process object
   *
   * @param ev Underlying event which is triggered when the process finishes.
   */
  explicit process(event ev);
};
} // namespace simcpp20
