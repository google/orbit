// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HTTP_MOCK_DOWNLOAD_MANAGER_H_
#define HTTP_MOCK_DOWNLOAD_MANAGER_H_

#include <gmock/gmock.h>

#include "Http/DownloadManager.h"

namespace orbit_http {
class MockDownloadManager : public DownloadManager {
 public:
  MOCK_METHOD(
      orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<orbit_base::NotFoundOr<void>>>>,
      Download, (std::string, std::filesystem::path, orbit_base::StopToken), (override));
};
}  // namespace orbit_http

#endif  // HTTP_MOCK_DOWNLOAD_MANAGER_H_