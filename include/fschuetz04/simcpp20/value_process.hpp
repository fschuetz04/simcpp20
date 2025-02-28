#pragma once

#include <coroutine>  // std::coroutine_handle
#include <functional> // std::function
#include <utility>    // std::forward

#include "process.hpp"
#include "promise_type.hpp"
#include "value_event.hpp"

namespace simcpp20 {

template <typename Time> class simulation;

/**
 * A running process (coroutine) that returns a value when it completes. Like
 * process<>, it can be awaited.
 *
 * @tparam Value Type of the value returned when the process completes.
 * @tparam Time Type used for simulation time.
 */
template <typename Value, typename Time = double> class value_process {
public:
  /**
   * Constructor.
   *
   * @param simulation Reference to the simulation.
   */
  explicit value_process(simulation<Time> &sim) : event_{sim} {}

  /**
   * Abort the process.
   */
  void abort() const { event_.abort(); }

  /**
   * Check if the process is pending.
   *
   * @return Whether the process is pending.
   */
  bool pending() const { return event_.pending(); }

  /**
   * Check if the process is triggered or processed.
   *
   * @return Whether the process is triggered or processed.
   */
  bool triggered() const { return event_.triggered(); }

  /**
   * Check if the process is processed.
   *
   * @return Whether the process is processed.
   */
  bool processed() const { return event_.processed(); }

  /**
   * Check if the process is aborted.
   *
   * @return Whether the process is aborted.
   */
  bool aborted() const { return event_.aborted(); }

  /**
   * Add a callback to be called when the process is processed.
   *
   * @param cb Callback to be called when the process is processed.
   */
  void
  add_callback(std::function<void(const value_event<Value, Time> &)> cb) const {
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
   *
   * @return The value returned by the process.
   */
  Value await_resume() { return event_.value(); }

  /**
   * Get the value of the process.
   * @return The value of the process.
   */
  const Value &value() const { return event_.value(); }

  /**
   * Value processes can only be triggered by the framework internally, not by
   * user code. Value processes are triggered automatically when the coroutine
   * finishes.
   */
  friend class promise_type;
  // Make simulation able to access protected methods
  friend class simulation<Time>;

protected:
  /**
   * Trigger the process with a value.
   * This method is protected to prevent direct calls from user code.
   */
  template <typename... Args> void trigger(Args &&...args) const {
    event_.trigger(std::forward<Args>(args)...);
  }

  /**
   * Get the internal event.
   * This method is protected to prevent direct access from user code.
   */
  const value_event<Value, Time> &get_event() const { return event_; }

  /// Internal event used to implement process behavior
  value_event<Value, Time> event_;

private:
  using generic_promise_type = internal::generic_promise_type<Time>;

public:
  /// Promise type for a coroutine returning a value_process.
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
     * @return Process associated with the coroutine. This process acts like
     * an event triggered when the coroutine returns.
     */
    value_process<Value, Time> get_return_object() const { return proc_; }

    /**
     * Called when the coroutine returns. Trigger the process associated with
     * the coroutine with the returned value.
     *
     * @tparam Args Types of arguments to construct the return value with.
     * @param args Arguments to construct the return value with.
     */
    template <typename... Args> void return_value(Args &&...args) const {
      proc_.trigger(std::forward<Args>(args)...);
    }

    /**
     * Value process associated with the coroutine. This process acts like an
     * event triggered when the coroutine returns with the value the coroutine
     * returns.
     */
    value_process<Value, Time> proc_;
  };
};

} // namespace simcpp20
