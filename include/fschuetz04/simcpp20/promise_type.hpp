#pragma once

#include <cassert>   // assert
#include <coroutine> // std::coroutine_handle, std::suspend_never

namespace simcpp20 {
template <typename Time> class simulation;
template <typename Time> class event;

namespace internal {
/**
 * Abstract base class for all promise types.
 *
 * @tparam Time Type used for simulation time.
 */
template <typename Time = double> class generic_promise_type {
public:
  /// Constructor.
  generic_promise_type(simulation<Time> &sim, std::coroutine_handle<> handle)
      : sim_{sim}, handle_{handle} {
    sim_.handles_.insert(handle_);
  }

  /// Destructor.
  virtual ~generic_promise_type() { sim_.handles_.erase(handle_); };

  /// @return Whether this process is aborted.
  virtual bool is_aborted() const = 0;

  /// @return Coroutine handle associated with the process.
  std::coroutine_handle<> process_handle() const { return handle_; }

  /**
   * Called when the coroutine is started. The coroutine awaits the return
   * value before running.
   *
   * @return Event which will be processed at the current simulation time.
   */
  event<Time> initial_suspend() const { return sim_.timeout(Time{0}); }

  /// Called when an exception is thrown inside the coroutine and not handled.
  void unhandled_exception() const { assert(false); }

  /**
   * Called after the coroutine returns.
   *
   * @return Awaitable which is always ready.
   */
  std::suspend_never final_suspend() const noexcept { return {}; }

  /// Reference to the simulation.
  simulation<Time> &sim_;

  /// Coroutine handle.
  std::coroutine_handle<> handle_;
};
} // namespace internal
} // namespace simcpp20
