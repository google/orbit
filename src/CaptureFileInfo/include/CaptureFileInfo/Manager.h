// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_INFO_MANAGER_H_
#define CAPTURE_FILE_INFO_MANAGER_H_

#include <absl/time/time.h>

#include <filesystem>
#include <optional>
#include <vector>

#include "CaptureFileInfo/CaptureFileInfo.h"
#include "OrbitBase/Result.h"

namespace orbit_capture_file_info {

class Manager {
 public:
  explicit Manager();
  [[nodiscard]] const std::vector<CaptureFileInfo>& GetCaptureFileInfos() const {
    return capture_file_infos_;
  }

  [[nodiscard]] std::optional<absl::Duration> GetCaptureLengthByPath(
      const std::filesystem::path& path) const;

  // This function adds or touches a capture file at `path` to the list of capture files saved in
  // this class. The file is added if path is not yet contained in the list, and touches it if is.
  // Whether a file is contained in the list is determined by whether there paths are
  // lexicographically equal. (determined by operator== of std::filesystem::path
  // https://en.cppreference.com/w/cpp/filesystem/path/operator_cmp). This means that on Windows
  // paths that use slash (/) as directory separators, are equal to paths that are using backslash.
  // TODO(http://b/218298681) use std::filesystem::equivalent instead of operator== to check whether
  // 2 paths are actually pointing to the same file.
  void AddOrTouchCaptureFile(const std::filesystem::path& path,
                             std::optional<absl::Duration> capture_length);
  void Clear();
  void PurgeNonExistingFiles();
  void ProcessOutOfSyncFiles();
  ErrorMessageOr<void> FillFromDirectory(const std::filesystem::path& directory);

 protected:
  std::vector<CaptureFileInfo> capture_file_infos_;

 private:
  void SaveCaptureFileInfos();
  void LoadCaptureFileInfos();
};

}  // namespace orbit_capture_file_info

#endif  // CAPTURE_FILE_INFO_MANAGER_H_
