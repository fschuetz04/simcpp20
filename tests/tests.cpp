// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "fschuetz04/simcpp20.hpp"

#include <string> // std::string

simcpp20::event<> awaiter(simcpp20::simulation<> &sim, simcpp20::event<> ev,
                          double expected_time, bool &finished) {
  REQUIRE(sim.now() == 0);
  co_await ev;
  REQUIRE(sim.now() == expected_time);
  finished = true;
};

simcpp20::event<> ref_awaiter(simcpp20::simulation<> &sim,
                              simcpp20::event<> &ev, double expected_time,
                              bool &finished) {
  REQUIRE(sim.now() == 0);
  co_await ev;
  REQUIRE(sim.now() == expected_time);
  finished = true;
}

TEST_CASE("an aborted process does not run") {
  simcpp20::simulation<> sim;

  auto ev = sim.timeout(1);
  bool finished = false;
  auto process_ev = awaiter(sim, ev, -1, finished);
  process_ev.abort();

  sim.run();

  REQUIRE(!finished);
}

TEST_CASE("multiple processes can await the same event reference") {
  simcpp20::simulation<> sim;

  auto ev = sim.timeout(1);
  bool finished_a = false;
  ref_awaiter(sim, ev, 1, finished_a);
  bool finished_b = false;
  ref_awaiter(sim, ev, 1, finished_b);

  sim.run();

  REQUIRE(finished_a);
  REQUIRE(finished_b);
}

TEST_CASE("any_of") {
  simcpp20::simulation<> sim;

  SECTION("any_of is not triggered when all events are never processed") {
    auto ev = sim.any_of({sim.event(), sim.event()});
    bool finished = false;
    awaiter(sim, ev, -1, finished);

    sim.run();

    REQUIRE(!finished);
  }

  double a = GENERATE(1, 2);
  auto ev_a = sim.timeout(a);
  auto ev_b = sim.timeout(3 - a);

  SECTION("any_of is triggered when the first event is processed") {
    auto ev = sim.any_of({ev_a, ev_b});
    bool finished = false;
    awaiter(sim, ev, 1, finished);

    sim.run();

    REQUIRE(finished);
  }

  SECTION("| is an alias for any_of") {
    auto ev = ev_a | ev_b;
    bool finished = false;
    awaiter(sim, ev, 1, finished);

    sim.run();

    REQUIRE(finished);
  }
}

template <typename Value>
simcpp20::event<>
value_awaiter(simcpp20::simulation<> &sim, simcpp20::value_event<Value> ev,
              Value expected_value, double expected_time, bool &finished) {
  REQUIRE(sim.now() == 0);
  Value value = co_await ev;
  REQUIRE(value == expected_value);
  REQUIRE(sim.now() == expected_time);
  finished = true;
};

TEST_CASE("any_of with value_event") {
  simcpp20::simulation<> sim;

  double delay_a = GENERATE(1, 2);
  std::string value_a = "a";
  auto ev_a = sim.timeout<std::string>(delay_a, value_a);
  std::string value_b = "b";
  auto ev_b = sim.timeout<std::string>(3 - delay_a, value_b);

  SECTION("any_of is triggered when the first event is processed") {
    auto ev = sim.any_of({ev_a, ev_b});
    auto expected_value = delay_a == 1 ? value_a : value_b;
    bool finished = false;
    value_awaiter(sim, ev, expected_value, 1, finished);

    sim.run();

    REQUIRE(finished);
  }

  SECTION("| is an alias for any_of") {
    auto ev = ev_a | ev_b;
    auto expected_value = delay_a == 1 ? value_a : value_b;
    bool finished = false;
    value_awaiter(sim, ev, expected_value, 1, finished);

    sim.run();

    REQUIRE(finished);
  }
}

TEST_CASE("all_of") {
  simcpp20::simulation<> sim;

  SECTION("all_of is not triggered when one event is never processed") {
    auto ev = sim.all_of({sim.timeout(1), sim.event()});
    bool finished = false;
    awaiter(sim, ev, -1, finished);

    sim.run();

    REQUIRE(!finished);
  }

  double delay_a = GENERATE(1, 2);
  auto ev_a = sim.timeout(delay_a);
  auto ev_b = sim.timeout(3 - delay_a);

  SECTION("all_of is triggered when all events are processed") {
    auto ev = sim.all_of({ev_a, ev_b});
    bool finished = false;
    awaiter(sim, ev, 2, finished);

    sim.run();

    REQUIRE(finished);
  }

  SECTION("& is an alias for any_of") {
    auto ev = ev_a & ev_b;
    bool finished = false;
    awaiter(sim, ev, 2, finished);

    sim.run();

    REQUIRE(finished);
  }
}
