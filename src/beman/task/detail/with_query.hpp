// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_WITH_QUERY_HPP
#define BEMAN_TASK_DETAIL_WITH_QUERY_HPP

#include <type_traits>
#include <utility>

namespace beman::task::detail {

template <class Tag, class Value> auto with_query(Tag, Value&& value) {
  struct env_t {
    auto query(Tag) const noexcept -> const std::remove_cvref_t<Value>& { return value_; }
    std::remove_cvref_t<Value> value_;
  };
  return env_t{std::forward<Value>(value)};
}

} // namespace beman::task::detail

#endif // BEMAN_TASK_DETAIL_WITH_QUERY_HPP