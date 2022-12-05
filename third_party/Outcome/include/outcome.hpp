// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_OUTCOME_OUTCOME_HPP_
#define THIRD_PARTY_OUTCOME_OUTCOME_HPP_

// This header is a shim layer which takes the Boost version of Outcome and makes it behave like the
// standalone version. It implements just enough to make Orbit compile with Boost..
//
// It also tries to keep Boost headers and libraries out of the include and dependency graph.
// Only outcome headers from Boost are required. No linking against Boost libraries.

// This will not be used because we always use the terminate policy, but it is needed to
// consume the following includes.
#define BOOST_OUTCOME_THROW_EXCEPTION(...) std::abort();

#include <boost/outcome/basic_result.hpp> // IWYU pragma: export
#include <boost/outcome/config.hpp> // IWYU pragma: export
#include <boost/outcome/std_result.hpp> // IWYU pragma: export
#include <boost/outcome/success_failure.hpp> // IWYU pragma: export
#include <boost/outcome/try.hpp> // IWYU pragma: export
#include <system_error>
#include <utility>

#define OUTCOME_V2_NAMESPACE outcome_v2

// Outcome 2.2's OUTCOME_TRY macro behaves differently than the one from previous versions.
// Boost 1.75 and earlier contain an older version of Outcome than 2.2, so we make some changes
// to the behaviour of BOOST_OUTCOME_TRY to make it behaves like OUTCOME_TRY from Outcome 2.2.
#if ((BOOST_VERSION / 100) % 1000) < 76
#define OUTCOME_TRY_INVOKE_TRY2(result, expr) \
  BOOST_OUTCOME_TRY2_SUCCESS_LIKELY(BOOST_OUTCOME_TRY_UNIQUE_NAME, result, expr)
#define OUTCOME_TRY_INVOKE_TRY1(expr) BOOST_OUTCOME_TRYV(expr)

#define OUTCOME_TRY(...) BOOST_OUTCOME_TRY_CALL_OVERLOAD(OUTCOME_TRY_INVOKE_TRY, __VA_ARGS__)
#else
#define OUTCOME_TRY(...) BOOST_OUTCOME_TRY(__VA_ARGS__)
#endif

namespace OUTCOME_V2_NAMESPACE {
namespace policy = BOOST_OUTCOME_V2_NAMESPACE::policy;

template <typename R, typename S = std::error_code,
          typename NoValuePolicy = policy::default_policy<R, S, void>>
using result = BOOST_OUTCOME_V2_NAMESPACE::std_result<R, S, NoValuePolicy>;

template <typename... Args>
auto success(Args&&... args)
    -> decltype(BOOST_OUTCOME_V2_NAMESPACE::success(std::forward<Args>(args)...)) {
  return BOOST_OUTCOME_V2_NAMESPACE::success(std::forward<Args>(args)...);
}

template <typename... Args>
auto failure(Args&&... args)
    -> decltype(BOOST_OUTCOME_V2_NAMESPACE::failure(std::forward<Args>(args)...)) {
  return BOOST_OUTCOME_V2_NAMESPACE::failure(std::forward<Args>(args)...);
}

}  // namespace OUTCOME_V2_NAMESPACE

#endif  // THIRD_PARTY_OUTCOME_OUTCOME_HPP_