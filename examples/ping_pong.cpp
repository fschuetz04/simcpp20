#include <cstdio>

#include "fschuetz04/simcpp20.hpp" // IWYU pragma: export

struct ev_type;
using ev_inner = simcpp20::value_event<ev_type>;
struct ev_type {
  ev_inner ev;
};

simcpp20::process<> party(simcpp20::simulation<> &sim, const char *name,
                          simcpp20::value_event<ev_type> my_event,
                          double delay) {
  while (true) {
    auto their_event = (co_await my_event).ev;
    printf("[%.0f] %s\n", sim.now(), name);
    co_await sim.timeout(delay);
    my_event = sim.event<ev_type>();
    their_event.trigger(ev_type{my_event});
  }
}

int main() {
  simcpp20::simulation<> sim;
  auto pong_event = sim.event<ev_type>();
  auto ping_event = sim.timeout<ev_type>(0, ev_type{pong_event});
  party(sim, "ping", ping_event, 1);
  party(sim, "pong", pong_event, 2);
  sim.run_until(8);
}
