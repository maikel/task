// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_ANY_RECEIVER_REF_HPP
#define BEMAN_TASK_DETAIL_ANY_RECEIVER_REF_HPP

#include <beman/execution26/execution.hpp>

#include <any>
#include <concepts>

namespace beman::task::detail {
template <auto...> class values;

template <class CompletionSigs, class Env> class any_receiver_ref;

template <class Env, class... Sigs>
class any_receiver_ref<::beman::execution26::completion_signatures<Sigs...>, Env> {
 public:
  template <class... ComplSigs> class receiver_completion_function;

  template <class Tag, class... Args> class receiver_completion_function<Tag(Args...)> {
   public:
    receiver_completion_function() noexcept = default;
    receiver_completion_function(const receiver_completion_function&) = default;
    receiver_completion_function& operator=(const receiver_completion_function&) = default;
    receiver_completion_function(receiver_completion_function&&) = default;
    receiver_completion_function& operator=(receiver_completion_function&&) = default;

    virtual void set_complete(Tag, Args...) noexcept = 0;

   protected:
    ~receiver_completion_function() = default;
  };

  template <class Tag, class... Args, class... ComplSigs>
  class receiver_completion_function<Tag(Args...), ComplSigs...>
      : public receiver_completion_function<ComplSigs...> {
   public:
    receiver_completion_function() noexcept = default;
    receiver_completion_function(const receiver_completion_function&) = default;
    receiver_completion_function& operator=(const receiver_completion_function&) = default;
    receiver_completion_function(receiver_completion_function&&) = default;
    receiver_completion_function& operator=(receiver_completion_function&&) = default;

    using receiver_completion_function<ComplSigs...>::set_complete;
    virtual void set_complete(Tag, Args...) noexcept = 0;

   protected:
    ~receiver_completion_function() = default;
  };

  class receiver_interface : public receiver_completion_function<Sigs...> {
   public:
    receiver_interface() noexcept = default;
    receiver_interface(const receiver_interface&) = default;
    receiver_interface& operator=(const receiver_interface&) = default;
    receiver_interface(receiver_interface&&) = default;
    receiver_interface& operator=(receiver_interface&&) = default;

    using receiver_completion_function<Sigs...>::set_complete;
    virtual auto get_env() const noexcept -> Env = 0;

   protected:
    ~receiver_interface() = default;
  };

 public:
  using receiver_concept = ::beman::execution26::receiver_t;

  any_receiver_ref() noexcept = default;

  template <class Receiver>
    requires(::std::derived_from<Receiver, receiver_interface>)
  explicit any_receiver_ref(Receiver& receiver)
      : receiver_{&receiver} {}

  any_receiver_ref(const any_receiver_ref& other) noexcept
      : receiver_{other.receiver_} {}

  any_receiver_ref(any_receiver_ref&& other) noexcept
      : receiver_{std::move(other.receiver_)} {}

  any_receiver_ref& operator=(const any_receiver_ref& other) noexcept { // NOLINT
    this->receiver_ = other.receiver_;
    return *this;
  }

  any_receiver_ref& operator=(any_receiver_ref&& other) noexcept {
    this->receiver_ = std::move(other.receiver_);
    return *this;
  }

  ~any_receiver_ref() = default;

  template <class... Args> void set_value(Args&&... args) && noexcept {
    assert(this->receiver_);
    this->receiver_->set_complete(::beman::execution26::set_value, ::std::forward<Args>(args)...);
  }

  template <class Error> void set_error(Error&& error) && noexcept {
    assert(this->receiver_);
    this->receiver_->set_complete(::beman::execution26::set_error, ::std::forward<Error>(error));
  }

  void set_stopped() && noexcept {
    assert(this->receiver_);
    this->receiver_->set_complete(::beman::execution26::set_stopped);
  }

  auto get_env() const noexcept -> Env {
    assert(this->receiver_);
    return this->receiver_->get_env();
  }

 private:
  receiver_interface* receiver_;
};

} // namespace beman::task::detail

#endif