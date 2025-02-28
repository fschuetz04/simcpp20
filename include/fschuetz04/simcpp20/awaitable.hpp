#pragma once

#include <functional> // std::function

namespace simcpp20 {

/// An interface for objects that can be awaited.
class awaitable {
public:
  /// Destructor.
  virtual ~awaitable() = default;

  /// @return Whether the awaitable is pending.
  virtual bool pending() const = 0;

  /// @return Whether the event is triggered or processed.
  virtual bool triggered() const = 0;

  /// @return Whether the awaitable is processed.
  virtual bool processed() const = 0;

  /// @return Whether the awaitable is aborted.
  virtual bool aborted() const = 0;

  /**
   * Add a callback to be called when the awaitable is processed.
   *
   * @param cb Callback.
   */
  virtual void add_callback(std::function<void()> cb) const = 0;
};

} // namespace simcpp20
