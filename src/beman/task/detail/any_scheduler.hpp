// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_ANY_SCHEDULER_HPP
#define BEMAN_TASK_DETAIL_ANY_SCHEDULER_HPP

#include <beman/task/detail/any_receiver_ref.hpp>
#include <beman/task/detail/join_envs.hpp>
#include <beman/task/detail/manual_lifetime.hpp>
#include <beman/task/detail/with_query.hpp>

#include <beman/execution26/execution.hpp>

#include <any>
#include <exception>
#include <memory>

namespace beman::task::detail {

class any_scheduler {
 private:
  struct operation_interface {
    operation_interface() noexcept = default;
    operation_interface(const operation_interface&) = default;
    operation_interface& operator=(const operation_interface&) = default;
    operation_interface(operation_interface&&) = default;
    operation_interface& operator=(operation_interface&&) = default;
    virtual ~operation_interface() = default;
    virtual void start() noexcept = 0;
  };

  template <class Sender, class Receiver>
  class operation_implementation : public operation_interface {
   public:
    explicit operation_implementation(Sender sender, Receiver receiver)
        : op_{::beman::execution26::connect(std::move(sender), std::move(receiver))} {}

    void start() noexcept override { ::beman::execution26::start(op_); }

   private:
    ::beman::execution26::connect_result_t<Sender, Receiver> op_;
  };

  struct sender_interface {
    using completion_signatures = ::beman::execution26::completion_signatures<
        ::beman::execution26::set_value_t(), ::beman::execution26::set_error_t(std::exception_ptr),
        ::beman::execution26::set_stopped_t()>;

    using any_receiver_ref =
        ::beman::task::detail::any_receiver_ref<completion_signatures,
                                                ::beman::execution26::empty_env>;

    sender_interface() noexcept = default;
    sender_interface(const sender_interface&) = default;
    sender_interface& operator=(const sender_interface&) = default;
    sender_interface(sender_interface&&) = default;
    sender_interface& operator=(sender_interface&&) = default;
    virtual ~sender_interface() = default;

    virtual auto connect(any_receiver_ref receiver) -> std::unique_ptr<operation_interface> = 0;

    virtual auto get_completion_scheduler() const noexcept -> any_scheduler = 0;
  };

  template <class Sender> class sender_implementation : public sender_interface {
   public:
    explicit sender_implementation(Sender sender) noexcept
        : sender_{std::move(sender)} {}

    auto connect(sender_interface::any_receiver_ref receiver)
        -> std::unique_ptr<operation_interface> override {
      using Receiver =
          ::beman::task::detail::any_receiver_ref<sender_interface::completion_signatures,
                                                  ::beman::execution26::empty_env>;
      return std::make_unique<operation_implementation<Sender, Receiver>>(std::move(sender_),
                                                                          std::move(receiver));
    }

    auto get_completion_scheduler() const noexcept -> any_scheduler override {
      return any_scheduler{
          ::beman::execution26::get_completion_scheduler<::beman::execution26::set_value_t>(
              ::beman::execution26::get_env(sender_))};
    }

   private:
    Sender sender_;
  };

  template <class Receiver> class schedule_operation {
   public:
    friend struct ::beman::execution26::start_t;
    using operation_state_concept = ::beman::execution26::operation_state_t;

    schedule_operation(std::unique_ptr<sender_interface> sender, Receiver receiver)
        : receiver_(std::move(receiver))
        , operation_(sender->connect(sender_interface::any_receiver_ref{receiver_})) {}

   private:
    class receiver_ref {
     public:
      using receiver_concept = ::beman::execution26::receiver_t;

      struct callback_type {
        receiver_ref& ref_;
        void operator()() const noexcept { ref_.stop_source_.request_stop(); }
      };

      using stop_token_type =
          ::beman::execution26::stop_token_of_t<::beman::execution26::env_of_t<Receiver>>;

      using stop_callback_type =
          ::beman::execution26::stop_callback_for_t<stop_token_type, callback_type>;

      explicit receiver_ref(Receiver receiver) noexcept
          : receiver_(std::move(receiver)) {}

      void start() noexcept {
        stop_token_type token =
            ::beman::execution26::get_stop_token(::beman::execution26::get_env(receiver_));
        stop_callback_.emplace(token, callback_type{*this});
      }

      template <class... Args> void set_value(Args&&... args) && noexcept {
        stop_callback_.reset();
        ::beman::execution26::set_value(std::move(receiver_), std::forward<Args>(args)...);
      }

      template <class Error> void set_error(Error&& error) && noexcept {
        stop_callback_.reset();
        ::beman::execution26::set_error(std::move(receiver_), std::forward<Error>(error));
      }

      void set_stopped() && noexcept {
        stop_callback_.reset();
        ::beman::execution26::set_stopped(std::move(receiver_));
      }

      auto get_env() const noexcept {
        return ::beman::task::detail::join_envs(
            ::beman::execution26::get_env(receiver_),
            ::beman::task::detail::with_query(::beman::execution26::get_stop_token,
                                              this->stop_source_.get_token()));
      }

      Receiver receiver_;
      ::beman::execution26::inplace_stop_source stop_source_;
      [[no_unique_address]]
      ::beman::task::detail::manual_lifetime<stop_callback_type> stop_callback_;
    };

    void start() noexcept {
      receiver_.start();
      operation_->start();
    }

    receiver_ref receiver_;
    std::unique_ptr<operation_interface> operation_;
  };

  class schedule_sender {
   public:
    using sender_concept = ::beman::execution26::sender_t;
    using completion_signatures = ::beman::execution26::completion_signatures<
        ::beman::execution26::set_value_t(), ::beman::execution26::set_error_t(std::exception_ptr),
        ::beman::execution26::set_stopped_t()>;

    schedule_sender() noexcept = default;

    schedule_sender(const schedule_sender& other) = delete;

    schedule_sender& operator=(const schedule_sender& other) = delete;

    schedule_sender(schedule_sender&& other) noexcept
        : sender_{std::move(other.sender_)} {}

    schedule_sender& operator=(schedule_sender&& other) noexcept {
      sender_ = std::move(other.sender_);
      return *this;
    }

    ~schedule_sender() = default;

    template <class OtherSender>
      requires(!std::same_as<std::remove_cvref_t<OtherSender>, schedule_sender>)
    explicit schedule_sender(OtherSender&& sender)
        : sender_{std::make_unique<sender_implementation<std::remove_cvref_t<OtherSender>>>(
              std::forward<OtherSender>(sender))} {}

   private:
    friend struct ::beman::execution26::connect_t;
    friend struct ::beman::execution26::get_env_t;

    template <class Receiver>
      requires ::beman::execution26::receiver_of<Receiver, completion_signatures>
    auto connect(Receiver receiver) && -> schedule_operation<Receiver> {
      return schedule_operation<Receiver>(std::move(sender_), std::move(receiver));
    }

    auto get_env() const noexcept {
      struct env_type {
        sender_interface* sender_;
        auto
        query(::beman::execution26::get_completion_scheduler_t<::beman::execution26::set_value_t>)
            const noexcept -> any_scheduler {
          assert(sender_);
          return sender_->get_completion_scheduler();
        }
      };
      return env_type{sender_.get()};
    }

    std::unique_ptr<sender_interface> sender_;
  };

  class scheduler_interface {
   public:
    scheduler_interface() noexcept = default;
    scheduler_interface(const scheduler_interface&) = default;
    scheduler_interface& operator=(const scheduler_interface&) = default;
    scheduler_interface(scheduler_interface&&) = default;
    scheduler_interface& operator=(scheduler_interface&&) = default;
    virtual ~scheduler_interface() = default;

    virtual auto schedule() -> schedule_sender = 0;

    virtual bool equal_to(const scheduler_interface& other) const noexcept = 0;
  };

  template <class OtherScheduler> struct scheduler_implementation : scheduler_interface {
    explicit scheduler_implementation(OtherScheduler scheduler) noexcept
        : scheduler_{std::move(scheduler)} {}

    scheduler_implementation(const scheduler_implementation& other)
        : scheduler_{other.scheduler_} {}

    scheduler_implementation(scheduler_implementation&& other) noexcept
        : scheduler_{std::move(other.scheduler_)} {}

    auto operator=(const scheduler_implementation& other) -> scheduler_implementation& = default;

    auto
    operator=(scheduler_implementation&& other) noexcept -> scheduler_implementation& = default;

    ~scheduler_implementation() override = default;

    auto schedule() -> schedule_sender override {
      return schedule_sender{::beman::execution26::schedule(scheduler_)};
    }

    bool equal_to(const scheduler_interface& other) const noexcept override {
      if (const auto* other_impl = dynamic_cast<const scheduler_implementation*>(&other)) {
        return scheduler_ == other_impl->scheduler_;
      }
      return false;
    }
    OtherScheduler scheduler_;
  };

  template <class OtherScheduler>
  inline static constexpr auto get_interface_fn_ =
      +[](std::any& storage) noexcept -> scheduler_interface& {
    auto ptr = std::any_cast<scheduler_implementation<OtherScheduler>>(&storage);
    assert(ptr);
    return *ptr;
  };

 public:
  using scheduler_concept = ::beman::execution26::scheduler_t;

  any_scheduler() noexcept = default;

  any_scheduler(const any_scheduler& other) noexcept
      : storage_{other.storage_}
      , get_interface_{other.get_interface_} {}

  any_scheduler(any_scheduler&& other) noexcept
      : storage_{std::move(other.storage_)}
      , get_interface_{other.get_interface_} {}

  any_scheduler& operator=(const any_scheduler& other) noexcept { // NOLINT
    storage_ = other.storage_;
    get_interface_ = other.get_interface_;
    return *this;
  }

  any_scheduler& operator=(any_scheduler&& other) noexcept {
    storage_ = std::move(other.storage_);
    get_interface_ = other.get_interface_;
    return *this;
  }

  ~any_scheduler() = default;

  template <class OtherSched>
    requires(!std::same_as<std::remove_cvref_t<OtherSched>, any_scheduler>)
  explicit any_scheduler(OtherSched&& sched) noexcept
      : storage_{scheduler_implementation<std::remove_cvref_t<OtherSched>>{
            std::forward<OtherSched>(sched)}}
      , get_interface_{get_interface_fn_<::std::remove_cvref_t<OtherSched>>} {}

  auto schedule() -> schedule_sender { return get_interface_(storage_).schedule(); }

  bool operator==(const any_scheduler& other) const noexcept {
    return get_interface_(const_cast<::std::any&>(storage_))
        .equal_to(get_interface_(const_cast<::std::any&>(other.storage_)));
  }

  bool operator!=(const any_scheduler& other) const noexcept { return !(*this == other); }

 private:
  std::any storage_{};
  auto (*get_interface_)(std::any&) noexcept -> scheduler_interface& {};
};

static_assert(::beman::execution26::scheduler<any_scheduler>);

} // namespace beman::task::detail

#endif