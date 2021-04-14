// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "process.hpp"

#include "await_event.hpp"

namespace simcpp20 {
// process::promise_type

process process::promise_type::get_return_object() {
  return proc_ev;
}

await_event process::promise_type::initial_suspend() {
  return sim.timeout(0);
}

std::suspend_never process::promise_type::final_suspend() noexcept {
  proc_ev.trigger();
  return {};
}

void process::promise_type::unhandled_exception() {}

await_event process::promise_type::await_transform(event ev) {
  return ev;
}

await_event process::promise_type::await_transform(process proc) {
  return proc.ev;
}

// process

process::process(event ev) : ev(ev) {}
} // namespace simcpp20
