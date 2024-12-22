// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_RECEIVER_INTERFACE_HPP
#define BEMAN_TASK_DETAIL_RECEIVER_INTERFACE_HPP

#include <beman/execution26/execution.hpp>

namespace beman::task::detail {

template <class Sig> class receiver_completion_function;
template <class Tag, class... Args> class receiver_completion_function<Tag(Args...)> {
 public:
  virtual void set_complete(Tag, Args...) noexcept = 0;

 protected:
  ~receiver_completion_function() = default;
};

template <class CompletionSigs, class Env> class receiver_interface;

template <class Env, class... Sigs>
class receiver_interface<::beman::execution26::completion_signatures<Sigs...>, Env>
    : public ::beman::task::detail::receiver_completion_function<Sigs>... {
 public:
  using ::beman::task::detail::receiver_completion_function<Sigs>::set_complete...;

  virtual auto get_env() const noexcept -> Env = 0;

 protected:
  ~receiver_interface() = default;
};

} // namespace beman::task::detail

#endif