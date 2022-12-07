// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TEST_UTILS_TEMPORARY_DIRECTORY_H_
#define ORBIT_TEST_UTILS_TEMPORARY_DIRECTORY_H_

#include <filesystem>
#include <utility>

#include "OrbitBase/Result.h"
#include "OrbitBase/UniqueResource.h"

namespace orbit_test_utils {
// Creates a temporary directory that tests can assume to be exclusive and initially empty. The
// directory and all the files inside will also be automatically deleted when an instance of
// `TemporaryDir` gets out of scope.
class TemporaryDirectory final {
 public:
  [[nodiscard]] const std::filesystem::path& GetDirectoryPath() const { return dir_.get(); }

  // Call this function to create a new temporary directory.
  static ErrorMessageOr<TemporaryDirectory> Create();

 private:
  explicit TemporaryDirectory(std::filesystem::path dir) : dir_{std::move(dir)} {}

  struct DirectoryDeleter {
    void operator()(const std::filesystem::path& dir) const;
  };
  orbit_base::unique_resource<std::filesystem::path, DirectoryDeleter> dir_;
};

}  // namespace orbit_test_utils

#endif  // ORBIT_TEST_UTILS_TEMPORARY_DIRECTORY_H_
