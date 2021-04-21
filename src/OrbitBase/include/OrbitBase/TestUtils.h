// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_TEST_UTILS_H_
#define ORBIT_BASE_TEST_UTILS_H_

#include <absl/strings/match.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <gmock/gmock.h>

namespace orbit_base {

MATCHER(HasValue, absl::StrCat(negation ? "Has no" : "Has a", " value.")) {
  if (arg.has_error()) {
    *result_listener << "Error: " << arg.error().message();
  }
  return arg.has_value();
}

MATCHER(HasNoValue, absl::StrCat(negation ? "Has a" : "Has no", " value.")) {
  if (arg.has_error()) {
    *result_listener << "Error: " << arg.error().message();
  }
  return !arg.has_value();
}

MATCHER(HasError, absl::StrCat(negation ? "Has no" : "Has an", " error.")) {
  if (arg.has_error()) {
    *result_listener << "Error: " << arg.error().message();
  }
  return arg.has_error();
}

MATCHER(HasNoError, absl::StrCat(negation ? "Has an" : "Has no", " error.")) {
  if (arg.has_error()) {
    *result_listener << "Error: " << arg.error().message();
  }
  return !arg.has_error();
}

MATCHER_P(HasErrorContaining, value,
          absl::StrCat(negation ? "Has no" : "Has an",
                       absl::StrFormat(" error containing \"%s\".", value))) {
  if (arg.has_error()) {
    *result_listener << "Error: " << arg.error().message();
  }
  return arg.has_error() && absl::StrContains(arg.error().message(), value);
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_TEST_UTILS_H_
