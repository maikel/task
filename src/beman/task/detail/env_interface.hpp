// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_ENV_INTERFACE_HPP
#define BEMAN_TASK_DETAIL_ENV_INTERFACE_HPP

#include <beman/task/detail/query_base.hpp>

namespace beman::task::detail {
template <class... Queries> class env_interface : query_base<Queries>... {
 public:
  env_interface() noexcept = default;
  env_interface(const env_interface&) = default;
  env_interface& operator=(const env_interface&) = default;
  env_interface(env_interface&&) = default;
  env_interface& operator=(env_interface&&) = default;

  using query_base<Queries>::query...;

 protected:
  ~env_interface() = default;
};
} // namespace beman::task::detail

#endif