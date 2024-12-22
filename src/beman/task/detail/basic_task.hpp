// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_BASIC_TASK_HPP
#define BEMAN_TASK_BASIC_TASK_HPP

#include <beman/execution26/execution.hpp>

#include <beman/task/detail/any_env.hpp>
#include <beman/task/detail/as_exception_ptr.hpp>
#include <beman/task/detail/env_interface.hpp>
#include <beman/task/detail/join_envs.hpp>
#include <beman/task/detail/manual_lifetime.hpp>
#include <beman/task/detail/query_base.hpp>
#include <beman/task/detail/set_value_for.hpp>
#include <beman/task/detail/task_promise_result_base.hpp>
#include <beman/task/detail/with_query.hpp>

#include <any>
#include <exception>
#include <utility>
#include <variant>

namespace beman::task {
template <class Ret, class... Queries> class basic_task {
 public:
  class promise_type;

 private:
  template <class Receiver> class operation {
   public:
    using operation_state_concept = ::beman::execution26::operation_state_t;

    explicit operation(::std::coroutine_handle<promise_type> handle, Receiver receiver) noexcept
        : receiver_{receiver}
        , handle_{handle} {
      handle_.promise().set_receiver(this->receiver_, stop_source_.get_token());
    }

    ~operation() {
      if (handle_) {
        handle_.destroy();
      }
    }

    void start() noexcept {
      auto stop_token =
          ::beman::execution26::get_stop_token(::beman::execution26::get_env(this->receiver_));
      if (stop_token.stop_requested()) {
        ::beman::execution26::set_stopped(std::move(this->receiver_));
      }
      this->stop_callback_.emplace(stop_token, callback_type{*this});
      handle_.resume();
    }

   private:
    struct callback_type {
      operation& op;
      void operator()() const noexcept { this->op.stop_source_.request_stop(); }
    };
    using stop_token_type =
        ::beman::execution26::stop_token_of_t<::beman::execution26::env_of_t<Receiver>>;
    using stop_callback_type =
        ::beman::execution26::stop_callback_for_t<stop_token_type, callback_type>;
    Receiver receiver_;
    ::std::coroutine_handle<promise_type> handle_{};
    ::beman::execution26::inplace_stop_source stop_source_{};
    [[no_unique_address]]
    ::beman::task::detail::manual_lifetime<stop_callback_type> stop_callback_;
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

  template <class Receiver> auto connect(Receiver receiver) && noexcept -> operation<Receiver> {
    return operation<Receiver>{std::exchange(handle_, {}), ::std::move(receiver)};
  }

 private:
  basic_task(::std::coroutine_handle<promise_type> handle) noexcept
      : handle_{handle} {}

  ::std::coroutine_handle<promise_type> handle_{};
};

template <class Ret, class... Queries>
class basic_task<Ret, Queries...>::promise_type
    : public ::beman::task::detail::task_promise_result_base<Ret, basic_task::completion_signatures,
                                                             Queries...> {
 public:
  auto get_return_object() noexcept -> basic_task {
    return basic_task{::std::coroutine_handle<promise_type>::from_promise(*this)};
  }

  auto initial_suspend() noexcept -> ::std::suspend_always { return {}; }

  struct final_awaiter {
    static constexpr bool await_ready() noexcept { return false; }

    void await_suspend(::std::coroutine_handle<promise_type> h) noexcept {
      promise_type& promise = h.promise();
      std::move(promise).set_complete();
    }

    void await_resume() noexcept {}
  };

  auto final_suspend() noexcept -> final_awaiter { return {}; }
};

} // namespace beman::task

#endif