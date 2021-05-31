// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_INFO_MANAGER_H_
#define CAPTURE_FILE_INFO_MANAGER_H_

#include <vector>

#include "CaptureFileInfo/CaptureFileInfo.h"

namespace orbit_capture_file_info {

class Manager {
 public:
  explicit Manager();
  [[nodiscard]] const std::vector<CaptureFileInfo>& GetCaptureFileInfos() const {
    return capture_file_infos_;
  }

  void AddOrTouchCaptureFile(const QString& path);
  void Clear();
  void PurgeNonExistingFiles();

 private:
  void SaveCaptureFileInfos();
  void LoadCaptureFileInfos();

  std::vector<CaptureFileInfo> capture_file_infos_;
};

}  // namespace orbit_capture_file_info

#endif  // CAPTURE_FILE_INFO_MANAGER_H_
