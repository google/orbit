// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/types/span.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <optional>

#include "SourcePathsMapping/Mapping.h"
#include "Test/Path.h"

const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();

static bool AlwaysTruePredicate(const std::filesystem::path& /*target_path*/) { return true; }

static bool AlwaysFalsePredicate(const std::filesystem::path& /*target_path*/) { return false; }

namespace orbit_source_paths_mapping {

TEST(Mapping, MapToFirstMatchingTargetSimple) {
  Mapping mapping{"/src/project", "/home/user/project"};

  const auto file_txt =
      MapToFirstMatchingTarget({mapping}, "/src/project/file.txt", &AlwaysTruePredicate);
  ASSERT_TRUE(file_txt.has_value());
  EXPECT_EQ(file_txt.value(), "/home/user/project/file.txt");

  const auto other_txt =
      MapToFirstMatchingTarget({mapping}, "/somewhere/different/other.txt", &AlwaysTruePredicate);
  ASSERT_FALSE(other_txt.has_value());
}

TEST(Mapping, MapToFirstMatchingTargetFalsePredicate) {
  Mapping mapping{"/src/project", "/home/user/project"};

  const auto no_file =
      MapToFirstMatchingTarget({mapping}, "/src/project/file.txt", &AlwaysFalsePredicate);
  ASSERT_FALSE(no_file.has_value());
}

#ifdef _WIN32
TEST(Mapping, MapToFirstMatchingTargetWindowsSeparators) {
  Mapping mapping{"C:\\UE4", "C:/Users/username/Downloads/UE4_424/UE4"};

  const auto file_txt = MapToFirstMatchingTarget(
      {mapping}, "C:\\UE4\\Engine\\Source\\Runtime\\Core\\Private\\HAL\\PThreadRunnableThread.cpp",
      &AlwaysTruePredicate);
  ASSERT_TRUE(file_txt.has_value());

  const std::filesystem::path target_path{
      "C:\\Users\\username\\Downloads\\UE4_"
      "424\\UE4\\Engine\\Source\\Runtime\\Core\\Private\\HAL\\PThreadRunnableThread.cpp"};
  EXPECT_EQ(file_txt.value(), target_path);

  // Even though we are on Windows, file paths in debug information are strings and should be
  // considered strings. If the case doesn't match, it means the mapping refers to a different
  // build.
  const auto other_txt = MapToFirstMatchingTarget(
      {mapping}, "C:\\ue4\\Engine\\Source\\Runtime\\Core\\Private\\HAL\\PThreadRunnableThread.cpp",
      &AlwaysTruePredicate);
  ASSERT_FALSE(other_txt.has_value());
}
#endif

TEST(Mapping, MapToFirstMatchingTargetMultiple) {
  Mapping mapping0{"/build/project", "/home/user/project0"};
  Mapping mapping1{"/src/project2", "/home/user/project1"};
  Mapping mapping2{"/src/project", "/home/user/project2"};

  const auto file_txt = MapToFirstMatchingTarget({mapping0, mapping1, mapping2},
                                                 "/src/project/file.txt", &AlwaysTruePredicate);
  ASSERT_TRUE(file_txt.has_value());
  EXPECT_EQ(file_txt.value(), "/home/user/project2/file.txt");

  const auto other_txt = MapToFirstMatchingTarget(
      {mapping0, mapping1, mapping2}, "/somewhere/different/other.txt", &AlwaysTruePredicate);
  ASSERT_FALSE(other_txt.has_value());
}

TEST(Mapping, MapToFirstMatchingTargetEmpty) {
  const auto file_txt = MapToFirstMatchingTarget({}, "/src/project/file.txt", &AlwaysTruePredicate);
  ASSERT_FALSE(file_txt.has_value());

  const auto other_txt =
      MapToFirstMatchingTarget({}, "/somewhere/different/other.txt", &AlwaysTruePredicate);
  ASSERT_FALSE(other_txt.has_value());
}

TEST(Mapping, MapToFirstExistingTargetSimple) {
  Mapping mapping{"/src/project", testdata_directory};

  const auto file_txt = MapToFirstExistingTarget({mapping}, "/src/project/plain.txt");
  ASSERT_TRUE(file_txt.has_value());
  EXPECT_EQ(file_txt.value(), testdata_directory / "plain.txt");

  const auto other_txt = MapToFirstExistingTarget({mapping}, "/src/project/other.txt");
  ASSERT_FALSE(other_txt.has_value());
}

TEST(Mapping, MapToFirstExistingTargetMultiple) {
  Mapping mapping0{"/build/project", "/home/user/project"};
  Mapping mapping1{"/src/project2", "/home/user/project"};
  Mapping mapping2{"/src/project", testdata_directory};

  const auto file_txt =
      MapToFirstExistingTarget({mapping0, mapping1, mapping2}, "/src/project/plain.txt");
  ASSERT_TRUE(file_txt.has_value());
  EXPECT_EQ(file_txt.value(), testdata_directory / "plain.txt");

  const auto other_txt =
      MapToFirstExistingTarget({mapping0, mapping1, mapping2}, "/build/project/other.txt");
  ASSERT_FALSE(other_txt.has_value());
}

TEST(Mapping, InferMappingFromExampleSimple) {
  const std::filesystem::path source_path = "/build/libc/glibc.c";
  const std::filesystem::path target_path = "C:/src/sysroot/usr/src/libc/glibc.c";

  auto maybe_mapping = InferMappingFromExample(source_path, target_path);
  ASSERT_TRUE(maybe_mapping);
  EXPECT_EQ(maybe_mapping.value().source_path, "/build");
  EXPECT_EQ(maybe_mapping.value().target_path, "C:/src/sysroot/usr/src");
}

TEST(Mapping, InferMappingFromExampleMismatchingFilename) {
  const std::filesystem::path source_path = "/build/libc/glibc.c";
  const std::filesystem::path target_path = "C:/src/sysroot/usr/src/libc/glibc.cpp";

  auto maybe_mapping = InferMappingFromExample(source_path, target_path);
  ASSERT_FALSE(maybe_mapping);
}

TEST(Mapping, InferMappingFromExampleIdentity) {
  const std::filesystem::path source_path = "C:/build/libc/glibc.c";
  const std::filesystem::path target_path = "C:/build/libc/glibc.c";

  auto maybe_mapping = InferMappingFromExample(source_path, target_path);
  ASSERT_FALSE(maybe_mapping);
}

#ifdef _WIN32
TEST(Mapping, InferMappingFromExampleWindows) {
  const std::filesystem::path source_path =
      "C:\\UE4\\Engine\\Source\\Runtime\\Core\\Private\\HAL\\PThreadRunnableThread.cpp";
  const std::filesystem::path target_path =
      "C:/Users/user/Downloads/UE4_424/UE4/Engine/Source/Runtime/Core/Private/HAL/"
      "PThreadRunnableThread.cpp";

  auto maybe_mapping = InferMappingFromExample(source_path, target_path);
  ASSERT_TRUE(maybe_mapping);
  EXPECT_EQ(maybe_mapping.value().source_path, "C:\\");
  EXPECT_EQ(maybe_mapping.value().target_path, "C:/Users/user/Downloads/UE4_424");
}
#endif

}  // namespace orbit_source_paths_mapping
