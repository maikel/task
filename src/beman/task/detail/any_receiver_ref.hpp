// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_ANY_RECEIVER_REF_HPP
#define BEMAN_TASK_DETAIL_ANY_RECEIVER_REF_HPP

#include <beman/execution26/execution.hpp>

#include <any>
#include <concepts>

namespace beman::task::detail {
template <class CompletionSigs, class Env> class any_receiver_ref;

template <class Env, class... Sigs>
class any_receiver_ref<::beman::execution26::completion_signatures<Sigs...>, Env> {
 private:
  template <class Sig> class receiver_completion_function;

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

  class receiver_interface : public receiver_completion_function<Sigs>... {
   public:
    receiver_interface() noexcept = default;
    receiver_interface(const receiver_interface&) = default;
    receiver_interface& operator=(const receiver_interface&) = default;
    receiver_interface(receiver_interface&&) = default;
    receiver_interface& operator=(receiver_interface&&) = default;

    using receiver_completion_function<Sigs>::set_complete...;
    virtual auto get_env() const noexcept -> Env = 0;

   protected:
    ~receiver_interface() = default;
  };

  template <class Derived, class Receiver, class... Sig> class receiver_completion_function_impl;

  template <class Derived, class Receiver>
  class receiver_completion_function_impl<Derived, Receiver> : public receiver_interface {
   public:
    receiver_completion_function_impl() noexcept = default;
    receiver_completion_function_impl(const receiver_completion_function_impl&) = default;
    receiver_completion_function_impl&
    operator=(const receiver_completion_function_impl&) = default;
    receiver_completion_function_impl(receiver_completion_function_impl&&) = default;
    receiver_completion_function_impl& operator=(receiver_completion_function_impl&&) = default;

   protected:
    ~receiver_completion_function_impl() = default;
  };

  template <class Derived, class Receiver, class Tag, class... Args, class... Sig>
  class receiver_completion_function_impl<Derived, Receiver, Tag(Args...), Sig...>
      : public receiver_completion_function_impl<Derived, Receiver, Sig...> {
   public:
    receiver_completion_function_impl() noexcept = default;
    receiver_completion_function_impl(const receiver_completion_function_impl&) = default;
    receiver_completion_function_impl&
    operator=(const receiver_completion_function_impl&) = default;
    receiver_completion_function_impl(receiver_completion_function_impl&&) = default;
    receiver_completion_function_impl& operator=(receiver_completion_function_impl&&) = default;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
    void set_complete(Tag tag, Args... args) noexcept override final {
      Receiver* receiver = static_cast<Derived*>(this)->receiver_;
      tag(std::move(*receiver), std::forward<Args>(args)...);
    }
#pragma clang diagnostic pop

   protected:
    ~receiver_completion_function_impl() = default;
  };

  template <class Receiver>
  class receiver_implementation final
      : public receiver_completion_function_impl<receiver_implementation<Receiver>, Receiver,
                                                 Sigs...> {
   public:
    explicit receiver_implementation(Receiver* receiver) noexcept
        : receiver_{receiver} {}

    receiver_implementation(const receiver_implementation& other) noexcept = default;
    receiver_implementation(receiver_implementation&& other) noexcept = default;
    receiver_implementation& operator=(const receiver_implementation& other) noexcept = default;
    receiver_implementation& operator=(receiver_implementation&& other) noexcept = default;
    ~receiver_implementation() = default;

    using receiver_completion_function_impl<receiver_implementation<Receiver>, Receiver,
                                            Sigs...>::set_complete;

    auto get_env() const noexcept -> Env override {
      return Env(::beman::execution26::get_env(receiver_));
    }

    Receiver* receiver_;
  };

 public:
  using receiver_concept = ::beman::execution26::receiver_t;

  template <class Receiver>
  inline static constexpr auto get_interface_fn_ = +[](std::any& storage) -> receiver_interface& {
    auto* ptr = ::std::any_cast<receiver_implementation<Receiver>>(&storage);
    assert(ptr);
    return *ptr;
  };

  any_receiver_ref() noexcept = default;

  template <class Receiver>
    requires(!::std::same_as<any_receiver_ref, Receiver>)
  explicit any_receiver_ref(Receiver& receiver)
      : storage_{receiver_implementation<Receiver>{&receiver}}
      , get_interface_{get_interface_fn_<Receiver>} {}

  any_receiver_ref(const any_receiver_ref& other) noexcept
      : storage_{other.storage_}
      , get_interface_{other.get_interface_} {}

  any_receiver_ref(any_receiver_ref&& other) noexcept
      : storage_{std::move(other.storage_)}
      , get_interface_{other.get_interface_} {}

  any_receiver_ref& operator=(const any_receiver_ref& other) noexcept { // NOLINT
    this->storage_ = &other.storage_;
    this->get_interface_ = other.get_interface_;
    return *this;
  }

  any_receiver_ref& operator=(any_receiver_ref&& other) noexcept {
    this->storage_ = std::move(other.storage_);
    this->get_interface_ = other.get_interface_;
    return *this;
  }

  ~any_receiver_ref() = default;

  template <class... Args> void set_value(Args&&... args) && noexcept {
    assert(this->get_interface_);
    assert(this->storage_.has_value());
    this->get_interface_(this->storage_)
        .set_complete(::beman::execution26::set_value, ::std::forward<Args>(args)...);
  }

  template <class Error> void set_error(Error&& error) && noexcept {
    assert(this->get_interface_);
    assert(this->storage_.has_value());
    this->get_interface_(this->storage_)
        .set_complete(::beman::execution26::set_error, ::std::forward<Error>(error));
  }

  void set_stopped() && noexcept {
    assert(this->get_interface_);
    assert(this->storage_.has_value());
    this->get_interface_(this->storage_).set_complete(::beman::execution26::set_stopped);
  }

  auto get_env() const noexcept -> Env {
    assert(this->get_interface_);
    assert(this->storage_.has_value());
    return this->get_interface_(const_cast<std::any&>(this->storage_)).get_env();
  }

 private:
  std::any storage_{};
  auto (*get_interface_)(std::any&) -> receiver_interface& {};
};

} // namespace beman::task::detail

#endif