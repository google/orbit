// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "OrbitBase/Result.h"
#include "TestUtils/TestUtils.h"

static ErrorMessageOr<std::string> ReturnString() { return "This is fine."; }

static ErrorMessageOr<std::string> ReturnError() { return ErrorMessage{"This is not fine."}; }

namespace orbit_test_utils {

TEST(TestUtils, HasValue) {
  EXPECT_THAT(ReturnString(), HasValue());
  EXPECT_THAT(ReturnString(), HasValue("This is fine."));
  EXPECT_THAT(ReturnString(), HasValue(testing::Eq("This is fine.")));
  EXPECT_THAT(ReturnString(), HasValue(testing::EndsWith("fine.")));

  EXPECT_THAT(ReturnError(), testing::Not(HasValue()));
  EXPECT_THAT(ReturnError(), testing::Not(HasValue(testing::Eq("This is fine."))));
}

TEST(TestUtils, HasError) {
  EXPECT_THAT(ReturnString(), testing::Not(HasError("This is not fine")));
  EXPECT_THAT(ReturnString(), HasValue("This is fine."));
  EXPECT_THAT(ReturnString(), HasValue(testing::Eq("This is fine.")));
  EXPECT_THAT(ReturnString(), HasValue(testing::EndsWith("fine.")));

  EXPECT_THAT(ReturnError(), HasError("This is not fine."));
  EXPECT_THAT(ReturnError(), HasError("not fine."));
  EXPECT_THAT(ReturnError(), testing::Not(HasError("Other error message")));
}

TEST(TestUtils, HasNoError) {
  EXPECT_THAT(ReturnString(), HasNoError());
  EXPECT_THAT(ReturnError(), testing::Not(HasNoError()));
}

}  // namespace orbit_test_utils