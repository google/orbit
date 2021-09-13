// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <qvector.h>

#include <QString>

#include "OrbitGgp/Project.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ggp {

TEST(OrbitGgpProject, GetListFromJson) {
  {  // invalid json
    const auto json = QString("json").toUtf8();
    EXPECT_THAT(Project::GetListFromJson(json), orbit_test_utils::HasError("Unable to parse JSON"));
  }

  {
    // empty json
    const auto json = QString("[]").toUtf8();
    const auto empty_instances = Project::GetListFromJson(json);
    ASSERT_THAT(empty_instances, orbit_test_utils::HasValue());
    EXPECT_TRUE(empty_instances.value().empty());
  }

  {
    // one empty json object
    const auto json = QString("[{}]").toUtf8();
    EXPECT_THAT(Project::GetListFromJson(json), orbit_test_utils::HasError("Unable to parse JSON"));
  }

  {
    // one partial (invalid) element
    const auto json_string = QString(R"([
 {
  "displayName":"display name"
 }
])");
    EXPECT_THAT(Project::GetListFromJson(json_string.toUtf8()),
                orbit_test_utils::HasError("Unable to parse JSON"));
  }

  {
    // one valid and one invalid element
    const auto json_string = QString(R"([
 {
  "displayName":"a display name",
  "id":"project id"
 },
 {
  "displayName":"second display name",
  "wrong id identifier":"project id 2"
 }
])");
    EXPECT_THAT(Project::GetListFromJson(json_string.toUtf8()),
                orbit_test_utils::HasError("Unable to parse JSON"));
  }

  {
    // two valid elements
    const auto json_string = QString(R"([
 {
  "displayName":"a display name",
  "id":"project id"
 },
 {
  "displayName":"second display name",
  "id":"project id 2"
 }
])");
    const auto result = Project::GetListFromJson(json_string.toUtf8());
    ASSERT_THAT(result, orbit_test_utils::HasNoError());
    const QVector<Project>& projects{result.value()};
    ASSERT_EQ(projects.size(), 2);
    EXPECT_EQ(projects[0].display_name, "a display name");
    EXPECT_EQ(projects[0].id, "project id");
    EXPECT_EQ(projects[1].display_name, "second display name");
    EXPECT_EQ(projects[1].id, "project id 2");
  }
}

}  // namespace orbit_ggp