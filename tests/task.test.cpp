#include <beman/task/task.hpp>

#include <beman/execution26/execution.hpp>

#include <iostream>

#define ASSERT(...)                                                                                \
  if (!(__VA_ARGS__)) {                                                                            \
    std::cerr << "Assertion failed: " #__VA_ARGS__ << std::endl;                                   \
    ::std::terminate();                                                                            \
  }

beman::task::task<void> void_task() { co_return; }
auto test_void_task() noexcept {
  auto task = void_task();
  ASSERT(::beman::execution26::sync_wait(std::move(task)));
}

beman::task::task<int> int_task() { co_return 42; }
auto test_int_task() noexcept {
  auto task = int_task();
  auto result = ::beman::execution26::sync_wait(std::move(task));
  ASSERT(result);
  auto [value] = result.value();
  ASSERT(value == 42);
}

beman::task::task<void> await_just_int() {
  auto value = co_await beman::execution26::just(42);
  ASSERT(value == 42);
  co_return;
}
auto test_await_just_int() noexcept {
  auto task = await_just_int();
  auto result = ::beman::execution26::sync_wait(std::move(task));
  ASSERT(result);
}

int main() {
  test_void_task();
  test_int_task();
  test_await_just_int();
}