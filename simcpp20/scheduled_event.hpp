// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <memory>

#include "event.hpp"
#include "types.hpp"

namespace simcpp20 {
class event;

/// One event in the event queue of simulation.
class scheduled_event {
public:
  /**
   * Construct a new scheduled event.
   *
   * @param time Time at which to process the event.
   * @param id Incremental ID to sort events scheduled at the same time by
   * insertion order.
   * @param ev Event to process.
   */
  scheduled_event(simtime time, id_type id, std::shared_ptr<event> ev);

  /**
   * @param other Scheduled event to compare to.
   * @return Whether this event is scheduled before the given event.
   */
  bool operator>(const scheduled_event &other) const;

  /// @return Time at which to process the event.
  simtime time() const;

  /// @return Event to process.
  std::shared_ptr<event> ev();

private:
  /// Time at which to process the event.
  simtime time_;

  /**
   * Incremental ID to sort events scheduled at the same time by insertion
   * order.
   */
  id_type id_;

  /// Event to process.
  std::shared_ptr<event> ev_;
};
} // namespace simcpp20
