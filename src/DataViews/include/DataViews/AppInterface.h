// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_APP_INTERFACE_H_
#define DATA_VIEWS_APP_INTERFACE_H_

#include <absl/types/span.h>
#include <stdint.h>

#include <string>

#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "ClientModel/CaptureData.h"
#include "OrbitBase/Future.h"
#include "capture_data.pb.h"

namespace orbit_data_views {

class AppInterface {
 public:
  virtual ~AppInterface() noexcept = default;

  virtual void SetClipboard(const std::string& contents) = 0;
  [[nodiscard]] virtual std::string GetSaveFile(const std::string& extension) const = 0;

  virtual void SendErrorToUi(const std::string& title, const std::string& text) = 0;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_APP_INTERFACE_H_
