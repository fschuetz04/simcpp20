#include "catch2/catch_test_macros.hpp"
#include "fschuetz04/simcpp20.hpp"
#include "fschuetz04/simcpp20/simulation.hpp"

#include <vector> // std::vector

TEST_CASE("store") {
  SECTION("immediate get returns value") {
    simcpp20::simulation<> sim;
    simcpp20::store<int> store(sim);

    store.put(42);
    auto ev = store.get();

    // for an immediately available value, the event should be triggered
    REQUIRE(ev.triggered());
    REQUIRE(ev.value() == 42);
  }

  SECTION("queued get is triggered after put") {
    simcpp20::simulation<> sim;
    simcpp20::store<int> store(sim);
    int result = 0;

    // process that gets a value and records it
    auto proc = [&store, &result](auto &) -> simcpp20::process<> {
      result = co_await store.get();
    };

    proc(sim);

    store.put(55);
    sim.run();

    REQUIRE(result == 55);
  }

  SECTION("capacity constraint re-queues extra puts") {
    simcpp20::simulation<> sim;
    simcpp20::store<int> store(sim, 1);
    auto put1 = store.put(100);
    auto put2 = store.put(200); // queued
    bool finished = false;

    REQUIRE(put1.triggered());
    REQUIRE(!put2.triggered());

    // process that gets a value, waits for put2, and then gets a second value
    auto proc = ([&put2, &store, &finished](auto &) -> simcpp20::process<> {
      auto value = co_await store.get();
      REQUIRE(value == 100);

      co_await put2;

      value = co_await store.get();
      REQUIRE(value == 200);

      finished = true;
    });

    proc(sim);

    sim.run();

    REQUIRE(finished);
  }

  SECTION("multiple queued gets are triggered in order") {
    simcpp20::simulation<> sim;
    simcpp20::store<int> store(sim);
    std::vector<int> results;

    // define a process that gets a value and records it
    auto consumer = [&store, &results](auto &) -> simcpp20::process<> {
      auto ev = store.get();
      results.push_back(ev.value());
      co_return;
    };

    consumer(sim);
    consumer(sim);
    consumer(sim);

    store.put(10);
    store.put(20);
    store.put(30);

    sim.run();
    REQUIRE(results == std::vector<int>{10, 20, 30});
  }
}
