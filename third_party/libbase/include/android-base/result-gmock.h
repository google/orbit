/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android-base/result.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

/*
 * Matchers for android::base::Result<T> that produces human-readable test results.
 *
 * Example:
 *
 * Result<int> result = ...
 *
 * using namespace ::android::base::testing;
 * using namespace ::testing;
 *
 * EXPECT_THAT(result, Ok());
 * EXPECT_THAT(result, Not(Ok()));
 * EXPECT_THAT(result, HasValue(5));
 * EXPECT_THAT(result, HasError(WithCode(EBADF)));
 * EXPECT_THAT(result, HasError(WithMessageMessage("expected error message")));
 *
 * // Advance usage
 * EXPECT_THAT(result, AnyOf(Ok(), HasError(WithCode(EACCES)));
 * EXPECT_THAT(result, HasError(WithCode(AnyOf(EBADF, EACCES))) << "Unexpected code from library";
 */

namespace android::base {

template <typename T>
void PrintTo(const Result<T>& result, std::ostream* os) {
  if (result.ok()) {
    *os << "OK: " << ::testing::PrintToString(result.value());
  } else {
    *os << "Error: " << result.error();
  }
}

template <>
void PrintTo(const Result<void>& result, std::ostream* os) {
  if (result.ok()) {
    *os << "OK";
  } else {
    *os << "Error: " << result.error();
  }
}

namespace testing {

MATCHER(Ok, "") {
  if (arg.ok()) {
    *result_listener << "result is OK";
    return true;
  }
  *result_listener << "error is " << arg.error();
  return false;
}

MATCHER_P(HasValue, value_matcher, "") {
  if (arg.ok()) {
    return ::testing::ExplainMatchResult(value_matcher, arg.value(), result_listener);
  }
  *result_listener << "error is " << arg.error();
  return false;
}

MATCHER_P(HasError, error_matcher, "") {
  if (!arg.ok()) {
    return ::testing::ExplainMatchResult(error_matcher, arg.error(), result_listener);
  }
  *result_listener << "result is OK";
  return false;
}

MATCHER_P(WithCode, code_matcher, "") {
  *result_listener << "actual error is " << arg;
  return ::testing::ExplainMatchResult(code_matcher, arg.code(), result_listener);
}

MATCHER_P(WithMessage, messgae_matcher, "") {
  *result_listener << "actual error is " << arg;
  return ::testing::ExplainMatchResult(messgae_matcher, arg.message(), result_listener);
}

}  // namespace testing
}  // namespace android::base
