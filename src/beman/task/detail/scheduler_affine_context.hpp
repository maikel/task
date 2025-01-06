// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_SCHEDULER_AFFINE_CONTEXT_HPP
#define BEMAN_TASK_DETAIL_SCHEDULER_AFFINE_CONTEXT_HPP

#include <beman/task/detail/any_env.hpp>
#include <beman/task/detail/any_scheduler.hpp>
#include <beman/task/detail/join_envs.hpp>
#include <beman/task/detail/with_query.hpp>

#include <beman/execution26/execution.hpp>

namespace beman::task {

template <class... Queries> struct scheduler_affine_context {
  using env_type = ::beman::task::detail::any_env<                                      //
      ::beman::task::detail::any_scheduler(::beman::execution26::get_scheduler_t),      //
      ::beman::execution26::inplace_stop_token(::beman::execution26::get_stop_token_t), //
      Queries...>;

  template <class Receiver> class operation_state_context;

  template <class Awaitable> static auto await_transform(Awaitable awaitable, const env_type& env) {
    return ::beman::execution26::continues_on(std::move(awaitable),
                                              ::beman::execution26::get_scheduler(env));
  }
};

template <class... Queries>
template <class Receiver>
class scheduler_affine_context<Queries...>::operation_state_context {
 public:
  using env_type = typename scheduler_affine_context<Queries...>::env_type;

  struct env_t {
    const operation_state_context* self;

    template <class Query>
      requires ::beman::task::detail::queryable_with<env_type, Query>
    auto query(Query query) const noexcept {
      return query(self->env_);
    }

    auto query(::beman::execution26::get_stop_token_t) const noexcept
        -> ::beman::execution26::inplace_stop_token {
      return self->stop_source_.get_token();
    }
  };

  explicit operation_state_context(const Receiver& receiver) noexcept
      : operation_state_context(receiver, ::beman::execution26::get_env(receiver)) {}

  explicit operation_state_context(const Receiver&,
                                   ::beman::execution26::env_of_t<Receiver> env) noexcept
      : env_{env}
      , stop_callback_(::beman::execution26::get_stop_token(env), callback_type{*this}) {}

  auto get_env() const noexcept -> env_type { return env_type(env_t{this}); }

 private:
  struct callback_type {
    operation_state_context& self;
    void operator()() const noexcept { this->self.stop_source_.request_stop(); }
  };

  using stop_token_type =
      ::beman::execution26::stop_token_of_t<::beman::execution26::env_of_t<Receiver>>;
  using stop_callback_type =
      ::beman::execution26::stop_callback_for_t<stop_token_type, callback_type>;

  ::beman::execution26::inplace_stop_source stop_source_{};
  ::beman::execution26::env_of_t<Receiver> env_;
  stop_callback_type stop_callback_;
};

} // namespace beman::task

#endif // BEMAN_TASK_DETAIL_SCHEDULER_AFFINE_CONTEXT_HPP