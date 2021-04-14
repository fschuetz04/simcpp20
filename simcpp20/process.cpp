// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "process.hpp"

namespace simcpp20 {
// process::promise_type

process process::promise_type::get_return_object() {
  return process{proc_ev};
}

event process::promise_type::initial_suspend() {
  return sim.timeout(0);
}

std::suspend_never process::promise_type::final_suspend() noexcept {
  proc_ev.trigger();
  return {};
}

void process::promise_type::unhandled_exception() {}

// process

process::process(event ev) : ev(ev) {}
} // namespace simcpp20
