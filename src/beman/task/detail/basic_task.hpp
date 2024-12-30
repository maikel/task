// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_BASIC_TASK_HPP
#define BEMAN_TASK_BASIC_TASK_HPP

#include <beman/task/detail/as_exception_ptr.hpp>
#include <beman/task/detail/join_envs.hpp>
#include <beman/task/detail/manual_lifetime.hpp>
#include <beman/task/detail/set_value_for.hpp>
#include <beman/task/detail/task_promise_result_base.hpp>
#include <beman/task/detail/with_query.hpp>

#include <beman/execution26/execution.hpp>

#include <any>
#include <exception>
#include <utility>
#include <variant>

namespace beman::task {
template <class Ret, class Context> class basic_task {
 public:
  class promise_type;

 private:
  template <class Receiver>
  class operation final
      : public ::beman::task::detail::task_promise_env_base<Ret, Context>::receiver_base {
   public:
    using operation_state_concept = ::beman::execution26::operation_state_t;

    explicit operation(::std::coroutine_handle<promise_type> handle, Receiver receiver) noexcept
        : receiver_{std::move(receiver)}
        , handle_{handle} {
      context_.emplace(receiver_);
      handle_.promise().connect(*this);
    }

    operation(const operation&) = delete;
    operation& operator=(const operation&) = delete;
    operation(operation&& other) = delete;
    operation& operator=(operation&& other) = delete;

    ~operation() {
      if (handle_) {
        handle_.destroy();
      }
    }

    void start() noexcept { handle_.resume(); }

   private:
    void set_value(::beman::task::detail::value_or_void_t<Ret> value) noexcept override {
      context_.reset();
      if constexpr (::std::is_void_v<Ret>) {
        ::beman::execution26::set_value(std::move(this->receiver_));
      } else {
        ::beman::execution26::set_value(std::move(this->receiver_), std::move(value));
      }
    }

    void set_error(const std::exception_ptr& err) noexcept override {
      context_.reset();
      ::beman::execution26::set_error(std::move(this->receiver_), err);
    }

    void set_stopped() noexcept override {
      context_.reset();
      ::beman::execution26::set_stopped(std::move(this->receiver_));
    }

    auto get_env() const noexcept -> typename Context::env_type override {
      return context_->get_env();
    }

    Receiver receiver_;
    ::beman::task::detail::manual_lifetime<typename Context::template type<Receiver>> context_;
    ::std::coroutine_handle<promise_type> handle_{};
  };

 public:
  using sender_concept = ::beman::execution26::sender_t;

  using completion_signatures = ::beman::execution26::completion_signatures<
      ::beman::task::detail::set_value_for<Ret>,
      ::beman::execution26::set_error_t(std::exception_ptr), ::beman::execution26::set_stopped_t()>;

  ~basic_task() {
    if (handle_) {
      handle_.destroy();
    }
  }

  basic_task(const basic_task&) = delete;

  basic_task& operator=(const basic_task&) = delete;

  basic_task(basic_task&& other) noexcept
      : handle_{::std::exchange(other.handle_, {})} {}

  basic_task& operator=(basic_task&& other) noexcept {
    if (this != &other) {
      if (handle_) {
        handle_.destroy();
      }
      handle_ = ::std::exchange(other.handle_, {});
    }
    return *this;
  }

  template <class Receiver>
    requires ::beman::execution26::receiver_of<Receiver, completion_signatures>
  auto connect(Receiver receiver) && noexcept -> operation<Receiver> {
    return operation<Receiver>{std::exchange(handle_, {}), ::std::move(receiver)};
  }

 private:
  explicit basic_task(::std::coroutine_handle<promise_type> handle) noexcept
      : handle_{handle} {}

  ::std::coroutine_handle<promise_type> handle_{};
};

template <class Ret, class Context>
class basic_task<Ret, Context>::promise_type
    : public ::beman::task::detail::task_promise_result_base<Ret, Context> {
 public:
  promise_type() noexcept = default;

  auto unhandled_stopped() noexcept -> ::std::coroutine_handle<> {
    this->receiver_->set_stopped();
    return ::std::noop_coroutine();
  }

  auto get_return_object() noexcept -> basic_task {
    return basic_task{::std::coroutine_handle<promise_type>::from_promise(*this)};
  }

  auto initial_suspend() noexcept -> ::std::suspend_always { return {}; }

  struct final_awaiter {
    static constexpr bool await_ready() noexcept { return false; }

    void await_suspend(::std::coroutine_handle<promise_type> h) noexcept {
      promise_type& promise = h.promise();
      std::move(promise).do_complete();
    }

    void await_resume() noexcept {}
  };

  auto final_suspend() noexcept -> final_awaiter { return {}; }
};

} // namespace beman::task

#endif