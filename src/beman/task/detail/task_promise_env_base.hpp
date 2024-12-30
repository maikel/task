// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_TASK_PROMISE_ENV_BASE_HPP
#define BEMAN_TASK_DETAIL_TASK_PROMISE_ENV_BASE_HPP

#include <beman/task/detail/manual_lifetime.hpp>
#include <beman/task/detail/task_receiver_interface.hpp>

#include <beman/execution26/execution.hpp>

#include <exception>

namespace beman::task::detail {
template <class...> class types;

template <class Tp, class Context> class task_promise_env_base {
 public:
  class receiver_base : public task_receiver_interface<Tp> {
   public:
    receiver_base() = default;
    receiver_base(const receiver_base&) = default;
    receiver_base& operator=(const receiver_base&) = default;
    receiver_base(receiver_base&&) = default;
    receiver_base& operator=(receiver_base&&) = default;

    virtual auto get_env() const noexcept -> typename Context::env_type = 0;

   protected:
    ~receiver_base() = default;
  };

  task_promise_env_base() noexcept = default;

  void connect(receiver_base& receiver) noexcept { this->receiver_ = &receiver; }

  auto get_env() const noexcept {
    assert(this->receiver_);
    return this->receiver_->get_env();
  }

  template <class Self, class Value> auto await_transform(this Self& self, Value&& value) {
    if constexpr (requires(Self& slf, Value&& val) {
                    { Context::await_transform(::std::forward<Value>(val), slf.get_env()) };
                  }) {
      assert(self.receiver_);
      return ::beman::execution26::as_awaitable(
          Context::await_transform(::std::forward<Value>(value), self.get_env()), self);
    } else {
      return ::beman::execution26::as_awaitable(::std::forward<Value>(value), self);
    }
  }

  receiver_base* receiver_;
};

} // namespace beman::task::detail

#endif