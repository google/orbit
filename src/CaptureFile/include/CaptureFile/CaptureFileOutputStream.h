// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_CAPTURE_FILE_OUTPUT_STREAM_H_
#define CAPTURE_FILE_CAPTURE_FILE_OUTPUT_STREAM_H_

#include <google/protobuf/message.h>

#include <filesystem>
#include <memory>

#include "CaptureFile/BufferOutputStream.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_capture_file {

// This class in used for creating new capture file from
// a stream of ClientCaptureEvents. If the file already exists
// it is going to be overwritten. Appending to the existing file
// is not supported.
//
// Usage example:
//
// auto output_stream_or_error = CaptureOutputStream::Create("path/to/file.capture");
// if (!output_stream_or_error.has_value()) {
//   // Handle the error
// }
// auto& output_stream = output_stream_or_error.value();
// for (...) {
//   auto result = output_stream->WriteCaptureEvent(event);
//   if (result.has_error()) {
//     // This is an unrecoverable error: the stream is closed and
//     // the file is deleted.
//     // Handle/Report the error.
//     break;
//   }
// }
//
// output_stream->Close();
//
// Note: the stream will be closed on destruction if it was not explicitly closed before that.
// Note: Write after close or error will result in CHECK failure.
class CaptureFileOutputStream {
 public:
  virtual ~CaptureFileOutputStream() = default;
  [[nodiscard]] virtual ErrorMessageOr<void> WriteCaptureEvent(
      const orbit_grpc_protos::ClientCaptureEvent& event) = 0;
  virtual ErrorMessageOr<void> Close() = 0;

  [[nodiscard]] virtual bool IsOpen() = 0;

  // Create new capture file output stream. If the file exists it is going to be
  // overwritten.
  [[nodiscard]] static ErrorMessageOr<std::unique_ptr<CaptureFileOutputStream>> Create(
      std::filesystem::path path);
  [[nodiscard]] static std::unique_ptr<CaptureFileOutputStream> Create(
      BufferOutputStream* output_buffer);
};

}  // namespace orbit_capture_file

#endif  // CAPTURE_FILE_CAPTURE_FILE_OUTPUT_STREAM_H_
