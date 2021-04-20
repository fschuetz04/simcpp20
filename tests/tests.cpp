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
    auto first_ev = sim.any_of({sim.timeout(1), sim.timeout(2)});
    bool first_finished = false;
    awaiter(sim, first_ev, 1, first_finished);

    auto second_ev = sim.any_of({sim.timeout(2), sim.timeout(1)});
    bool second_finished = false;
    awaiter(sim, second_ev, 1, second_finished);

    sim.run();

    REQUIRE(first_finished);
    REQUIRE(second_finished);
  }

  SECTION("| is an alias for any_of") {
    auto first_ev = sim.timeout(1) | sim.timeout(2);
    auto first_finished = false;
    awaiter(sim, first_ev, 1, first_finished);

    auto second_ev = sim.timeout(2) | sim.timeout(1);
    auto second_finished = false;
    awaiter(sim, second_ev, 1, second_finished);

    sim.run();

    REQUIRE(first_finished);
    REQUIRE(second_finished);
  }
}

TEST_CASE("all_of") {
  simcpp20::simulation sim;

  SECTION("all_of is triggered when all events are processed") {
    auto first_ev = sim.all_of({sim.timeout(1), sim.timeout(2)});
    bool first_finished = false;
    awaiter(sim, first_ev, 2, first_finished);

    auto second_ev = sim.all_of({sim.timeout(2), sim.timeout(1)});
    bool second_finished = false;
    awaiter(sim, second_ev, 2, second_finished);

    sim.run();

    REQUIRE(first_finished);
    REQUIRE(second_finished);
  }

  SECTION("& is an alias for any_of") {
    auto first_ev = sim.timeout(1) & sim.timeout(2);
    auto first_finished = false;
    awaiter(sim, first_ev, 2, first_finished);

    auto second_ev = sim.timeout(2) & sim.timeout(1);
    auto second_finished = false;
    awaiter(sim, second_ev, 2, second_finished);

    sim.run();

    REQUIRE(first_finished);
    REQUIRE(second_finished);
  }
}
