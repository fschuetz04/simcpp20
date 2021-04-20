// Copyright © 2021 Felix Schütz.
// Licensed under the MIT license. See the LICENSE file for details.

#include <catch2/catch_test_macros.hpp>

#include "simcpp20.hpp"

simcpp20::event awaiter(simcpp20::simulation &sim, simcpp20::event ev,
                        double target, bool &finished) {
  REQUIRE(sim.now() == 0);
  co_await ev;
  REQUIRE(sim.now() == target);
  finished = true;
};

TEST_CASE("any_of") {
  simcpp20::simulation sim;

  SECTION("any_of is triggered when the first event is processed") {
    auto ev = sim.any_of({sim.timeout(1), sim.timeout(2)});
    double target = 1;
    bool finished = false;
    awaiter(sim, ev, target, finished);

    sim.run();

    REQUIRE(finished);
  }
}

TEST_CASE("all_of") {
  simcpp20::simulation sim;

  SECTION("all_of is triggered when all events are processed") {
    auto ev = sim.all_of({sim.timeout(1), sim.timeout(2)});
    double target = 2;
    bool finished = false;
    awaiter(sim, ev, target, finished);

    sim.run();

    REQUIRE(finished);
  }
}
