// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_TASK_PROMISE_ENV_BASE_HPP
#define BEMAN_TASK_DETAIL_TASK_PROMISE_ENV_BASE_HPP

#include <beman/task/detail/any_receiver_ref.hpp>
#include <beman/task/detail/any_scheduler.hpp>
#include <beman/task/detail/join_envs.hpp>
#include <beman/task/detail/with_query.hpp>

#include <beman/execution26/execution.hpp>

namespace beman::task::detail {

template <class CompletionSigs, class... Queries> class task_promise_env_base {
 public:
  task_promise_env_base() noexcept = default;

  template <class Receiver>
  void set_receiver(Receiver& receiver,
                    ::beman::execution26::inplace_stop_token stop_token) noexcept {
    this->receiver_ = receiver;
    this->stop_token_ = ::std::move(stop_token);
  }

  auto get_env() const noexcept {
    return ::beman::task::detail::join_envs(
        ::beman::execution26::get_env(this->receiver_),
        ::beman::task::detail::with_query(::beman::execution26::get_stop_token, this->stop_token_));
  }

  template <class Self, class Value>
  auto await_transform(this Self& self, Value&& value)
      -> ::beman::execution26::detail::call_result_t<::beman::execution26::as_awaitable_t, Value,
                                                     Self&> {
    return ::beman::execution26::as_awaitable(
        ::beman::execution26::continues_on(::std::forward<Value>(value), self.scheduler_), self);
  }

  ::beman::task::detail::any_receiver_ref<CompletionSigs, any_env<Queries...>> receiver_;
  ::beman::execution26::inplace_stop_token stop_token_;
};

} // namespace beman::task::detail

#endif