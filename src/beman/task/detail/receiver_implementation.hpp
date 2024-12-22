// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_RECEIVER_IMPLEMENTAITON_HPP
#define BEMAN_TASK_DETAIL_RECEIVER_IMPLEMENTAITON_HPP

#include <beman/execution26/execution.hpp>

#include <beman/task/detail/receiver_interface.hpp>

namespace beman::task::detail {

template <class Derived, class Receiver, class CompletionSigs, class Env, class... Sig>
class receiver_completion_function_impl;

template <class Derived, class Receiver, class CompletionSigs, class Env>
class receiver_completion_function_impl<Derived, Receiver, CompletionSigs, Env>
    : public beman::task::detail::receiver_interface<CompletionSigs, Env> {
 protected:
  ~receiver_completion_function_impl() = default;
};

template <class Derived, class Receiver, class CompletionSigs, class Env, class Tag, class... Args,
          class... Sigs>
class receiver_completion_function_impl<Derived, Receiver, CompletionSigs, Env, Tag(Args...),
                                        Sigs...>
    : public receiver_completion_function_impl<Derived, Receiver, CompletionSigs, Env, Sigs...> {
 public:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
  void set_complete(Tag tag, Args... args) noexcept override {
    Receiver* receiver = static_cast<Derived*>(this)->receiver_;
    tag(std::move(*receiver), std::forward<Args>(args)...);
  }
#pragma clang diagnostic pop

 protected:
  ~receiver_completion_function_impl() = default;
};

template <class Receiver, class CompletionSigs, class Env> class receiver_implementation;

template <class Receiver, class Env, class... Sigs>
class receiver_implementation<Receiver, ::beman::execution26::completion_signatures<Sigs...>, Env>
    final
    : public ::beman::task::detail::receiver_completion_function_impl<
          receiver_implementation<Receiver, ::beman::execution26::completion_signatures<Sigs...>,
                                  Env>,
          Receiver, ::beman::execution26::completion_signatures<Sigs...>, Env, Sigs...> {
 public:
  receiver_implementation(Receiver* receiver) noexcept
      : receiver_{receiver} {}

  using ::beman::task::detail::receiver_completion_function_impl<
      receiver_implementation<Receiver, ::beman::execution26::completion_signatures<Sigs...>, Env>,
      Receiver, ::beman::execution26::completion_signatures<Sigs...>, Env, Sigs...>::set_complete;

  auto get_env() const noexcept -> Env { return Env(::beman::execution26::get_env(receiver_)); }

  Receiver* receiver_;
};

} // namespace beman::task::detail

#endif