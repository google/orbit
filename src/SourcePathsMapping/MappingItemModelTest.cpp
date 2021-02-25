// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QAbstractItemModelTester>

#include "QtUtils/AssertNoQtLogWarnings.h"
#include "SourcePathsMapping/Mapping.h"
#include "SourcePathsMapping/MappingItemModel.h"

namespace orbit_source_paths_mapping {

const Mapping mapping0{"/build/project", "/home/user/project"};
const Mapping mapping1{"/src/project2", "/home/user/project"};
const Mapping mapping2{"/src/project", "/home/user/project"};

TEST(MappingItemModel, EmptyModel) {
  orbit_qt_utils::AssertNoQtLogWarnings message_handler{};

  MappingItemModel model{};

  QAbstractItemModelTester{&model, QAbstractItemModelTester::FailureReportingMode::Warning};
}

TEST(MappingItemModel, FilledModel) {
  orbit_qt_utils::AssertNoQtLogWarnings message_handler{};

  MappingItemModel model{};
  model.SetMappings({mapping0, mapping1, mapping2});

  QAbstractItemModelTester{&model, QAbstractItemModelTester::FailureReportingMode::Warning};
}

}  // namespace orbit_source_paths_mapping