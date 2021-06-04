// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_PROTO_SECTION_INPUT_STREAM_H_
#define CAPTURE_FILE_PROTO_SECTION_INPUT_STREAM_H_

#include <google/protobuf/message.h>

#include "OrbitBase/Result.h"

namespace orbit_capture_file {

// This class is used to read from the main section of capture file.
// It is created by CaptureFile::CreateCaptureSectionInputStream() method.
class ProtoSectionInputStream {
 public:
  ProtoSectionInputStream() = default;
  virtual ~ProtoSectionInputStream() = default;

  // Reads next event from the stream to message. Note that the caller must not read past
  // orbit_grpc_protos::CaptureFinished message in the case of Capture Section. Doing so
  // will result in undefined behavior.
  //
  // This is because the capture section does not have size and is bounded by the
  // start of the next section and start of all sections are aligned to 8bytes.
  // This means reading after CaptureFinished message sometimes end up reading
  // padded zeros which yield an empty message, or it could generate end of section
  // error.
  virtual ErrorMessageOr<void> ReadEvent(google::protobuf::Message* message) = 0;
};

}  // namespace orbit_capture_file

#endif  // CAPTURE_FILE_PROTO_SECTION_INPUT_STREAM_H_
