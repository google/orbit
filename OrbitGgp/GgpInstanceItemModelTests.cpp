// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QAbstractItemModel>
#include <QDateTime>
#include <QVector>

#include "OrbitGgp/GgpInstance.h"
#include "OrbitGgp/GgpInstanceItemModel.h"

TEST(GgpInstanceItemModelTests, columnCount) {
  GgpInstanceItemModel model{};
  EXPECT_EQ(model.columnCount(), 6);

  model.SetInstances({GgpInstance{}});
  EXPECT_EQ(model.columnCount(), 6);
}

TEST(GgpInstanceItemModelTests, rowCount) {
  GgpInstanceItemModel model{};
  EXPECT_EQ(model.rowCount(), 0);

  model.SetInstances({GgpInstance{}});
  EXPECT_EQ(model.rowCount(), 1);

  model.SetInstances({GgpInstance{}, GgpInstance{}});
  EXPECT_EQ(model.rowCount(), 2);
}

TEST(GgpInstanceItemModelTests, index) {
  GgpInstanceItemModel model{};
  EXPECT_FALSE(model.index(0, 0).isValid());
  EXPECT_FALSE(model.index(0, 1).isValid());
  EXPECT_FALSE(model.index(0, 2).isValid());
  EXPECT_FALSE(model.index(0, 6).isValid());
  EXPECT_FALSE(model.index(1, 0).isValid());
  EXPECT_FALSE(model.index(1, 1).isValid());
  EXPECT_FALSE(model.index(1, 2).isValid());
  EXPECT_FALSE(model.index(1, 6).isValid());

  model.SetInstances({GgpInstance{}});
  EXPECT_TRUE(model.index(0, 0).isValid());
  EXPECT_TRUE(model.index(0, 1).isValid());
  EXPECT_TRUE(model.index(0, 2).isValid());
  EXPECT_FALSE(model.index(0, 6).isValid());
  EXPECT_FALSE(model.index(1, 0).isValid());
  EXPECT_FALSE(model.index(1, 1).isValid());
  EXPECT_FALSE(model.index(1, 2).isValid());
  EXPECT_FALSE(model.index(1, 6).isValid());

  model.SetInstances({GgpInstance{}, GgpInstance{}});
  EXPECT_TRUE(model.index(0, 0).isValid());
  EXPECT_TRUE(model.index(0, 1).isValid());
  EXPECT_TRUE(model.index(0, 2).isValid());
  EXPECT_FALSE(model.index(0, 6).isValid());
  EXPECT_TRUE(model.index(1, 0).isValid());
  EXPECT_TRUE(model.index(1, 1).isValid());
  EXPECT_TRUE(model.index(1, 2).isValid());
  EXPECT_FALSE(model.index(1, 6).isValid());

  // parent is set
  const QModelIndex cell_index = model.index(0, 0);
  EXPECT_FALSE(model.index(0, 0, cell_index).isValid());
}

TEST(GgpInstanceItemModelTests, headerData) {
  GgpInstanceItemModel model{};

  // role
  EXPECT_TRUE(model.headerData(0, Qt::Horizontal, Qt::DisplayRole).isValid());
  EXPECT_FALSE(model.headerData(0, Qt::Horizontal, Qt::UserRole).isValid());

  // Orientation
  EXPECT_TRUE(model.headerData(0, Qt::Horizontal).isValid());
  EXPECT_FALSE(model.headerData(0, Qt::Vertical).isValid());

  // section in range
  EXPECT_FALSE(model.headerData(-1, Qt::Horizontal).isValid());
  EXPECT_TRUE(model.headerData(0, Qt::Horizontal).isValid());
  EXPECT_TRUE(model.headerData(1, Qt::Horizontal).isValid());
  EXPECT_FALSE(model.headerData(6, Qt::Horizontal).isValid());

  // Section correct
  EXPECT_EQ(model.headerData(0, Qt::Horizontal).toString().toStdString(),
            "Display Name");
  EXPECT_EQ(model.headerData(1, Qt::Horizontal).toString().toStdString(), "ID");
  EXPECT_EQ(model.headerData(2, Qt::Horizontal).toString().toStdString(),
            "IP Address");
  EXPECT_EQ(model.headerData(3, Qt::Horizontal).toString().toStdString(),
            "Last Updated");
  EXPECT_EQ(model.headerData(4, Qt::Horizontal).toString().toStdString(),
            "Owner");
  EXPECT_EQ(model.headerData(5, Qt::Horizontal).toString().toStdString(),
            "Pool");
}

TEST(GgpInstanceItemModelTests, data) {
  // Setup
  GgpInstance test_instance_0{};
  test_instance_0.display_name = "displayName1";
  test_instance_0.id = "id1";
  test_instance_0.ip_address = "10.10.0.1";
  test_instance_0.last_updated =
      QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instance_0.owner = "hebecker@";
  test_instance_0.pool = "foo-gen1-anything";

  GgpInstance test_instance_1{};
  test_instance_1.display_name = "displayName2";
  test_instance_1.id = "id2";
  test_instance_1.ip_address = "10.10.0.2";
  test_instance_1.last_updated =
      QDateTime::fromString("2020-02-02T00:42:42Z", Qt::ISODate);
  test_instance_1.owner = "programmer@";
  test_instance_1.pool = "foo-gen42-anything";

  GgpInstanceItemModel model{{test_instance_0, test_instance_1}};

  // test_instance_0 cast
  const QModelIndex cell_0_0 = model.index(0, 0, {});
  ASSERT_TRUE(cell_0_0.isValid());

  const QVariant cell_0_0_user_data = cell_0_0.data(Qt::UserRole);
  ASSERT_EQ(cell_0_0_user_data.userType(), qMetaTypeId<GgpInstance>());

  const auto cell_0_0_ggp_instance = cell_0_0_user_data.value<GgpInstance>();
  EXPECT_EQ(cell_0_0_ggp_instance, test_instance_0);

  // test_instance_1 cast
  const auto cell_1_0 = model.index(1, 0, {});
  ASSERT_TRUE(cell_1_0.isValid());

  const auto cell_1_0_user_data = cell_1_0.data(Qt::UserRole);
  ASSERT_EQ(cell_1_0_user_data.userType(), qMetaTypeId<GgpInstance>());

  const auto cell_1_0_ggp_instance = cell_1_0_user_data.value<GgpInstance>();
  EXPECT_EQ(cell_1_0_ggp_instance, test_instance_1);

  // test_instance_0 details
  EXPECT_EQ(model.index(0, 0).data().toString(), test_instance_0.display_name);
  EXPECT_EQ(model.index(0, 1).data().toString(), test_instance_0.id);
  EXPECT_EQ(model.index(0, 2).data().toString(), test_instance_0.ip_address);
  EXPECT_EQ(model.index(0, 3).data().toString(),
            test_instance_0.last_updated.toString());
  EXPECT_EQ(model.index(0, 4).data().toString(), test_instance_0.owner);
  EXPECT_EQ(model.index(0, 5).data().toString(), test_instance_0.pool);

  // test_instance_1 details
  EXPECT_EQ(model.index(1, 0).data().toString(), test_instance_1.display_name);
  EXPECT_EQ(model.index(1, 1).data().toString(), test_instance_1.id);
  EXPECT_EQ(model.index(1, 2).data().toString(), test_instance_1.ip_address);
  EXPECT_EQ(model.index(1, 3).data().toString(),
            test_instance_1.last_updated.toString());
  EXPECT_EQ(model.index(1, 4).data().toString(), test_instance_1.owner);
  EXPECT_EQ(model.index(1, 5).data().toString(), test_instance_1.pool);
}

TEST(GgpInstanceItemModelTests, SetInstances) {
  int rows_added_counter = 0;
  int rows_removed_counter = 0;
  int data_changed_counter = 0;

  const auto reset_counters = [&]() {
    rows_added_counter = 0;
    rows_removed_counter = 0;
    data_changed_counter = 0;
  };

  // initialize with one instance
  QVector<GgpInstance> test_instances{GgpInstance{}};
  GgpInstanceItemModel model{test_instances};

  QObject::connect(&model, &QAbstractItemModel::rowsInserted, &model,
                   [&]() { ++rows_added_counter; });
  QObject::connect(&model, &QAbstractItemModel::rowsRemoved, &model,
                   [&]() { ++rows_removed_counter; });
  QObject::connect(&model, &QAbstractItemModel::dataChanged, &model,
                   [&]() { ++data_changed_counter; });

  // set the same instance
  model.SetInstances(test_instances);
  EXPECT_EQ(rows_added_counter, 0);
  EXPECT_EQ(rows_removed_counter, 0);
  EXPECT_EQ(data_changed_counter, 0);

  // add 1 instance
  test_instances.push_back(GgpInstance{});
  reset_counters();
  model.SetInstances(test_instances);
  EXPECT_EQ(rows_added_counter, 1);
  EXPECT_EQ(rows_removed_counter, 0);
  EXPECT_EQ(data_changed_counter, 0);

  // remove 1 instance
  test_instances.removeLast();
  reset_counters();
  model.SetInstances(test_instances);
  EXPECT_EQ(rows_added_counter, 0);
  EXPECT_EQ(rows_removed_counter, 1);
  EXPECT_EQ(data_changed_counter, 0);

  // change a property of instance
  test_instances[0].display_name = "changed name";
  reset_counters();
  model.SetInstances(test_instances);
  EXPECT_EQ(rows_added_counter, 0);
  EXPECT_EQ(rows_removed_counter, 0);
  EXPECT_EQ(data_changed_counter, 1);

  // change all properties except id
  test_instances[0].display_name = "changed again";
  test_instances[0].ip_address = "10.10.0.1";
  test_instances[0].last_updated =
      QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate);
  test_instances[0].owner = "owner@";
  test_instances[0].pool = "foo-gen1-anything";
  reset_counters();
  model.SetInstances(test_instances);
  EXPECT_EQ(rows_added_counter, 0);
  EXPECT_EQ(rows_removed_counter, 0);
  EXPECT_EQ(data_changed_counter, 1);

  // change id of instance
  test_instances[0].id = "new id";
  reset_counters();
  model.SetInstances(test_instances);
  EXPECT_EQ(rows_added_counter, 1);
  EXPECT_EQ(rows_removed_counter, 1);
  EXPECT_EQ(data_changed_counter, 0);
}