// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_TASK_HPP
#define BEMAN_TASK_TASK_HPP

#include <beman/task/detail/basic_task.hpp>
#include <beman/task/detail/scheduler_affine_context.hpp>

namespace beman::task {

template <class Ret>
using task = ::beman::task::basic_task<Ret, ::beman::task::scheduler_affine_context<>>;

} // namespace beman::task

#endif