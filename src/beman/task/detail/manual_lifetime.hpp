// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_MANUAL_LIFETIME_HPP
#define BEMAN_TASK_MANUAL_LIFETIME_HPP

#include <utility>

namespace beman::task::detail {
template <class Tp> class manual_lifetime {
 public:
  constexpr manual_lifetime() noexcept = default;

  template <class... Args> constexpr Tp& emplace(Args&&... args) {
    return *new (&storage_.value_) Tp(std::forward<Args>(args)...);
  }

  template <class Function, class... Args>
  constexpr Tp& emplace_from(Function&& f, Args&&... args) {
    return *new (&storage_.value_) Tp(f(std::forward<Args>(args)...));
  }

  constexpr void reset() noexcept { storage_.value_.~Tp(); }

  constexpr Tp& get() noexcept { return storage_.value_; }

  constexpr const Tp& get() const noexcept { return storage_.value_; }

 private:
  struct empty_byte {};
  union Storage {
    empty_byte empty_;
    Tp value_;
    constexpr Storage() noexcept {}
    constexpr ~Storage() noexcept {}
  } storage_;
};
} // namespace beman::task::detail

#endif