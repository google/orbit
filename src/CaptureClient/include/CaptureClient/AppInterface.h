// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_APP_INTERFACE_H_
#define CAPTURE_CLIENT_APP_INTERFACE_H_

#include "CaptureClient.h"

namespace orbit_capture_client {

class CaptureControlInterface {
 public:
  virtual ~CaptureControlInterface() = default;

  [[nodiscard]] virtual CaptureClient::State GetCaptureState() const = 0;
  [[nodiscard]] virtual bool IsCapturing() const = 0;

  virtual void StartCapture() = 0;
  virtual void StopCapture() = 0;
  virtual void AbortCapture() = 0;
  virtual void ToggleCapture() = 0;
};
}  // namespace orbit_capture_client

#endif
