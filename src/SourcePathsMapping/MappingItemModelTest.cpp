// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QAbstractItemModelTester>
#include <QModelIndex>
#include <QVariant>
#include <Qt>
#include <filesystem>
#include <vector>

#include "QtUtils/AssertNoQtLogWarnings.h"
#include "SourcePathsMapping/Mapping.h"
#include "SourcePathsMapping/MappingItemModel.h"

namespace orbit_source_paths_mapping {

const Mapping mapping0{"/build/project", "/home/user/project"};
const Mapping mapping1{"/src/project2", "/home/user/project"};
const Mapping mapping2{"/src/project", "/home/user/project"};
const std::vector<Mapping> mappings{mapping0, mapping1, mapping2};

TEST(MappingItemModel, EmptyModel) {
  orbit_qt_utils::AssertNoQtLogWarnings message_handler{};

  MappingItemModel model{};

  QAbstractItemModelTester{&model, QAbstractItemModelTester::FailureReportingMode::Warning};
}

TEST(MappingItemModel, FilledModel) {
  orbit_qt_utils::AssertNoQtLogWarnings message_handler{};

  MappingItemModel model{};
  model.SetMappings(mappings);

  QAbstractItemModelTester{&model, QAbstractItemModelTester::FailureReportingMode::Warning};
}

TEST(MappingItemModel, SetMappings) {
  MappingItemModel model{};
  EXPECT_EQ(model.rowCount(), 0);

  model.SetMappings(mappings);
  EXPECT_EQ(model.rowCount(), 3);

  model.SetMappings({});
  EXPECT_EQ(model.rowCount(), 0);

  model.SetMappings(mappings);
  EXPECT_EQ(model.rowCount(), 3);
}

TEST(MappingItemModel, GetMappings) {
  MappingItemModel model{};

  model.SetMappings(mappings);
  EXPECT_THAT(model.GetMappings(), testing::ElementsAreArray(mappings));

  model.SetMappings({});
  EXPECT_THAT(model.GetMappings(), testing::IsEmpty());
}

TEST(MappingItemModel, RemoveFirstRow) {
  MappingItemModel model{};
  model.SetMappings(mappings);

  EXPECT_TRUE(model.RemoveRows(0, 1));
  EXPECT_EQ(model.rowCount(), 2);
  ASSERT_TRUE(model.index(0, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(0, 0).data(Qt::UserRole).value<const Mapping*>(), mapping1);

  ASSERT_TRUE(model.index(1, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(1, 0).data(Qt::UserRole).value<const Mapping*>(), mapping2);
}

TEST(MappingItemModel, RemoveMidRow) {
  MappingItemModel model{};
  model.SetMappings(mappings);

  EXPECT_TRUE(model.RemoveRows(1, 1));
  EXPECT_EQ(model.rowCount(), 2);
  ASSERT_TRUE(model.index(0, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(0, 0).data(Qt::UserRole).value<const Mapping*>(), mapping0);

  ASSERT_TRUE(model.index(1, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(1, 0).data(Qt::UserRole).value<const Mapping*>(), mapping2);
}

TEST(MappingItemModel, RemoveLastRow) {
  MappingItemModel model{};
  model.SetMappings(mappings);

  EXPECT_TRUE(model.RemoveRows(2, 1));
  EXPECT_EQ(model.rowCount(), 2);
  ASSERT_TRUE(model.index(0, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(0, 0).data(Qt::UserRole).value<const Mapping*>(), mapping0);

  ASSERT_TRUE(model.index(1, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(1, 0).data(Qt::UserRole).value<const Mapping*>(), mapping1);
}

TEST(MappingItemModel, RemoveMultipleRows) {
  MappingItemModel model{};
  model.SetMappings(mappings);

  EXPECT_TRUE(model.RemoveRows(0, 2));
  EXPECT_EQ(model.rowCount(), 1);

  ASSERT_TRUE(model.index(0, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(0, 0).data(Qt::UserRole).value<const Mapping*>(), mapping2);
}

TEST(MappingItemModel, RemoveAllRows) {
  MappingItemModel model{};
  model.SetMappings(mappings);

  EXPECT_TRUE(model.RemoveRows(0, 3));
  EXPECT_EQ(model.rowCount(), 0);
}

TEST(MappingItemModel, setData) {
  MappingItemModel model{};
  model.SetMappings(mappings);

  Mapping other_mapping = mapping2;
  other_mapping.target_path = std::filesystem::path{"/home/other/path"};
  EXPECT_TRUE(model.setData(model.index(2, 0), QVariant::fromValue(other_mapping)));

  ASSERT_TRUE(model.index(2, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(2, 0).data(Qt::UserRole).value<const Mapping*>(), other_mapping);
}

TEST(MappingItemModel, moveRowsDown) {
  MappingItemModel model{};
  model.SetMappings(mappings);

  // Move first row into last position
  EXPECT_TRUE(model.moveRows({}, 0, 1, {}, 3));
  EXPECT_EQ(model.rowCount(), 3);

  ASSERT_TRUE(model.index(0, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(0, 0).data(Qt::UserRole).value<const Mapping*>(), mapping1);

  ASSERT_TRUE(model.index(1, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(1, 0).data(Qt::UserRole).value<const Mapping*>(), mapping2);

  ASSERT_TRUE(model.index(2, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(2, 0).data(Qt::UserRole).value<const Mapping*>(), mapping0);
}

TEST(MappingItemModel, moveRowsUp) {
  MappingItemModel model{};
  model.SetMappings(mappings);

  // Move last row into first position
  EXPECT_TRUE(model.moveRows({}, 2, 1, {}, 0));
  EXPECT_EQ(model.rowCount(), 3);

  ASSERT_TRUE(model.index(0, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(0, 0).data(Qt::UserRole).value<const Mapping*>(), mapping2);

  ASSERT_TRUE(model.index(1, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(1, 0).data(Qt::UserRole).value<const Mapping*>(), mapping0);

  ASSERT_TRUE(model.index(2, 0).data(Qt::UserRole).canConvert<const Mapping*>());
  EXPECT_EQ(*model.index(2, 0).data(Qt::UserRole).value<const Mapping*>(), mapping1);
}

TEST(MappingItemModel, AppendNewEmptyMapping) {
  MappingItemModel model{};
  EXPECT_EQ(model.rowCount(), 0);

  model.AppendNewEmptyMapping();
  EXPECT_EQ(model.rowCount(), 1);

  model.AppendNewEmptyMapping();
  EXPECT_EQ(model.rowCount(), 2);
}
}  // namespace orbit_source_paths_mapping