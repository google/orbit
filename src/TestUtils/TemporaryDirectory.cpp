// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TestUtils/TemporaryDirectory.h"

#include <absl/strings/str_format.h>
#include <absl/synchronization/mutex.h>
#include <stdint.h>

#include <filesystem>
#include <limits>
#include <random>
#include <string>
#include <system_error>
#include <tuple>

#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"

namespace orbit_test_utils {

[[nodiscard]] static uint32_t Get4BytesOfRandomness() {
  // Since we keep the mersenne twister engine in static storage, we have to ensure thread safety.
  static absl::Mutex mutex{};
  absl::MutexLock lock{&mutex};

  static std::random_device random_device{};
  static std::mt19937 gen{random_device()};
  static std::uniform_int_distribution<uint32_t> distribution{std::numeric_limits<uint32_t>::min(),
                                                              std::numeric_limits<uint32_t>::max()};
  return distribution(gen);
}

ErrorMessageOr<TemporaryDirectory> TemporaryDirectory::Create() {
  std::error_code error_code{};
  std::filesystem::path tmp_dir = std::filesystem::temp_directory_path(error_code);
  if (error_code) return ErrorMessage{error_code};

  constexpr int kTries = 10;
  for (int i = 0; i < kTries; ++i) {
    std::filesystem::path unique_path =
        tmp_dir / absl::StrFormat("orbit_%08X", Get4BytesOfRandomness());
    OUTCOME_TRY(bool exists, orbit_base::FileOrDirectoryExists(unique_path));
    if (exists) continue;

    OUTCOME_TRY(orbit_base::CreateDirectories(unique_path));
    return TemporaryDirectory{std::move(unique_path)};
  }

  return ErrorMessage{"Failed to create a temporary directory."};
}

void TemporaryDirectory::DirectoryDeleter::operator()(const std::filesystem::path& dir) const {
  std::error_code error{};
  std::filesystem::remove_all(dir, error);
  std::ignore = error;  // Since this is running in a destructor we can't handle the error here.
}

}  // namespace orbit_test_utils
