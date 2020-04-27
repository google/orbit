// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QDateTime>
#include <QJsonDocument>
#include <QVector>

#include "OrbitGgp/GgpInstance.h"

TEST(GgpInstanceTests, GetListFromJson) {
  QByteArray json;
  QVector<GgpInstance> test_instances;
  GgpInstance test_instance;

  // invalid json
  json = QString("json").toUtf8();
  test_instances = GgpInstance::GetListFromJson(json);
  EXPECT_TRUE(test_instances.empty());

  // empty json
  json = QString("[]").toUtf8();
  test_instances = GgpInstance::GetListFromJson(json);
  EXPECT_TRUE(test_instances.empty());

  // one empty json object
  json = QString("[{}]").toUtf8();
  test_instances = GgpInstance::GetListFromJson(json);
  ASSERT_EQ(test_instances.size(), 1);
  test_instance = test_instances[0];
  EXPECT_TRUE(test_instance.display_name.isEmpty());
  EXPECT_TRUE(test_instance.id.isEmpty());
  EXPECT_TRUE(test_instance.ip_address.isEmpty());
  EXPECT_EQ(test_instance.last_updated, QDateTime{});
  EXPECT_TRUE(test_instance.owner.isEmpty());
  EXPECT_TRUE(test_instance.pool.isEmpty());

  // two empty json objects
  json = QString("[{},{}]").toUtf8();
  test_instances = GgpInstance::GetListFromJson(json);
  ASSERT_EQ(test_instances.size(), 2);
  EXPECT_EQ(test_instances[0], GgpInstance{});
  EXPECT_EQ(test_instances[1], GgpInstance{});

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
  json = QString(
             "[{\"displayName\":\"a display name\",\"id\":\"instance "
             "id\",\"ipAddress\":\"1.1.0.1\",\"lastUpdated\":\"2020-04-09T09:"
             "55:20Z\",\"owner\":\"a username\",\"pool\":\"a pool\",\"other "
             "key\":\"other value\",\"other complex object\":{\"object "
             "key\":\"object value\"}}]")
             .toUtf8();
  test_instances = GgpInstance::GetListFromJson(json);
  ASSERT_EQ(test_instances.size(), 1);
  test_instance = test_instances[0];
  EXPECT_EQ(test_instance.display_name, QString{"a display name"});
  EXPECT_EQ(test_instance.id, QString{"instance id"});
  EXPECT_EQ(test_instance.ip_address, QString{"1.1.0.1"});
  EXPECT_EQ(
      test_instance.last_updated,
      QDateTime::fromString(QString{"2020-04-09T09:55:20Z"}, Qt::ISODate));
  EXPECT_EQ(test_instance.owner, QString{"a username"});
  EXPECT_EQ(test_instance.pool, QString{"a pool"});
}

TEST(GgpInstanceTests, CmpById) {
  GgpInstance test_instance_0;
  GgpInstance test_instance_1;

  // Empty id
  EXPECT_FALSE(GgpInstance::CmpById(test_instance_0, test_instance_1));

  // Same id
  test_instance_0.id = QString{"id"};
  test_instance_1.id = QString{"id"};
  EXPECT_FALSE(GgpInstance::CmpById(test_instance_0, test_instance_1));

  // first < second
  test_instance_0.id = QString{"id a"};
  test_instance_1.id = QString{"id b"};
  EXPECT_TRUE(GgpInstance::CmpById(test_instance_0, test_instance_1));

  // first > second
  test_instance_0.id = QString{"id b"};
  test_instance_1.id = QString{"id a"};
  EXPECT_FALSE(GgpInstance::CmpById(test_instance_0, test_instance_1));
}

TEST(GgpInstanceTests, EqualToOperator) {
  GgpInstance test_instance_0;
  GgpInstance test_instance_1;

  EXPECT_EQ(test_instance_0, test_instance_1);

  test_instance_0.display_name = "a display name";
  test_instance_0.id = "a id";
  test_instance_0.ip_address = "1.1.0.1";
  test_instance_0.last_updated =
      QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instance_0.owner = "a username";
  test_instance_0.pool = "a pool";

  EXPECT_NE(test_instance_0, test_instance_1);

  test_instance_1.display_name = "a display name";
  test_instance_1.id = "a id";
  test_instance_1.ip_address = "1.1.0.1";
  test_instance_1.last_updated =
      QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instance_1.owner = "a username";
  test_instance_1.pool = "a pool";

  EXPECT_EQ(test_instance_0, test_instance_1);
}

TEST(GgpInstanceTests, NotEqualToOperator) {
  GgpInstance test_instance_0;
  GgpInstance test_instance_1;

  EXPECT_FALSE(test_instance_0 != test_instance_1);

  test_instance_0.display_name = "a display name";
  test_instance_0.id = "a id";
  test_instance_0.ip_address = "1.1.0.1";
  test_instance_0.last_updated =
      QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instance_0.owner = "a username";
  test_instance_0.pool = "a pool";

  EXPECT_TRUE(test_instance_0 != test_instance_1);

  test_instance_1.display_name = "a display name";
  test_instance_1.id = "a id";
  test_instance_1.ip_address = "1.1.0.1";
  test_instance_1.last_updated =
      QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instance_1.owner = "a username";
  test_instance_1.pool = "a pool";

  EXPECT_FALSE(test_instance_0 != test_instance_1);
}

TEST(GgpInstanceTests, QMetaTypeId) {
  EXPECT_STREQ("GgpInstance", QMetaType::typeName(qMetaTypeId<GgpInstance>()));
}