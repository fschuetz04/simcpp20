#pragma once

#include <cassert>    // assert
#include <coroutine>  // std::coroutine_handle
#include <functional> // std::function

#include "awaitable.hpp"
#include "event.hpp"
#include "promise_type.hpp"

namespace simcpp20 {

template <typename Time> class simulation;
template <typename Value, typename Time> class value_process;

/**
 * A running process (coroutine) that can be awaited.
 *
 * @tparam Time Type used for simulation time.
 */
template <typename Time = double> class process : public awaitable {
public:
  /**
   * Constructor.
   *
   * @param simulation Reference to the simulation.
   */
  explicit process(simulation<Time> &sim) : event_{sim} {}

  /**
   * Abort the process.
   */
  void abort() const { event_.abort(); }

  /// @return Whether the process is pending.
  bool pending() const override { return event_.pending(); }

  /// @return Whether the process is triggered or processed.
  bool triggered() const override { return event_.triggered(); }

  /// @return Whether the process is processed.
  bool processed() const override { return event_.processed(); }

  /// @return Whether the process is aborted.
  bool aborted() const override { return event_.aborted(); }

  /**
   * Add a callback to be called when the awaitable is processed.
   *
   * @param cb Callback.
   */
  void add_callback(std::function<void()> cb) const override {
    event_.add_callback(cb);
  }

  /**
   * Called when using co_await on the process. The calling coroutine is only
   * suspended if this method returns false.
   *
   * @return Whether the process is processed.
   */
  bool await_ready() const { return event_.processed(); }

  /**
   * Called when a coroutine is suspended after using co_await on the process.
   *
   * @tparam Promise Promise type of the coroutine.
   * @param handle Coroutine handle.
   */
  template <typename Promise>
  void await_suspend(std::coroutine_handle<Promise> handle) {
    event_.await_suspend(handle);
  }

  /**
   * Called when a coroutine is resumed after using co_await on the process or
   * if the coroutine did not need to be suspended.
   */
  void await_resume() { event_.await_resume(); }

  // TODO: remove?
  // Make value_process able to access the process internals
  template <typename Value, typename T> friend class value_process;

  // Make simulation able to access protected methods
  friend class simulation<Time>;

  /**
   * Alias for simulation::any_of.
   *
   * @param other Other awaitable.
   * @return New pending event which is triggered when this process or the other
   * awaitable is processed.
   */
  template <typename Awaitable>
  event<Time> operator|(const Awaitable &other) const {
    return event_ | other;
  }

  /**
   * Alias for simulation::all_of.
   *
   * @param other Other awaitable.
   * @return New pending event which is triggered when this process and the
   * other awaitable are processed.
   */
  template <typename Awaitable>
  event<Time> operator&(const Awaitable &other) const {
    return event_ & other;
  }

protected:
  /**
   * Trigger the process.
   * This method is protected to prevent direct calls from user code.
   */
  void trigger() const { event_.trigger(); }

  /// Internal event used to implement process behavior
  event<Time> event_;

private:
  using generic_promise_type = internal::generic_promise_type<Time>;

public:
  /// Promise type for a coroutine returning a process.
  class promise_type : public generic_promise_type {
  public:
    using handle_type = std::coroutine_handle<promise_type>;

    /**
     * Constructor.
     *
     * @tparam Args Types of additional arguments passed to the coroutine
     * function.
     * @param sim Reference to the simulation.
     */
    template <typename... Args>
    explicit promise_type(simulation<Time> &sim, Args &&...)
        : generic_promise_type{sim, handle_type::from_promise(*this)},
          proc_{sim} {}

    /**
     * Constructor.
     *
     * @tparam Class Class type if the coroutine function is a lambda or a
     * member function of a class.
     * @tparam Args Types of additional arguments passed to the coroutine
     * function.
     * @param sim Reference to the simulation.
     */
    template <typename Class, typename... Args>
    explicit promise_type(Class &&, simulation<Time> &sim, Args &&...)
        : generic_promise_type{sim, handle_type::from_promise(*this)},
          proc_{sim} {}

    /**
     * Constructor.
     *
     * @tparam Class Class type if the coroutine function is a member function
     * of a class. Must contain a member variable sim referencing the simulation
     * instance.
     * @tparam Args Types of additional arguments passed to the coroutine
     * function.
     * @param c Class instance.
     */
    template <typename Class, typename... Args>
    explicit promise_type(Class &&c, Args &&...)
        : generic_promise_type{c.sim, handle_type::from_promise(*this)},
          proc_{c.sim} {}

#ifdef __INTELLISENSE__
    // IntelliSense fix. See https://stackoverflow.com/q/67209981.
    promise_type();
#endif

    /// Destructor.
    ~promise_type() override = default;

    /// @return Whether this process is aborted.
    bool is_aborted() const override { return proc_.aborted(); }

    /**
     * Called to get the return value of the coroutine function.
     *
     * @return Process associated with the coroutine. This process acts like an
     * event which is triggered when the coroutine returns.
     */
    process<Time> get_return_object() const { return proc_; }

    /**
     * Called when the coroutine returns. Trigger the process associated with
     * the coroutine.
     */
    void return_void() const { proc_.trigger(); }

    /**
     * Process associated with the coroutine. This process acts like an event
     * which is triggered when the coroutine returns.
     */
    process<Time> proc_;
  };
};

} // namespace simcpp20
