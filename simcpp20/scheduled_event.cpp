// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "scheduled_event.hpp"

namespace simcpp20 {
scheduled_event::scheduled_event(simtime time, id_type id, event ev)
    : time_(time), id_(id), ev_(ev) {}

bool scheduled_event::operator>(const scheduled_event &other) const {
  if (time() != other.time()) {
    return time() > other.time();
  }

  return id_ > other.id_;
}

simtime scheduled_event::time() const {
  return time_;
}

event scheduled_event::ev() {
  return ev_;
}
} // namespace simcpp20
