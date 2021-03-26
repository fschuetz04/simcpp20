// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "simcpp20.hpp"

namespace simcpp20 {
std::shared_ptr<simcpp20::event> simulation::timeout(simtime delay) {
  auto ev = event();
  schedule(now() + delay, ev);
  return ev;
}

std::shared_ptr<simcpp20::event> simulation::event() {
  return std::make_shared<simcpp20::event>(*this);
}

void simulation::schedule(simtime time, std::shared_ptr<simcpp20::event> ev) {
  scheduled_evs.emplace(time, ev);
}

void simulation::step() {
  auto scheduled_ev = scheduled_evs.top();
  scheduled_evs.pop();

  _now = scheduled_ev.time();
  scheduled_ev.ev()->process();
}

void simulation::run() {
  while (!scheduled_evs.empty()) {
    step();
  }
}

void simulation::run_until(simtime target) {
  while (!scheduled_evs.empty() && scheduled_evs.top().time() <= target) {
    step();
  }
  _now = target;
}

simtime simulation::now() { return _now; }

scheduled_event::scheduled_event(simtime time, std::shared_ptr<event> ev)
    : _time(time), _ev(ev) {}

bool scheduled_event::operator<(const scheduled_event &other) const {
  return time() > other.time();
}

simtime scheduled_event::time() const { return _time; }

std::shared_ptr<event> scheduled_event::ev() { return _ev; }

event::event(simulation &sim) : sim(sim) {}

event::~event() {
  for (auto &handle : handles) {
    handle.destroy();
  }
}

void event::trigger() {
  state = event_state::triggered;
  sim.schedule(0, shared_from_this());
}

void event::process() {
  state = event_state::processed;
  for (auto &handle : handles) {
    handle.resume();
  }
  handles.clear();
}

void event::add_handle(std::coroutine_handle<> h) { handles.emplace_back(h); }

bool event::pending() { return state == event_state::pending; }

bool event::triggered() {
  return state == event_state::triggered || state == event_state::processed;
}

bool event::processed() { return state == event_state::processed; }

await_event::await_event(std::shared_ptr<event> ev) : ev(ev) {}

bool await_event::await_ready() { return ev->processed(); }

void await_event::await_suspend(std::coroutine_handle<> handle) {
  ev->add_handle(handle);
  ev = nullptr;
}

void await_event::await_resume() {}

process process::promise_type::get_return_object() { return {}; }

await_event process::promise_type::initial_suspend() { return sim.timeout(0); }

std::suspend_never process::promise_type::final_suspend() { return {}; }

void process::promise_type::unhandled_exception() {}

await_event process::promise_type::await_transform(std::shared_ptr<event> ev) {
  return ev;
}
} // namespace simcpp20
