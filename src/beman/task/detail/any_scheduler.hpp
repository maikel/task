// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_ANY_SCHEDULER_HPP
#define BEMAN_TASK_DETAIL_ANY_SCHEDULER_HPP

#include <beman/task/detail/any_receiver_ref.hpp>

namespace beman::task::detail {

class any_scheduler {
 private:
  class operation_interface {
   public:
    virtual void start() noexcept = 0;

   protected:
    ~operation_interface() = default;
  };

  class sender_interface {
   public:
    using completion_signatures = ::beman::execution26::completion_signatures<
        ::beman::execution26::set_value_t(), ::beman::execution26::set_error_t(std::exception_ptr),
        ::beman::execution26::set_stopped_t()>;

    virtual auto connect(::beman::task::detail::any_receiver_ref<completion_signatures,
                                                                 ::beman::execution26::empty_env>
                             receiver) && -> std::unique_ptr<operation_interface> = 0;

   protected:
    ~sender_interface() = default;
  };

  class sender {};
};

} // namespace beman::task::detail

#endif