// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "event.hpp"

#include "simulation.hpp"

namespace simcpp20 {
event event::promise_type::get_return_object() {
  return ev;
}

event event::promise_type::initial_suspend() {
  return sim.timeout(0);
}

std::suspend_never event::promise_type::final_suspend() noexcept {
  ev.trigger();
  return {};
}

void event::promise_type::unhandled_exception() {}

void event::promise_type::return_void() {}
} // namespace simcpp20
