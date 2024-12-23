// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_RECEIVER_INTERFACE_HPP
#define BEMAN_TASK_DETAIL_RECEIVER_INTERFACE_HPP

#include <beman/execution26/execution.hpp>

namespace beman::task::detail {

template <class... Sigs> class receiver_completion_function;

template <> class receiver_completion_function<> {
 public:
  void set_complete() noexcept {}
};

template <class Tag, class... Args, class... Sigs>
class receiver_completion_function<Tag(Args...), Sigs...>
    : public receiver_completion_function<Sigs...> {
 public:
  receiver_completion_function() noexcept = default;
  receiver_completion_function(const receiver_completion_function&) = default;
  receiver_completion_function& operator=(const receiver_completion_function&) = default;
  receiver_completion_function(receiver_completion_function&&) = default;
  receiver_completion_function& operator=(receiver_completion_function&&) = default;

  using receiver_completion_function<Sigs...>::set_complete;
  virtual void set_complete(Tag, Args...) noexcept = 0;

 protected:
  ~receiver_completion_function() = default;
};

template <class CompletionSigs, class Env> class receiver_interface;

template <class Env, class... Sigs>
class receiver_interface<::beman::execution26::completion_signatures<Sigs...>, Env>
    : public ::beman::task::detail::receiver_completion_function<Sigs...> {
 public:
  receiver_interface() noexcept = default;
  receiver_interface(const receiver_interface&) = default;
  receiver_interface& operator=(const receiver_interface&) = default;
  receiver_interface(receiver_interface&&) = default;
  receiver_interface& operator=(receiver_interface&&) = default;

  using ::beman::task::detail::receiver_completion_function<Sigs...>::set_complete;
  virtual auto get_env() const noexcept -> Env = 0;

 protected:
  ~receiver_interface() = default;
};

} // namespace beman::task::detail

#endif