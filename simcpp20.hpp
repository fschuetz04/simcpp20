// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#pragma once

#include <coroutine>
#include <functional>
#include <memory>
#include <queue>
#include <vector>

namespace simcpp20 {
class scheduled_event;
class event;
struct await_event;
using simtime = double;
using event_ptr = std::shared_ptr<simcpp20::event>;

class simulation {
public:
  event_ptr timeout(simtime delay);

  event_ptr event();

  event_ptr any_of(std::initializer_list<event_ptr> evs);

  event_ptr all_of(std::initializer_list<event_ptr> evs);

  void schedule(simtime delay, event_ptr ev);

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
  scheduled_event(simtime time, event_ptr ev);

  bool operator<(const scheduled_event &other) const;

  simtime time() const;

  event_ptr ev();

private:
  simtime _time;
  event_ptr _ev;
};

enum class event_state { pending, triggered, processed };

class event : public std::enable_shared_from_this<event> {
public:
  event(simulation &sim);

  ~event();

  void trigger();

  void process();

  void add_handle(std::coroutine_handle<> handle);

  void add_callback(std::function<void(event_ptr)> cb);

  bool pending();

  bool triggered();

  bool processed();

private:
  event_state state = event_state::pending;
  std::vector<std::coroutine_handle<>> handles = {};
  std::vector<std::function<void(event_ptr)>> cbs = {};
  simulation &sim;
};

struct await_event {
  event_ptr ev;

  await_event(event_ptr ev);

  bool await_ready();

  void await_suspend(std::coroutine_handle<> handle);

  void await_resume();
};

class process {
public:
  class promise_type {
  public:
    template <typename... Args>
    promise_type(simulation &sim, Args &&...args) : sim(sim), ev(sim.event()) {}

    process get_return_object();

    await_event initial_suspend();

    std::suspend_never final_suspend();

    void unhandled_exception();

    await_event await_transform(event_ptr ev);

    await_event await_transform(process proc);

  private:
    simulation &sim;
    event_ptr ev;
  };

  process(event_ptr ev);

private:
  event_ptr ev;
};
} // namespace simcpp20
