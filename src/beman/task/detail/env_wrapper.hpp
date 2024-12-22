// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_ENV_WRAPPER_HPP
#define BEMAN_TASK_DETAIL_ENV_WRAPPER_HPP

#include <beman/task/detail/env_implementation.hpp>
#include <beman/task/detail/env_interface.hpp>

#include <utility>

namespace beman::task::detail {

template <class Env, class... Queries>
class env_wrapper final
    : public env_implmentation<env_wrapper<Env, Queries...>, env_interface<Queries...>,
                               Queries...> {
 public:
  explicit env_wrapper(Env env) noexcept
      : env_{std::move(env)} {}

  env_wrapper(const env_wrapper& other) noexcept
      : env_{other.env_} {}
  env_wrapper(env_wrapper&& other) noexcept
      : env_{std::move(other.env_)} {}
  env_wrapper& operator=(const env_wrapper& other) noexcept {
    env_ = other.env_;
    return *this;
  }
  env_wrapper& operator=(env_wrapper&& other) noexcept {
    env_ = std::move(other.env_);
    return *this;
  }

  template <class Tag, class... Args> auto do_query(Tag tag, Args&&... args) const noexcept {
    return env_.query(tag, std::forward<Args>(args)...);
  }

  Env env_;
};

} // namespace beman::task::detail

#endif