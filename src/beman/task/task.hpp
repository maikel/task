// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_TASK_HPP
#define BEMAN_TASK_TASK_HPP

#include <beman/task/detail/basic_task.hpp>

namespace beman::task {

template <class Ret> using task = basic_task<Ret>;

} // namespace beman::task

#endif