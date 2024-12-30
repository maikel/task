// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_ANY_ENV_HPP
#define BEMAN_TASK_DETAIL_ANY_ENV_HPP

#include <beman/task/detail/has_query_for.hpp>

#include <any>
#include <concepts>
#include <utility>

namespace beman::task::detail {

template <class... Queries> class any_env {
 private:
  template <class QuerySig> class query_base;

  template <class Ret, class... Args> class query_base<Ret(Args...)> {
   public:
    query_base() = default;
    query_base(const query_base&) = delete;
    query_base& operator=(const query_base&) = delete;
    query_base(query_base&&) = delete;
    query_base& operator=(query_base&&) = delete;

    virtual Ret query(Args...) const noexcept = 0;

   protected:
    ~query_base() = default;
  };

  class env_interface : query_base<Queries>... {
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

 public:
  any_env() noexcept = default;

  any_env(const any_env& other) = default;
  any_env(any_env&& other) noexcept = default;
  any_env& operator=(const any_env& other) = default;
  any_env& operator=(any_env&& other) noexcept = default;
  ~any_env() = default;

  template <class Env>
    requires(!::std::same_as<any_env, Env> &&
             (::beman::task::detail::has_query_for<Env, Queries> && ...))
  explicit any_env(Env env) noexcept
      : storage_{env_wrapper<Env>{::std::move(env)}}
      , get_interface_{+[](const ::std::any& storage) noexcept -> const env_interface& {
        return *::std::any_cast<env_wrapper<Env>>(&storage);
      }} {}

  template <class Env>
    requires(!::std::same_as<any_env, Env> &&
             (::beman::task::detail::has_query_for<Env, Queries> && ...))
  any_env& operator=(Env env) noexcept {
    storage_ = env_wrapper<Env>{::std::move(env)};
    get_interface_ = +[](const ::std::any& storage) noexcept -> const env_interface& {
      return *::std::any_cast<env_wrapper<Env>>(&storage);
    };
    return *this;
  }

  template <class... Args>
    requires ::beman::task::detail::queryable_with<env_interface, Args...>
  auto query(Args&&... args) const noexcept {
    return get_interface_(storage_).query(::std::forward<Args>(args)...);
  }

 private:
  template <class Derived, class... Qs> class env_implmentation;

  template <class Derived> class env_implmentation<Derived> : public env_interface {
   public:
    env_implmentation() noexcept = default;
    env_implmentation(const env_implmentation&) = default;
    env_implmentation& operator=(const env_implmentation&) = default;
    env_implmentation(env_implmentation&&) = default;
    env_implmentation& operator=(env_implmentation&&) = default;

   protected:
    ~env_implmentation() = default;
  };

  template <class Derived, class Ret, class... Args, class... Qs>
  class env_implmentation<Derived, Ret(Args...), Qs...> : public env_implmentation<Derived, Qs...> {
   public:
    env_implmentation() noexcept {}
    env_implmentation(const env_implmentation&) noexcept {}
    env_implmentation& operator=(const env_implmentation&) noexcept {}
    env_implmentation(env_implmentation&&) noexcept {}
    env_implmentation& operator=(env_implmentation&&) noexcept {}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
    Ret query(Args... args) const noexcept final {
      return Ret(static_cast<const Derived*>(this)->do_query(::std::forward<Args>(args)...));
    }
#pragma clang diagnostic pop

   protected:
    ~env_implmentation() = default;
  };

  template <class Env>
  class env_wrapper final : public env_implmentation<env_wrapper<Env>, Queries...> {
   public:
    explicit env_wrapper(Env env) noexcept
        : env_{std::move(env)} {}

    env_wrapper(const env_wrapper& other) noexcept = default;

    env_wrapper(env_wrapper&& other) noexcept = default;

    env_wrapper& operator=(const env_wrapper& other) noexcept = default;
    env_wrapper& operator=(env_wrapper&& other) noexcept = default;

    ~env_wrapper() = default;

    template <class Tag, class... Args> auto do_query(Tag tag, Args&&... args) const noexcept {
      return env_.query(tag, std::forward<Args>(args)...);
    }

    Env env_;
  };

  ::std::any storage_{};
  auto (*get_interface_)(const ::std::any&) noexcept -> const env_interface& {};
};

} // namespace beman::task::detail

#endif