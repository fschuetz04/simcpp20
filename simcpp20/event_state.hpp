// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

namespace simcpp20 {
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
} // namespace simcpp20
