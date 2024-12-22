// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_HAS_QUERY_FOR_HPP
#define BEMAN_TASK_DETAIL_HAS_QUERY_FOR_HPP

#include <concepts>

namespace beman::task::detail {

template <class Env, class... Args>
concept queryable_with = requires(const Env& env, const Args&... args) {
  { env.query(args...) } noexcept;
};

template <class Env, class... Args> struct ERROR_QUERY_NOT_FOUND_FOR {};

template <class Env, class Ret, class... Args>
  requires queryable_with<Env, Args...>
int query_args_helper(const Env&, Ret (*)(Args...));

template <class Env, class Ret, class... Args>
auto query_args_helper(const Env&, Ret (*)(Args...)) -> ERROR_QUERY_NOT_FOUND_FOR<Env, Args...>;

template <class Env, class Sig>
concept has_query_for = requires(const Env& env, Sig* sig) {
  { ::beman::task::detail::query_args_helper(env, sig) } -> ::std::same_as<int>;
};

} // namespace beman::task::detail

#endif