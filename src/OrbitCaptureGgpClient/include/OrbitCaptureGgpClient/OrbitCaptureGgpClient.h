// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CAPTURE_GGP_CLIENT_ORBIT_CAPTURE_GGP_CLIENT_H_
#define ORBIT_CAPTURE_GGP_CLIENT_ORBIT_CAPTURE_GGP_CLIENT_H_

#include <absl/types/span.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class CaptureClientGgpClient {
 public:
  explicit CaptureClientGgpClient(std::string_view grpc_server_address);
  ~CaptureClientGgpClient();
  [[maybe_unused]] CaptureClientGgpClient(CaptureClientGgpClient&&);
  CaptureClientGgpClient& operator=(CaptureClientGgpClient&&);

  int StartCapture();
  int StopCapture();
  int UpdateSelectedFunctions(absl::Span<const std::string> selected_functions);
  void ShutdownService();

 private:
  class CaptureClientGgpClientImpl;

  std::unique_ptr<CaptureClientGgpClientImpl> pimpl;
};

#endif  // ORBIT_CAPTURE_GGP_CLIENT_ORBIT_CAPTURE_GGP_CLIENT_H_