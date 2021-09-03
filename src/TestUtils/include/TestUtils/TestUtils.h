// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TEST_UTILS_TEST_UTILS_H_
#define TEST_UTILS_TEST_UTILS_H_

#include <absl/strings/match.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <gmock/gmock.h>

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

MATCHER_P(HasError, value,
          absl::StrCat(negation ? "Has no" : "Has an",
                       absl::StrFormat(" error containing \"%s\".", value))) {
  if (arg.has_error()) {
    *result_listener << "Error: " << arg.error().message();
  }
  return arg.has_error() && absl::StrContains(arg.error().message(), value);
}

}  // namespace orbit_test_utils

#endif  // TEST_UTILS_TEST_UTILS_H_
