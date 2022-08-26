// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTE_SYMBOL_PROVIDER_DOWNLOAD_MANAGER_INTERFACE_H_
#define REMOTE_SYMBOL_PROVIDER_DOWNLOAD_MANAGER_INTERFACE_H_

#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"

namespace orbit_remote_symbol_provider {

class DownloadManagerInterface {
 public:
  virtual ~DownloadManagerInterface() = default;

  [[nodiscard]] virtual orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>> Download(
      std::string url, std::filesystem::path save_file_path, orbit_base::StopToken stop_token) = 0;
};

}  // namespace orbit_remote_symbol_provider

#endif  // REMOTE_SYMBOL_PROVIDER_DOWNLOAD_MANAGER_INTERFACE_H_