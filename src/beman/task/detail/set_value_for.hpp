// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_SET_VALUE_FOR_HPP
#define BEMAN_TASK_DETAIL_SET_VALUE_FOR_HPP

#include <beman/execution26/execution.hpp>

namespace beman::task::detail {

template <class... Args> struct set_value_for_ {
  using type = ::beman::execution26::set_value_t(Args...);
};
template <> struct set_value_for_<void> {
  using type = ::beman::execution26::set_value_t();
};

template <class... Args> using set_value_for = typename set_value_for_<Args...>::type;

} // namespace beman::task::detail

#endif