// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QDateTime>
#include <QJsonDocument>
#include <QVector>

#include "OrbitGgp/Error.h"
#include "OrbitGgp/Instance.h"

namespace OrbitGgp {

TEST(InstanceTests, GetListFromJson) {
  {
    // invalid json
    const auto json = QString("json").toUtf8();
    EXPECT_FALSE(Instance::GetListFromJson(json));
  }

  {
    // empty json
    const auto json = QString("[]").toUtf8();
    const auto empty_instances = Instance::GetListFromJson(json);
    ASSERT_TRUE(empty_instances);
    EXPECT_TRUE(empty_instances.value().empty());
  }

  {
    // one empty json object
    const auto json = QString("[{}]").toUtf8();
    EXPECT_FALSE(Instance::GetListFromJson(json));
  }

  {
    // one element with invalid date
    // pretty json:
    // [
    //  {
    //   "displayName": "a display name",
    //   "id": "instance id",
    //   "ipAddress": "1.1.0.1",
    //   "lastUpdated": "2020-29-09T09:55:20Z",
    //   "owner": "a username",
    //   "pool": "a pool",
    //   "other key": "other value",
    //   "other complex object": {
    //    "object key": "object value"
    //   },
    //  }
    // ]
    const auto json = QString(
                          "[{\"displayName\":\"a display name\",\"id\":\"instance "
                          "id\",\"ipAddress\":\"1.1.0.1\",\"lastUpdated\":\"2020-29-09T09:"
                          "55:20Z\",\"owner\":\"a username\",\"pool\":\"a pool\",\"other "
                          "key\":\"other value\",\"other complex object\":{\"object "
                          "key\":\"object value\"}}]")
                          .toUtf8();
    const auto result = Instance::GetListFromJson(json);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().value(), static_cast<int>(OrbitGgp::Error::kUnableToParseJson));
  }

  {
    // one full json object
    // pretty json:
    // [
    //  {
    //   "displayName": "a display name",
    //   "id": "instance id",
    //   "ipAddress": "1.1.0.1",
    //   "lastUpdated": "2020-04-09T09:55:20Z",
    //   "owner": "a username",
    //   "pool": "a pool",
    //   "other key": "other value",
    //   "other complex object": {
    //    "object key": "object value"
    //   },
    //  }
    // ]
    const auto json = QString(
                          "[{\"displayName\":\"a display name\",\"id\":\"instance "
                          "id\",\"ipAddress\":\"1.1.0.1\",\"lastUpdated\":\"2020-04-09T09:"
                          "55:20Z\",\"owner\":\"a username\",\"pool\":\"a pool\",\"other "
                          "key\":\"other value\",\"other complex object\":{\"object "
                          "key\":\"object value\"}}]")
                          .toUtf8();
    const auto result = Instance::GetListFromJson(json);
    ASSERT_TRUE(result);
    const QVector<Instance> instances = std::move(result.value());
    ASSERT_EQ(instances.size(), 1);
    const Instance instance = instances[0];
    EXPECT_EQ(instance.display_name, QString{"a display name"});
    EXPECT_EQ(instance.id, QString{"instance id"});
    EXPECT_EQ(instance.ip_address, QString{"1.1.0.1"});
    EXPECT_EQ(instance.last_updated,
              QDateTime::fromString(QString{"2020-04-09T09:55:20Z"}, Qt::ISODate));
    EXPECT_EQ(instance.owner, QString{"a username"});
    EXPECT_EQ(instance.pool, QString{"a pool"});
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

  EXPECT_NE(test_instance_0, test_instance_1);

  test_instance_1.display_name = "a display name";
  test_instance_1.id = "a id";
  test_instance_1.ip_address = "1.1.0.1";
  test_instance_1.last_updated = QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instance_1.owner = "a username";
  test_instance_1.pool = "a pool";

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

  EXPECT_TRUE(test_instance_0 != test_instance_1);

  test_instance_1.display_name = "a display name";
  test_instance_1.id = "a id";
  test_instance_1.ip_address = "1.1.0.1";
  test_instance_1.last_updated = QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instance_1.owner = "a username";
  test_instance_1.pool = "a pool";

  EXPECT_FALSE(test_instance_0 != test_instance_1);
}

TEST(InstanceTests, QMetaTypeId) {
  EXPECT_STREQ("OrbitGgp::Instance", QMetaType::typeName(qMetaTypeId<Instance>()));
}

}  // namespace OrbitGgp
