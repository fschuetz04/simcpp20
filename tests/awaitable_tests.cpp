#include "catch2/catch_test_macros.hpp"
#include "fschuetz04/simcpp20.hpp"

simcpp20::process<> producer_process(simcpp20::simulation<> &sim, int delay) {
  co_await sim.timeout(delay);
}

template <typename A1, typename A2>
simcpp20::process<> consumer_process(simcpp20::simulation<> &sim, 
                                     A1 &a1, 
                                     A2 &a2,
                                     double &first_time,
                                     double &all_time) {
  // Wait for any producer to finish
  co_await (a1 | a2);
  first_time = sim.now();
  
  // Wait for all producers to finish
  co_await (a1 & a2);
  all_time = sim.now();
}

TEST_CASE("processes can be used with any_of and all_of") {
  simcpp20::simulation<> sim;

  // Test with process objects
  SECTION("process | process") {
    auto p1 = producer_process(sim, 5);
    auto p2 = producer_process(sim, 10);
    
    double first_time = -1;
    double all_time = -1;
    consumer_process(sim, p1, p2, first_time, all_time);
    
    sim.run();
    
    REQUIRE(first_time == 5);
    REQUIRE(all_time == 10);
  }
  
  // Test with mixing process and event
  SECTION("process | event") {
    auto p1 = producer_process(sim, 5);
    auto e2 = sim.timeout(10);
    
    double first_time = -1;
    double all_time = -1;
    consumer_process(sim, p1, e2, first_time, all_time);
    
    sim.run();
    
    REQUIRE(first_time == 5);
    REQUIRE(all_time == 10);
  }
  
  // Test with event objects
  SECTION("event | event using processes") {
    auto e1 = sim.timeout(5);
    auto e2 = sim.timeout(10);
    
    double first_time = -1;
    double all_time = -1;
    consumer_process(sim, e1, e2, first_time, all_time);
    
    sim.run();
    
    REQUIRE(first_time == 5);
    REQUIRE(all_time == 10);
  }
}

simcpp20::process<> callback_test_process(simcpp20::simulation<> &sim, int delay, bool &called) {
  co_await sim.timeout(delay);
  called = true;
}

TEST_CASE("awaitable callbacks work with processes") {
  simcpp20::simulation<> sim;

  SECTION("process callbacks are called on completion") {
    bool callback_called = false;
    auto p = callback_test_process(sim, 5, callback_called);
    
    bool manual_callback_called = false;
    p.add_callback([&manual_callback_called]() {
      manual_callback_called = true;
    });
    
    sim.run();
    
    REQUIRE(callback_called);
    REQUIRE(manual_callback_called);
  }
}

// Test composite operations with multiple process objects
TEST_CASE("complex process compositions with any_of and all_of") {
  simcpp20::simulation<> sim;
  
  SECTION("complex composition") {
    auto p1 = producer_process(sim, 5);
    auto p2 = producer_process(sim, 10);
    auto p3 = producer_process(sim, 15);
    
    // Any of p1 or p2, then all of those and p3
    auto any12 = p1 | p2;
    auto all_events = any12 & p3;
    
    double completion_time = -1;
    
    // Define a lambda that returns a process
    auto wait_fn = [&completion_time](simcpp20::simulation<>& s, simcpp20::event<>& ev) -> simcpp20::process<> {
      co_await ev;
      completion_time = s.now();
    };
    
    // Call the lambda with sim and the composed event
    wait_fn(sim, all_events);
    
    sim.run();
    
    // The first of p1 or p2 completes at t=5, then we wait for both p2 and p3
    // p2 completes at t=10, p3 completes at t=15, so the final time should be 15
    REQUIRE(completion_time == 15);
  }
}