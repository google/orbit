// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <filesystem>
#include <optional>
#include <vector>

#include "SourcePathsMapping/Mapping.h"
#include "SourcePathsMapping/MappingManager.h"

namespace orbit_source_paths_mapping {

const Mapping mapping0{"/build/project", "/home/user/project"};
const Mapping mapping1{"/src/project2", "/home/user/project"};
const Mapping mapping2{"/src/project", "/home/user/project"};

constexpr const char* kOrgName = "The Orbit Authors";

TEST(MappingManager, SetAndGet) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("MappingManager.SetAndGet");

  MappingManager manager{};

  std::vector<Mapping> mappings{mapping0, mapping1, mapping2};
  manager.SetMappings(mappings);
  EXPECT_EQ(manager.GetMappings(), mappings);
}

TEST(MappingManager, Append) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("MappingManager.Append");

  MappingManager manager{};

  std::vector<Mapping> mappings{mapping0, mapping1};
  manager.SetMappings(mappings);

  manager.AppendMapping(mapping2);
  mappings.emplace_back(mapping2);

  EXPECT_EQ(manager.GetMappings(), mappings);
}

TEST(MappingManager, SaveLoadAndClear) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("MappingManager.SaveAndLoad");

  std::vector<Mapping> mappings{mapping0, mapping1, mapping2};
  {
    MappingManager manager{};
    manager.SetMappings(mappings);
  }

  {
    MappingManager manager{};
    EXPECT_EQ(manager.GetMappings(), mappings);
    manager.SetMappings({});
  }

  {
    MappingManager manager{};
    EXPECT_TRUE(manager.GetMappings().empty());
  }
}

TEST(MappingManager, InferAndAppendSourcePathsMapping) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("MappingManager.InferAndAppendSourcePathsMapping");

  {
    MappingManager manager{};
    manager.SetMappings({});
  }

  const std::filesystem::path source_path = "/build/libc/glibc.c";
  const std::filesystem::path target_path = "C:/src/sysroot/usr/src/libc/glibc.c";
  InferAndAppendSourcePathsMapping(source_path, target_path);

  {
    const std::optional<Mapping> reference_mapping =
        InferMappingFromExample(source_path, target_path);
    ASSERT_TRUE(reference_mapping.has_value());

    MappingManager manager{};
    ASSERT_EQ(manager.GetMappings().size(), 1);
    EXPECT_EQ(manager.GetMappings().front(), reference_mapping.value());
  }
}
}  // namespace orbit_source_paths_mapping