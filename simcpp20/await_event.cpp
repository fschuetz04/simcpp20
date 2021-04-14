// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "await_event.hpp"

namespace simcpp20 {
await_event::await_event(std::shared_ptr<event> ev) : ev(ev) {}

bool await_event::await_ready() { return ev->processed(); }

void await_event::await_suspend(std::coroutine_handle<> handle) {
  ev->add_handle(handle);
  ev = nullptr;
}

void await_event::await_resume() {}
} // namespace simcpp20
