#ifndef BEMAN_TASK_DETAIL_ANY_RECEIVER_REF_HPP
#define BEMAN_TASK_DETAIL_ANY_RECEIVER_REF_HPP

#include <beman/execution26/execution.hpp>

#include <concepts>

namespace beman::task::detail {

template <class CompletionSigs, class Env> class any_receiver_ref {
 public:
  using receiver_concept = ::beman::execution26::receiver_t;

  any_receiver_ref() noexcept = default;

  template <class Receiver>
    requires(!::std::same_as<any_receiver_ref, Receiver> &&
             ::beman::execution26::receiver_of<Receiver, CompletionSigs> &&
             ::std::same_as<Env, ::beman::execution26::env_of_t<Receiver>>)
  any_receiver_ref(Receiver& receiver) noexcept
      : ref_{&receiver} {}

  any_receiver_ref(const any_receiver_ref& other) noexcept
      : ref_{other.ref_} {}

  any_receiver_ref(any_receiver_ref&& other) noexcept
      : ref_{other.ref_} {}

  any_receiver_ref& operator=(const any_receiver_ref& other) noexcept {
    this->ref_ = other.ref_;
    return *this;
  }

  any_receiver_ref& operator=(any_receiver_ref&& other) noexcept {
    this->ref_ = other.ref_;
    return *this;
  }

  template <class... Args> void set_value(Args&&... args) && noexcept {
    this->get_interface_(this->ref_).set_value(::std::forward<Args>(args)...);
  }

  template <class Error> void set_error(Error&& error) && noexcept {
    this->get_interface_(this->ref_).set_error(::std::forward<Error>(error));
  }

  void set_stopped() && noexcept { this->get_interface_(this->ref_).set_stopped(); }

  auto get_env() const noexcept -> const Env& { return this->get_interface_(this->ref_).get_env(); }

 private:
  void* ref_{};
  auto (*get_interface_)(void*) noexcept
      -> ::beman::task::detail::receiver_interface<CompletionSigs, Env>& {};
};

} // namespace beman::task::detail

#endif