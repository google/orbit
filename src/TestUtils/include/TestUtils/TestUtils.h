// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TEST_UTILS_TEST_UTILS_H_
#define TEST_UTILS_TEST_UTILS_H_

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <gmock/gmock.h>

#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Result.h"

namespace orbit_test_utils {

MATCHER(HasValue, absl::StrCat(negation ? "Has no" : "Has a", " value.")) {
  if (arg.has_error()) {
    *result_listener << "Error: " << arg.error().message();
  }
  return arg.has_value();
}

MATCHER_P(HasValue, value_matcher, absl::StrCat(negation ? "Has no" : "Has a", " value.")) {
  if (arg.has_error()) {
    *result_listener << "Error: " << arg.error().message();
  }
  return arg.has_value() && ExplainMatchResult(value_matcher, arg.value(), result_listener);
}

MATCHER(HasNoError, absl::StrCat(negation ? "Has an" : "Has no", " error.")) {
  if (arg.has_error()) {
    *result_listener << "Error: " << arg.error().message();
  }
  return !arg.has_error();
}

MATCHER(HasError, absl::StrCat(negation ? "Has no" : "Has an", " error.")) {
  return arg.has_error();
}

MATCHER_P(HasError, value_matcher, absl::StrCat(negation ? "Has no" : "Has an", " error.")) {
  if (arg.has_error()) {
    *result_listener << "Error: " << arg.error().message();
  }
  return arg.has_error() && ExplainMatchResult(value_matcher, arg.error(), result_listener);
}

MATCHER_P(HasErrorWithMessage, value,
          absl::StrCat(negation ? "Has no" : "Has an",
                       absl::StrFormat(" error containing \"%s\".", value))) {
  if (arg.has_error()) {
    *result_listener << "Error: " << arg.error().message();
  }
  return arg.has_error() &&
         ExplainMatchResult(testing::HasSubstr(value), arg.error().message(), result_listener);
}

MATCHER(HasBeenCanceled, absl::StrCat("Has", negation ? " not" : "", " been cancelled.")) {
  return orbit_base::IsCanceled(arg);
}

MATCHER(HasNotBeenCanceled, absl::StrCat("Has", negation ? "" : " not", " been cancelled.")) {
  return !orbit_base::IsCanceled(arg);
}

}  // namespace orbit_test_utils

#endif  // TEST_UTILS_TEST_UTILS_H_
