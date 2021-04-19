// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "event.hpp"

#include "simulation.hpp"

namespace simcpp20 {
event::event(simulation &sim) : shared{std::make_shared<data>(sim)} {}

void event::trigger() {
  if (!pending()) {
    return;
  }

  shared->sim.schedule(*this);
  shared->state = state::triggered;
}

void event::abort() {
  if (!pending()) {
    return;
  }

  shared->state = state::aborted;

  for (auto &handle : shared->handles) {
    handle.destroy();
  }
  shared->handles.clear();

  shared->cbs.clear();
}

void event::add_callback(std::function<void(event &)> cb) {
  if (processed() || aborted()) {
    return;
  }

  shared->cbs.emplace_back(cb);
}

bool event::pending() {
  return shared->state == state::pending;
}

bool event::triggered() {
  return shared->state == state::triggered || processed();
}

bool event::processed() {
  return shared->state == state::processed;
}

bool event::aborted() {
  return shared->state == state::aborted;
}

bool event::await_ready() {
  return processed();
}

void event::await_suspend(std::coroutine_handle<> handle) {
  assert(!processed());

  if (!aborted() && !processed()) {
    shared->handles.push_back(handle);
  }

  shared = nullptr;
}

void event::await_resume() {}

void event::process() {
  if (processed() || aborted()) {
    return;
  }

  shared->state = state::processed;

  for (auto &handle : shared->handles) {
    handle.resume();
  }
  shared->handles.clear();

  for (auto &cb : shared->cbs) {
    cb(*this);
  }
  shared->cbs.clear();
}

event::data::data(simulation &sim) : sim{sim} {}

event::data::~data() {
  for (auto &handle : handles) {
    handle.destroy();
  }
}

event event::promise_type::get_return_object() {
  return ev;
}

event event::promise_type::initial_suspend() {
  event ev{sim};
  sim.schedule(ev);
  return ev;
}

std::suspend_never event::promise_type::final_suspend() noexcept {
  return {};
}

void event::promise_type::unhandled_exception() {}

void event::promise_type::return_void() {
  ev.trigger();
}
} // namespace simcpp20
