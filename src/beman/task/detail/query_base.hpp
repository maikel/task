// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_QUERY_BASE_HPP
#define BEMAN_TASK_DETAIL_QUERY_BASE_HPP

namespace beman::task::detail {
template <class QuerySig> class query_base;

template <class Ret, class... Args> class query_base<Ret(Args...)> {
 public:
  query_base() = default;
  query_base(const query_base&) = delete;
  query_base& operator=(const query_base&) = delete;
  query_base(query_base&&) = delete;
  query_base& operator=(query_base&&) = delete;

  virtual Ret query(Args...) const noexcept = 0;

 protected:
  ~query_base() = default;
};
} // namespace beman::task::detail

#endif