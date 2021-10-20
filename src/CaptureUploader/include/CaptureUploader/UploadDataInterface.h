// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_UPLOADER_UPLOAD_DATA_INTERFACE_H_
#define CAPTURE_UPLOADER_UPLOAD_DATA_INTERFACE_H_

#include <vector>

namespace orbit_capture_uploader {

// Defines status of data to upload.
enum class DataReadiness { HAS_DATA, WAITING_FOR_DATA, END_OF_DATA };

// Defines the required methods for streaming data to Scotty through a
// CaptureUploader. This interface allows CaptureUploader to access the upload
// data buffer as well as query the upload data status.
class UploadDataInterface {
 public:
  virtual ~UploadDataInterface() = default;

  // Get status of data to upload. CaptureUploader calls this method frequently
  // to determine whether to continue, pause, resume or stop the upload.
  virtual DataReadiness GetDataReadiness() const = 0;

  // Get reference of the upload data buffer which stores data in raw bytes.
  // CaptureUploader calls this method to get the upload data buffer where it
  // will read data from while uploading.
  virtual const std::vector<unsigned char>& GetUploadDataBuffer() const = 0;

  // Clear the upload data buffer and refresh it with newly produced data.
  // CaptureUploader calls this method when it finishes uploading existing data
  // in the upload data buffer.
  virtual void RefreshUploadDataBuffer() = 0;
};

}  // namespace orbit_capture_uploader

#endif  // CAPTURE_UPLOADER_UPLOAD_DATA_INTERFACE_H_