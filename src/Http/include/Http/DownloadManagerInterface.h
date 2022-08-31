// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HTTP__DOWNLOAD_MANAGER_INTERFACE_H_
#define HTTP__DOWNLOAD_MANAGER_INTERFACE_H_

#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"

namespace orbit_http {

class DownloadManagerInterface {
 public:
  virtual ~DownloadManagerInterface() = default;

  [[nodiscard]] virtual orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>> Download(
      std::string url, std::filesystem::path save_file_path, orbit_base::StopToken stop_token) = 0;
};

}  // namespace orbit_http

#endif  // HTTP_DOWNLOAD_MANAGER_INTERFACE_H_