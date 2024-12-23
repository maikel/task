// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_JOIN_ENVS_HPP
#define BEMAN_TASK_DETAIL_JOIN_ENVS_HPP

#include <beman/task/detail/has_query_for.hpp>

namespace beman::task::detail {

template <class Env1, class Env2> class joined_envs {
 public:
  explicit joined_envs(const Env1& env1, const Env2& env2) noexcept
      : env1_(env1)
      , env2_(env2) {}

  template <class... Args>
    requires queryable_with<Env1, const Args&...> || queryable_with<Env2, const Args&...>
  auto query(const Args&... args) const noexcept {
    if constexpr (queryable_with<Env2, const Args&...>) {
      return env2_.query(args...);
    } else {
      return env1_.query(args...);
    }
  }

 private:
  [[no_unique_address]] Env1 env1_;
  [[no_unique_address]] Env2 env2_;
};

template <class Env1, class Env2>
auto join_envs(const Env1& env1, const Env2& env2) noexcept -> joined_envs<Env1, Env2> {
  return joined_envs<Env1, Env2>(env1, env2);
}

template <class Env1, class Env2, class... Envs>
  requires(sizeof...(Envs) > 0)
auto join_envs(const Env1& env1, const Env2& env2, const Envs&... envs) noexcept {
  return join_envs(joined_envs<Env1, Env2>(env1, env2), envs...);
}

} // namespace beman::task::detail

#endif