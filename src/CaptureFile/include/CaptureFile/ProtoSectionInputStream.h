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

  // Reads next message from the stream. Note that the caller should not read past
  // orbit_grpc_protos::CaptureFinished message in the case of Capture Section.
  // This is because the capture section does not have a size and is bounded by the
  // start of the next section or the section list and start of all sections are
  // aligned to 8bytes. Reading beyond the CaptureFinished message will incorrectly
  // read padded zeros as empty messages until finally causing an end of section error.
  virtual ErrorMessageOr<void> ReadMessage(google::protobuf::Message* message) = 0;
};

}  // namespace orbit_capture_file

#endif  // CAPTURE_FILE_PROTO_SECTION_INPUT_STREAM_H_
