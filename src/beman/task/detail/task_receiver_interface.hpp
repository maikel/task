// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_TASK_RECEIVER_INTERFACE_HPP
#define BEMAN_TASK_DETAIL_TASK_RECEIVER_INTERFACE_HPP

#include <exception>

namespace beman::task::detail {
struct void_t {};
template <class Tp> struct value_or_void {
  using type = Tp;
};
template <> struct value_or_void<void> {
  using type = void_t;
};
template <class Tp> using value_or_void_t = typename ::beman::task::detail::value_or_void<Tp>::type;

template <class Tp> class task_receiver_interface {
 public:
  task_receiver_interface() = default;
  task_receiver_interface(const task_receiver_interface&) = default;
  task_receiver_interface& operator=(const task_receiver_interface&) = default;
  task_receiver_interface(task_receiver_interface&&) = default;
  task_receiver_interface& operator=(task_receiver_interface&&) = default;

  virtual void set_value(::beman::task::detail::value_or_void_t<Tp>) noexcept = 0;
  virtual void set_error(const ::std::exception_ptr&) noexcept = 0;
  virtual void set_stopped() noexcept = 0;

 protected:
  ~task_receiver_interface() = default;
};

} // namespace beman::task::detail

#endif // BEMAN_TASK_DETAIL_TASK_RECEIVER_INTERFACE_HPP