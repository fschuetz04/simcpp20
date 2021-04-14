// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "event.hpp"

namespace simcpp20 {
event::event(simulation &sim) : sim(sim) {}

event::~event() {
  for (auto &handle : handles) {
    handle.destroy();
  }
}

void event::trigger() {
  if (triggered()) {
    return;
  }

  state = event_state::triggered;
  sim.schedule(0, shared_from_this());
}

void event::trigger_delayed(simtime delay) {
  if (triggered()) {
    return;
  }

  sim.schedule(delay, shared_from_this());
}

void event::add_callback(std::function<void(std::shared_ptr<event>)> cb) {
  if (processed()) {
    return;
  }

  cbs.emplace_back(cb);
}

bool event::pending() { return state == event_state::pending; }

bool event::triggered() {
  return state == event_state::triggered || state == event_state::processed;
}

bool event::processed() { return state == event_state::processed; }

void event::process() {
  if (processed()) {
    return;
  }

  state = event_state::processed;

  for (auto &handle : handles) {
    handle.resume();
  }

  handles.clear();

  for (auto &cb : cbs) {
    cb(shared_from_this());
  }

  cbs.clear();
}

void event::add_handle(std::coroutine_handle<> h) {
  if (processed()) {
    return;
  }

  handles.emplace_back(h);
}
} // namespace simcpp20
