// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <coroutine>
#include <functional>
#include <memory>
#include <queue>
#include <vector>

namespace simcpp20 {
using simtime = double;
class scheduled_event;
class event;
struct await_event;

class simulation {
public:
  std::shared_ptr<simcpp20::event> timeout(simtime delay);

  std::shared_ptr<simcpp20::event> event();

  void schedule(simtime time, std::shared_ptr<simcpp20::event> ev);

  void step();

  void run();

  void run_until(simtime target);

  simtime now();

private:
  simtime _now = 0;
  std::priority_queue<scheduled_event> scheduled_evs = {};
};

class scheduled_event {
public:
  scheduled_event(simtime time, std::shared_ptr<event> ev);

  bool operator<(const scheduled_event &other) const;

  simtime time() const;

  std::shared_ptr<event> ev();

private:
  simtime _time;
  std::shared_ptr<event> _ev;
};

enum class event_state { pending, triggered, processed };

class event : public std::enable_shared_from_this<event> {
public:
  event(simulation &sim);

  void trigger();

  void process();

  void add_handle(std::coroutine_handle<> handle);

  bool pending();

  bool triggered();

  bool processed();

private:
  event_state state = event_state::pending;
  std::vector<std::coroutine_handle<>> handles = {};
  simulation &sim;
};

struct await_event {
  std::shared_ptr<event> ev;

  await_event(std::shared_ptr<event> ev);

  bool await_ready();

  void await_suspend(std::coroutine_handle<> handle);

  void await_resume();
};

struct process {
  struct promise_type {
    simulation &sim;

    template <typename... Args>
    promise_type(simulation &sim, Args &&...args) : sim(sim) {}

    process get_return_object();

    await_event initial_suspend();

    std::suspend_never final_suspend();

    void unhandled_exception();

    await_event await_transform(std::shared_ptr<event> ev);
  };
};
} // namespace simcpp20
