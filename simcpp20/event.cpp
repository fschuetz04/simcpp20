// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "event.hpp"

#include "simulation.hpp"

namespace simcpp20 {
// event

event::event(simulation &sim) : shared{std::make_shared<shared_state>(sim)} {}

void event::trigger() {
  if (!pending()) {
    return;
  }

  shared->state = event_state::triggered;
  shared->sim.schedule(0, *this);
}

void event::trigger_delayed(simtime delay) {
  if (!pending()) {
    return;
  }

  shared->sim.schedule(delay, *this);
}

void event::abort() {
  if (!pending()) {
    return;
  }

  shared->state = event_state::aborted;

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
  return shared->state == event_state::pending;
}

bool event::triggered() {
  return shared->state == event_state::triggered ||
         shared->state == event_state::processed;
}

bool event::processed() {
  return shared->state == event_state::processed;
}

bool event::aborted() {
  return shared->state == event_state::aborted;
}

bool event::await_ready() {
  return processed();
}

void event::await_suspend(std::coroutine_handle<> handle) {
  shared->handles.emplace_back(handle);
  shared = nullptr;
}

void event::await_resume() {}

void event::process() {
  if (processed() || aborted()) {
    return;
  }

  shared->state = event_state::processed;

  for (auto &handle : shared->handles) {
    handle.resume();
  }
  shared->handles.clear();

  for (auto &cb : shared->cbs) {
    cb(*this);
  }
  shared->cbs.clear();
}

// event::shared_state

event::shared_state::shared_state(simulation &sim) : sim(sim) {}

event::shared_state::~shared_state() {
  for (auto &handle : handles) {
    handle.destroy();
  }
}
} // namespace simcpp20
