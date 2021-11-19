// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_UPLOADER_UPLOAD_DATA_INTERFACE_H_
#define CAPTURE_UPLOADER_UPLOAD_DATA_INTERFACE_H_

#include <vector>

namespace orbit_capture_uploader {

// Defines status of data to upload.
enum class DataReadiness { kHasData, kWaitingForData, kEndOfData };

// Defines the required methods for streaming data to Scotty through a
// CaptureUploader. This interface allows CaptureUploader to access the upload
// data buffer as well as query the upload data status.
class UploadDataInterface {
 public:
  virtual ~UploadDataInterface() = default;

  // Determine status of data to upload. CaptureUploader calls this method frequently to determine
  // whether to continue, pause, resume or stop the upload.
  [[nodiscard]] virtual DataReadiness DetermineDataReadiness() = 0;

  // Read at most `max_bytes` data into the buffer pointed to by `dest` and return the actual read
  // bytes.
  [[nodiscard]] virtual size_t ReadIntoBuffer(void* dest, size_t max_bytes) = 0;
};

}  // namespace orbit_capture_uploader

#endif  // CAPTURE_UPLOADER_UPLOAD_DATA_INTERFACE_H_