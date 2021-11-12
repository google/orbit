// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QVector>
#include <Qt>
#include <memory>
#include <system_error>
#include <utility>

#include "OrbitBase/Result.h"
#include "OrbitGgp/Error.h"
#include "OrbitGgp/Instance.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ggp {

// Instance with invalid date
// pretty json:
//  {
//   "displayName": "a display name",
//   "id": "instance id",
//   "ipAddress": "1.1.0.1",
//   "lastUpdated": "2020-29-09T09:55:20Z",
//   "owner": "a username",
//   "pool": "a pool",
//   "state": "a state",
//   "other key": "other value",
//   "other complex object": {
//    "object key": "object value"
//   },
//  }
const auto invalid_timestamp_instance_json = QString(
    "{\"displayName\":\"a display name\",\"id\":\"instance "
    "id\",\"ipAddress\":\"1.1.0.1\",\"lastUpdated\":\"2020-29-09T09:55:20Z\",\"owner\":\"a "
    "username\",\"pool\":\"a pool\",\"state\":\"a state\",\"other key\":\"other "
    "value\",\"other complex object\":{\"object key\":\"object value\"}}");

// Correct instance JSON declaration
// pretty json:
//  {
//   "displayName": "a display name",
//   "id": "instance id",
//   "ipAddress": "1.1.0.1",
//   "lastUpdated": "2020-04-09T09:55:20Z",
//   "owner": "a username",
//   "pool": "a pool",
//   "state": "a state",
//   "other key": "other value",
//   "other complex object": {
//    "object key": "object value"
//   },
//  }
const auto valid_instance_json = QString(
    "{\"displayName\":\"a display name\",\"id\":\"instance "
    "id\",\"ipAddress\":\"1.1.0.1\",\"lastUpdated\":\"2020-04-09T09:55:20Z\",\"owner\":\"a "
    "username\",\"pool\":\"a pool\",\"state\":\"a state\",\"other key\":\"other "
    "value\",\"other complex object\":{\"object key\":\"object value\"}}");

// Instance with missing "id" child
// pretty json:
//  {
//   "displayName": "a display name",
//   "ipAddress": "1.1.0.1",
//   "lastUpdated": "2020-04-09T09:55:20Z",
//   "owner": "a username",
//   "pool": "a pool",
//   "state": "a state",
//   "other key": "other value",
//   "other complex object": {
//    "object key": "object value"
//   },
//  }
const auto missing_element_instance_json = QString(
    "{\"displayName\":\"a display name\",\"ipAddress\":\"1.1.0.1\","
    "\"lastUpdated\":\"2020-04-09T09:55:20Z\",\"owner\":\"a "
    "username\",\"pool\":\"a pool\",\"state\":\"a state\",\"other key\":\"other "
    "value\",\"other complex object\":{\"object key\":\"object value\"}}");

const Instance valid_instance{
    QString{"a display name"}, QString{"instance id"},
    QString{"1.1.0.1"},        QDateTime::fromString(QString{"2020-04-09T09:55:20Z"}, Qt::ISODate),
    QString{"a username"},     QString{"a pool"},
    QString{"a state"}};

TEST(InstanceTests, GetListFromJson) {
  {
    // invalid json
    const auto json = QString("json").toUtf8();
    EXPECT_THAT(Instance::GetListFromJson(json),
                orbit_test_utils::HasError("Unable to parse JSON"));
  }

  {
    // empty json
    const auto json = QString("[]").toUtf8();
    const auto empty_instances = Instance::GetListFromJson(json);
    ASSERT_THAT(empty_instances, orbit_test_utils::HasValue());
    EXPECT_TRUE(empty_instances.value().empty());
  }

  {
    // one empty json object
    const auto json = QString("[{}]").toUtf8();
    EXPECT_THAT(Instance::GetListFromJson(json),
                orbit_test_utils::HasError("Unable to parse JSON"));
  }

  {
    const auto json = QString("[%1]").arg(invalid_timestamp_instance_json).toUtf8();
    const auto result = Instance::GetListFromJson(json);
    EXPECT_THAT(result, orbit_test_utils::HasError("Unable to parse JSON"));
  }

  {
    const auto json = QString("[%1]").arg(valid_instance_json).toUtf8();
    auto result = Instance::GetListFromJson(json);
    ASSERT_THAT(result, orbit_test_utils::HasValue());
    const QVector<Instance> instances = std::move(result.value());
    ASSERT_EQ(instances.size(), 1);
    const Instance instance = instances[0];
    EXPECT_EQ(instance, valid_instance);
  }
}

TEST(InstanceTests, CreateFromJson) {
  {
    // invalid json
    const auto json = QString("json").toUtf8();
    EXPECT_THAT(Instance::CreateFromJson(json), orbit_test_utils::HasError("Unable to parse JSON"));
  }

  {
    // empty json
    const auto json = QString("{}").toUtf8();
    const auto invalid_instance = Instance::CreateFromJson(json);
    EXPECT_THAT(invalid_instance, orbit_test_utils::HasError("Unable to parse JSON"));
  }

  {
    const auto json = missing_element_instance_json.toUtf8();
    EXPECT_THAT(Instance::CreateFromJson(json), orbit_test_utils::HasError("Unable to parse JSON"));
  }

  {
    const auto json = invalid_timestamp_instance_json.toUtf8();
    const auto result = Instance::CreateFromJson(json);
    EXPECT_THAT(result, orbit_test_utils::HasError("Unable to parse JSON"));
  }

  {
    const auto json = valid_instance_json.toUtf8();
    auto result = Instance::CreateFromJson(json);
    ASSERT_THAT(result, orbit_test_utils::HasValue());
    EXPECT_EQ(result.value(), valid_instance);
  }
}

TEST(InstanceTests, CmpById) {
  Instance test_instance_0;
  Instance test_instance_1;

  // Empty id
  EXPECT_FALSE(Instance::CmpById(test_instance_0, test_instance_1));

  // Same id
  test_instance_0.id = QString{"id"};
  test_instance_1.id = QString{"id"};
  EXPECT_FALSE(Instance::CmpById(test_instance_0, test_instance_1));

  // first < second
  test_instance_0.id = QString{"id a"};
  test_instance_1.id = QString{"id b"};
  EXPECT_TRUE(Instance::CmpById(test_instance_0, test_instance_1));

  // first > second
  test_instance_0.id = QString{"id b"};
  test_instance_1.id = QString{"id a"};
  EXPECT_FALSE(Instance::CmpById(test_instance_0, test_instance_1));
}

TEST(InstanceTests, EqualToOperator) {
  Instance test_instance_0;
  Instance test_instance_1;

  EXPECT_EQ(test_instance_0, test_instance_1);

  test_instance_0.display_name = "a display name";
  test_instance_0.id = "a id";
  test_instance_0.ip_address = "1.1.0.1";
  test_instance_0.last_updated = QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instance_0.owner = "a username";
  test_instance_0.pool = "a pool";
  test_instance_0.state = "a state";

  EXPECT_NE(test_instance_0, test_instance_1);

  test_instance_1.display_name = "a display name";
  test_instance_1.id = "a id";
  test_instance_1.ip_address = "1.1.0.1";
  test_instance_1.last_updated = QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instance_1.owner = "a username";
  test_instance_1.pool = "a pool";
  test_instance_1.state = "a state";

  EXPECT_EQ(test_instance_0, test_instance_1);
}

TEST(InstanceTests, NotEqualToOperator) {
  Instance test_instance_0;
  Instance test_instance_1;

  EXPECT_FALSE(test_instance_0 != test_instance_1);

  test_instance_0.display_name = "a display name";
  test_instance_0.id = "a id";
  test_instance_0.ip_address = "1.1.0.1";
  test_instance_0.last_updated = QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instance_0.owner = "a username";
  test_instance_0.pool = "a pool";
  test_instance_0.state = "a state";

  EXPECT_TRUE(test_instance_0 != test_instance_1);

  test_instance_1.display_name = "a display name";
  test_instance_1.id = "a id";
  test_instance_1.ip_address = "1.1.0.1";
  test_instance_1.last_updated = QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instance_1.owner = "a username";
  test_instance_1.pool = "a pool";
  test_instance_1.state = "a state";

  EXPECT_FALSE(test_instance_0 != test_instance_1);
}

TEST(InstanceTests, QMetaTypeId) {
  EXPECT_STREQ("orbit_ggp::Instance", QMetaType::typeName(qMetaTypeId<Instance>()));
}

}  // namespace orbit_ggp
