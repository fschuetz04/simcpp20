// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "simcpp20.hpp"

namespace simcpp20 {
// simulation

event_ptr simulation::any_of(std::initializer_list<event_ptr> evs) {
  for (auto &ev : evs) {
    if (ev->processed()) {
      return timeout(0);
    }
  }

  auto any_of_ev = event();

  for (auto &ev : evs) {
    ev->add_callback([any_of_ev](event_ptr) { any_of_ev->trigger(); });
  }

  return any_of_ev;
}

event_ptr simulation::all_of(std::initializer_list<event_ptr> evs) {
  int n = evs.size();

  for (auto &ev : evs) {
    if (ev->processed()) {
      --n;
    }
  }

  if (n == 0) {
    return timeout(0);
  }

  auto all_of_ev = event();
  auto n_ptr = std::make_shared<int>(n);

  for (auto &ev : evs) {
    ev->add_callback([all_of_ev, n_ptr](event_ptr) {
      --*n_ptr;
      if (*n_ptr == 0) {
        all_of_ev->trigger();
      }
    });
  }

  return all_of_ev;
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

void simulation::schedule(simtime delay, event_ptr ev) {
  scheduled_evs.emplace(now() + delay, ev);
}

// scheduled_event

scheduled_event::scheduled_event(simtime time, event_ptr ev)
    : _time(time), _ev(ev) {}

bool scheduled_event::operator<(const scheduled_event &other) const {
  return time() > other.time();
}

simtime scheduled_event::time() const { return _time; }

event_ptr scheduled_event::ev() { return _ev; }

// event

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

void event::add_callback(std::function<void(event_ptr)> cb) {
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

// await_event

await_event::await_event(event_ptr ev) : ev(ev) {}

bool await_event::await_ready() { return ev->processed(); }

void await_event::await_suspend(std::coroutine_handle<> handle) {
  ev->add_handle(handle);
  ev = nullptr;
}

void await_event::await_resume() {}

// process

process::process(event_ptr ev) : ev(ev) {}

// process::promise_type

process process::promise_type::get_return_object() { return proc_ev; }

await_event process::promise_type::initial_suspend() { return sim.timeout(0); }

std::suspend_never process::promise_type::final_suspend() noexcept {
  proc_ev->trigger();
  return {};
}

void process::promise_type::unhandled_exception() {}

await_event process::promise_type::await_transform(event_ptr ev) { return ev; }

await_event process::promise_type::await_transform(process proc) {
  return proc.ev;
}
} // namespace simcpp20
