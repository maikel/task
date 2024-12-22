// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_ANY_ENV_HPP
#define BEMAN_TASK_DETAIL_ANY_ENV_HPP

#include <beman/task/detail/env_interface.hpp>
#include <beman/task/detail/env_wrapper.hpp>
#include <beman/task/detail/has_query_for.hpp>

#include <any>
#include <concepts>
#include <utility>

namespace beman::task::detail {

template <class... Queries> class any_env {
 public:
  any_env() noexcept = default;

  any_env(const any_env& other) = default;
  any_env(any_env&& other) noexcept = default;
  any_env& operator=(const any_env& other) = default;
  any_env& operator=(any_env&& other) noexcept = default;

  template <class Env>
    requires(!::std::same_as<any_env, Env> &&
             (::beman::task::detail::has_query_for<Env, Queries> && ...))
  explicit any_env(Env env) noexcept
      : storage_{beman::task::detail::env_wrapper<Env, Queries...>{::std::move(env)}}
      , get_interface_{+[](const ::std::any& storage) noexcept
                       -> const beman::task::detail::env_interface<Queries...>& {
        return *::std::any_cast<beman::task::detail::env_wrapper<Env, Queries...>>(&storage);
      }} {}

  template <class Env>
    requires(!::std::same_as<any_env, Env> &&
             (::beman::task::detail::has_query_for<Env, Queries> && ...))
  any_env& operator=(Env env) noexcept {
    storage_ = beman::task::detail::env_wrapper<Env, Queries...>{::std::move(env)};
    get_interface_ = +[](const ::std::any& storage) noexcept
        -> const beman::task::detail::env_interface<Queries...>& {
      return *::std::any_cast<beman::task::detail::env_wrapper<Env, Queries...>>(&storage);
    };
    return *this;
  }

  template <class... Args>
    requires ::beman::task::detail::queryable_with<beman::task::detail::env_interface<Queries...>,
                                                   Args...>
  auto query(Args&&... args) const noexcept {
    return get_interface_(storage_).query(::std::forward<Args>(args)...);
  }

 private:
  ::std::any storage_{};
  auto (*get_interface_)(const ::std::any&) noexcept
      -> const beman::task::detail::env_interface<Queries...>& {};
};

} // namespace beman::task::detail

#endif