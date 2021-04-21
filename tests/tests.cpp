// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include <algorithm>
#include <iostream>

#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "simcpp20.hpp"

simcpp20::event awaiter(simcpp20::simulation &sim, simcpp20::event ev,
                        double target, bool &finished) {
  REQUIRE(sim.now() == 0);
  co_await ev;
  REQUIRE(sim.now() == target);
  finished = true;
};

TEST_CASE("boolean logic") {
  simcpp20::simulation sim;

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
