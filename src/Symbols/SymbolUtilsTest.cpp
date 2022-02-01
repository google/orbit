// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "Symbols/SymbolUtils.h"

namespace orbit_symbols {

TEST(GetStandardSymbolFilenamesForModule, ElfFile) {
  orbit_grpc_protos::ModuleInfo::ObjectFileType object_file_type =
      orbit_grpc_protos::ModuleInfo::kElfFile;
  std::filesystem::path directory = std::filesystem::path{"path"} / "to" / "folder";
  {  // .so file extention
    const std::vector<std::filesystem::path> file_names =
        GetStandardSymbolFilenamesForModule(directory / "lib.so", object_file_type);
    EXPECT_THAT(file_names, testing::Contains("lib.debug"));
    EXPECT_THAT(file_names, testing::Contains("lib.so.debug"));
    EXPECT_THAT(file_names, testing::Contains("lib.so"));
  }

  {  // generic file extention (.ext)
    const std::vector<std::filesystem::path> file_names =
        GetStandardSymbolFilenamesForModule(directory / "lib.ext", object_file_type);
    EXPECT_THAT(file_names, testing::Contains("lib.debug"));
    EXPECT_THAT(file_names, testing::Contains("lib.ext.debug"));
    EXPECT_THAT(file_names, testing::Contains("lib.ext"));
  }
}

TEST(GetStandardSymbolFilenamesForModule, CoffFile) {
  orbit_grpc_protos::ModuleInfo::ObjectFileType object_file_type =
      orbit_grpc_protos::ModuleInfo::kCoffFile;
  std::filesystem::path directory = std::filesystem::path{"C:"} / "path" / "to" / "folder";
  {  // .dll file extention
    const std::vector<std::filesystem::path> file_names =
        GetStandardSymbolFilenamesForModule(directory / "lib.dll", object_file_type);
    EXPECT_THAT(file_names, testing::Contains("lib.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.dll.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.pdb"));
  }

  {  // generic file extention (.ext)
    const std::vector<std::filesystem::path> file_names =
        GetStandardSymbolFilenamesForModule(directory / "lib.ext", object_file_type);
    EXPECT_THAT(file_names, testing::Contains("lib.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.ext.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.ext"));
  }
}

}  // namespace orbit_symbols