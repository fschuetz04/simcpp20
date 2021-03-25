#include <coroutine>
#include <iostream>

struct awaitable {
  bool await_ready() {
    std::cout << "awaitable::await_ready" << std::endl;
    return false;
  }
  void await_suspend(std::coroutine_handle<> h) {
    std::cout << "awaitable::await_suspend" << std::endl;
    h.resume();
  }
  void await_resume() { std::cout << "awaitable::await_resume" << std::endl; }
};

awaitable test1() {
  std::cout << "test1" << std::endl;
  return awaitable{};
}

struct task {
  struct promise_type {
    task get_return_object() {
      std::cout << "task::promise_type::get_return_object" << std::endl;
      return {};
    }
    std::suspend_never initial_suspend() {
      std::cout << "task::promise_type::initial_suspend" << std::endl;
      return {};
    }
    std::suspend_never final_suspend() {
      std::cout << "task::promise_type::final_suspend" << std::endl;
      return {};
    }
    void unhandled_exception() {
      std::cout << "task::promise_type::unhandled_exception" << std::endl;
    }
  };
};

task test2() {
  std::cout << "test2" << std::endl;
  co_await test1();
}

int main() {
  std::cout << "main" << std::endl;
  test2();
  std::cout << "exit" << std::endl;
}
