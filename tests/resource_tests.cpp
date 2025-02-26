#include "catch2/catch_test_macros.hpp"
#include "fschuetz04/simcpp20.hpp"

#include <vector> // std::vector

TEST_CASE("resource basics") {
  simcpp20::simulation<> sim;

  SECTION("resource can be created with initial available count") {
    simcpp20::resource res(sim, 3);
    REQUIRE(res.available() == 3);
  }

  SECTION("requesting and releasing resources") {
    simcpp20::resource res(sim, 2);

    // request first resource
    auto req1 = res.request();
    REQUIRE(res.available() == 1);

    // request second resource
    auto req2 = res.request();
    REQUIRE(res.available() == 0);

    // release first resource
    res.release();
    REQUIRE(res.available() == 1);

    // release second resource
    res.release();
    REQUIRE(res.available() == 2);
  }
}

simcpp20::process<> resource_user(simcpp20::simulation<> &sim,
                                  simcpp20::resource &res, double use_time,
                                  std::vector<double> &usage_times) {
  // request the resource
  co_await res.request();

  // record when we got the resource
  usage_times.push_back(sim.now());

  // use the resource for some time
  co_await sim.timeout(use_time);

  // release the resource
  res.release();
}

TEST_CASE("resource queuing behavior") {
  simcpp20::simulation<> sim;
  std::vector<double> usage_times;

  SECTION("processes queue for limited resources") {
    simcpp20::resource res(sim, 1);

    // start three processes that want to use the resource
    // t=0, uses until t=10
    resource_user(sim, res, 10, usage_times);
    // queued, gets resource at t=10, uses until t=15
    resource_user(sim, res, 5, usage_times);
    // queued, gets resource at t=15, uses until t=18
    resource_user(sim, res, 3, usage_times);

    sim.run();

    REQUIRE(usage_times == std::vector<double>{0, 10, 15});
  }

  SECTION("multiple resources can serve multiple processes") {
    simcpp20::resource res(sim, 2);

    // Start three processes that want to use the resource
    // t=0, uses until t=10
    resource_user(sim, res, 10, usage_times);
    // t=0, uses until t=15
    resource_user(sim, res, 15, usage_times);
    // queued, gets resource at t=10, uses until t=15
    resource_user(sim, res, 5, usage_times);

    sim.run();

    REQUIRE(usage_times == std::vector<double>{0, 0, 10});
  }
}

TEST_CASE("resource with aborted requests") {
  simcpp20::simulation<> sim;
  std::vector<double> usage_times;

  SECTION("process with timeout can abort resource request") {
    simcpp20::resource res(sim, 1);
    bool second_timed_out = false;

    // t=0, uses until t=10
    resource_user(sim, res, 10, usage_times);

    // queued, gives up after waiting until t=5
    auto proc = [&res, &usage_times,
                 &second_timed_out](auto &sim) -> simcpp20::process<> {
      auto request = res.request();
      auto timeout = sim.timeout(5);

      co_await (request | timeout);

      if (request.triggered()) {
        // got the resource
        usage_times.push_back(sim.now());
        co_await sim.timeout(5);
        res.release();
      } else {
        // timed out waiting
        second_timed_out = true;
        request.abort();
      }
    };

    proc(sim);

    // queued, gets resource at t=10, uses until t=15
    resource_user(sim, res, 5, usage_times);

    sim.run();

    REQUIRE(second_timed_out);
    REQUIRE(usage_times == std::vector<double>{0, 10});
  }
}
