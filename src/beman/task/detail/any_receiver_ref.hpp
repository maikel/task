// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_ANY_RECEIVER_REF_HPP
#define BEMAN_TASK_DETAIL_ANY_RECEIVER_REF_HPP

#include <beman/execution26/execution.hpp>

#include <beman/task/detail/receiver_implementation.hpp>
#include <beman/task/detail/receiver_interface.hpp>

#include <concepts>

namespace beman::task::detail {
template <class CompletionSigs, class Env> class any_receiver_ref {
 public:
  using receiver_concept = ::beman::execution26::receiver_t;

  template <class Receiver>
  inline static constexpr auto get_interface_fn_ =
      +[](std::any& storage) -> receiver_interface<CompletionSigs, Env>& {
    receiver_implementation<Receiver, CompletionSigs, Env>* ptr =
        ::std::any_cast<receiver_implementation<Receiver, CompletionSigs, Env>>(&storage);
    assert(ptr);
    return *ptr;
  };

  any_receiver_ref() noexcept = default;

  template <class Receiver>
    requires(!::std::same_as<any_receiver_ref, Receiver>)
  any_receiver_ref(Receiver& receiver) noexcept
      : storage_{receiver_implementation<Receiver, CompletionSigs, Env>{&receiver}}
      , get_interface_{get_interface_fn_<Receiver>} {}

  any_receiver_ref(const any_receiver_ref& other) noexcept
      : storage_{other.storage_} {}

  any_receiver_ref(any_receiver_ref&& other) noexcept
      : storage_{other.storage_} {}

  any_receiver_ref& operator=(const any_receiver_ref& other) noexcept {
    this->storage_ = &other.storage_;
    return *this;
  }

  any_receiver_ref& operator=(any_receiver_ref&& other) noexcept {
    this->storage_ = std::move(other.storage_);
    return *this;
  }

  template <class Receiver>
    requires(!::std::same_as<any_receiver_ref, Receiver>)
  any_receiver_ref& operator=(Receiver& receiver) noexcept {
    this->storage_ = receiver_implementation<Receiver, CompletionSigs, Env>{&receiver};
    this->get_interface_ = get_interface_fn_<Receiver>;
    return *this;
  }

  template <class... Args> void set_value(Args&&... args) && noexcept {
    this->get_interface_(this->storage_)
        .set_complete(::beman::execution26::set_value, ::std::forward<Args>(args)...);
  }

  template <class Error> void set_error(Error&& error) && noexcept {
    this->get_interface_(this->storage_)
        .set_complete(::beman::execution26::set_error, ::std::forward<Error>(error));
  }

  void set_stopped() && noexcept {
    this->get_interface_(this->storage_).set_complete(::beman::execution26::set_stopped);
  }

  auto get_env() const noexcept -> Env {
    return this->get_interface_(const_cast<std::any&>(this->storage_)).get_env();
  }

 private:
  std::any storage_{};
  auto (*get_interface_)(std::any&)
      -> ::beman::task::detail::receiver_interface<CompletionSigs, Env>& {};
};

} // namespace beman::task::detail

#endif