// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_TASK_DETAIL_AS_EXCEPTION_PTR_HPP
#define BEMAN_TASK_DETAIL_AS_EXCEPTION_PTR_HPP

#include <concepts>
#include <exception>
#include <system_error>
#include <type_traits>

namespace beman::task::detail {

template <class Error> auto as_exception_ptr(Error&& error) noexcept {
  using Decayed = ::std::remove_cvref_t<Error>;
  if constexpr (::std::same_as<Decayed, ::std::exception_ptr>) {
    return ::std::forward<Error>(error);
  } else if constexpr (::std::same_as<Decayed, ::std::error_code>) {
    return ::std::make_exception_ptr(::std::system_error(::std::forward<Error>(error)));
  } else {
    return ::std::make_exception_ptr(::std::forward<Error>(error));
  }
}

} // namespace beman::task::detail

#endif