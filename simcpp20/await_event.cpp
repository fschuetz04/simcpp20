// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "await_event.hpp"

namespace simcpp20 {
await_event::await_event(event ev) : ev(ev) {}

bool await_event::await_ready() {
  return ev.processed();
}

void await_event::await_suspend(std::coroutine_handle<> handle) {
  // TODO(fschuetz04): how to decrease ref count of ev::shared?
  ev.add_handle(handle);
}

void await_event::await_resume() {}
} // namespace simcpp20
