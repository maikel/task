// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_TASK_PROMISE_ENV_BASE_HPP
#define BEMAN_TASK_DETAIL_TASK_PROMISE_ENV_BASE_HPP

#include <beman/task/detail/any_env.hpp>
#include <beman/task/detail/any_receiver_ref.hpp>
#include <beman/task/detail/any_scheduler.hpp>
#include <beman/task/detail/join_envs.hpp>
#include <beman/task/detail/with_query.hpp>

#include <beman/execution26/execution.hpp>

namespace beman::task::detail {

template <class Derived, class... Queries> class task_promise_env_base {
 public:
  task_promise_env_base() noexcept = default;

  template <class Env>
  void set_env(Env env, ::beman::execution26::inplace_stop_token stop_token) noexcept {
    env_ = ::std::move(env);
    stop_token_ = ::std::move(stop_token);
  }

  auto get_env() const noexcept {
    return ::beman::task::detail::join_envs(
        env_, ::beman::task::detail::with_query(::beman::execution26::get_stop_token, stop_token_));
  }

  template <class Value>
  auto await_transform(Value&& value) -> ::beman::execution26::detail::call_result_t<
                                          ::beman::execution26::as_awaitable_t, Value, Promise&> {
    return ::beman::execution26::as_awaitable(
        ::beman::execution26::continues_on(::std::forward<Value>(value), this->scheduler_),
        static_cast<Promise&>(*this));
  }

 private:
  ::beman::task::detail::any_env<Queries...> env_;
  ::beman::task::detail::any_scheduler scheduler_;
  ::beman::execution26::inplace_stop_token stop_token_;
};

} // namespace beman::task::detail

#endif