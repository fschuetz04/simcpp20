// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "event.hpp"

namespace simcpp20 {
event::shared_state::shared_state(simulation &sim) : sim(sim) {}

event::shared_state::~shared_state() {
  for (auto &handle : handles) {
    handle.destroy();
  }
}
} // namespace simcpp20
