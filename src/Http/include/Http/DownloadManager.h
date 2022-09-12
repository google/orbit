// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HTTP_DOWNLOAD_MANAGER_H_
#define HTTP_DOWNLOAD_MANAGER_H_

#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"

namespace orbit_http {

class DownloadManager {
 public:
  virtual ~DownloadManager() = default;

  [[nodiscard]] virtual orbit_base::Future<
      ErrorMessageOr<orbit_base::CanceledOr<orbit_base::NotFoundOr<void>>>>
  Download(std::string url, std::filesystem::path save_file_path,
           orbit_base::StopToken stop_token) = 0;
};

}  // namespace orbit_http

#endif  // HTTP_DOWNLOAD_MANAGER_H_