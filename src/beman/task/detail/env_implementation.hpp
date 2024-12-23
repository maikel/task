// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_ENV_IMPLEMENTATION_HPP
#define BEMAN_TASK_DETAIL_ENV_IMPLEMENTATION_HPP

#include <utility>

namespace beman::task::detail {

template <class Derived, class Interface, class... Queries> class env_implmentation;

template <class Derived, class Interface>
class env_implmentation<Derived, Interface> : public Interface {
 public:
  env_implmentation() noexcept = default;
  env_implmentation(const env_implmentation&) = default;
  env_implmentation& operator=(const env_implmentation&) = default;
  env_implmentation(env_implmentation&&) = default;
  env_implmentation& operator=(env_implmentation&&) = default;

 protected:
  ~env_implmentation() = default;
};

template <class Derived, class Interface, class Ret, class... Args, class... Queries>
class env_implmentation<Derived, Interface, Ret(Args...), Queries...>
    : public env_implmentation<Derived, Interface, Queries...> {
 public:
  env_implmentation() noexcept = default;
  env_implmentation(const env_implmentation&) = default;
  env_implmentation& operator=(const env_implmentation&) = default;
  env_implmentation(env_implmentation&&) = default;
  env_implmentation& operator=(env_implmentation&&) = default;

  Ret query(Args... args) const noexcept override {
    return static_cast<const Derived*>(this)->do_query(::std::forward<Args>(args)...);
  }

 protected:
  ~env_implmentation() = default;
};

} // namespace beman::task::detail

#endif