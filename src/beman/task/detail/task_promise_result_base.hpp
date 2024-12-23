// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_TASK_PROMISE_RESULT_BASE_HPP
#define BEMAN_TASK_DETAIL_TASK_PROMISE_RESULT_BASE_HPP

#include <beman/task/detail/as_exception_ptr.hpp>
#include <beman/task/detail/task_promise_env_base.hpp>

#include <exception>
#include <utility>
#include <variant>

namespace beman::task::detail {
template <class Ret, class CompletionSigs, class... Queries>
class task_promise_result_base : public task_promise_env_base<CompletionSigs, Queries...> {
 public:
  task_promise_result_base() noexcept = default;

  template <class Arg> void return_value(Arg&& arg) noexcept {
    this->set_result(::std::forward<Arg>(arg));
  }

  void unhandled_exception() noexcept { this->set_exception(::std::current_exception()); }

  void set_complete() && noexcept {
    try {
      ::beman::execution26::set_value(::std::move(this->receiver_), get_result());
    } catch (...) {
      ::beman::execution26::set_error(::std::move(this->receiver_), ::std::current_exception());
    }
  }

  template <class... Args> void set_result(Args&&... args) noexcept {
    try {
      this->result_.template emplace<1>(::std::forward<Args>(args)...);
    } catch (...) {
      this->set_exception(::std::current_exception());
    }
  }

  template <class Error> void set_exception(Error&& error) noexcept {
    this->result_.template emplace<2>(
        ::beman::task::detail::as_exception_ptr(::std::forward<Error>(error)));
  }

 private:
  Ret&& get_result() {
    if (this->result_.index() == 2) {
      ::std::rethrow_exception(::std::get<2>(::std::move(this->result_)));
    }
    assert(this->result_.index() == 1);
    return ::std::get<1>(::std::move(this->result_));
  }

  struct empty_result_t {};
  ::std::variant<empty_result_t, Ret, ::std::exception_ptr> result_{};
};

template <class CompletionSigs, class... Queries>
class task_promise_result_base<void, CompletionSigs, Queries...>
    : public task_promise_env_base<CompletionSigs, Queries...> {
 public:
  task_promise_result_base() noexcept = default;

  void return_void() noexcept { this->set_result(); }

  void unhandled_exception() noexcept { this->set_exception(::std::current_exception()); }

  void set_complete() && noexcept {
    try {
      this->get_result();
      ::beman::execution26::set_value(::std::move(this->receiver_));
    } catch (...) {
      ::beman::execution26::set_error(::std::move(this->receiver_), ::std::current_exception());
    }
  }

  void set_result() noexcept { this->result_.template emplace<1>(); }

  template <class Error> void set_exception(Error&& error) noexcept {
    this->result_.template emplace<2>(
        ::beman::task::detail::as_exception_ptr(::std::forward<Error>(error)));
  }

 private:
  void get_result() {
    if (this->result_.index() == 2) {
      ::std::rethrow_exception(::std::get<2>(::std::move(this->result_)));
    }
    assert(this->result_.index() == 1);
  }
  struct void_t {};
  ::std::variant<::std::monostate, void_t, ::std::exception_ptr> result_{};
};
} // namespace beman::task::detail

#endif