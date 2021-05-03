// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SECTION_INPUT_STREAM_IMPL_H_
#define CAPTURE_SECTION_INPUT_STREAM_IMPL_H_

#include "CaptureFile/CaptureSectionInputStream.h"
#include "FileFragmentInputStream.h"
#include "OrbitBase/File.h"

namespace orbit_capture_file_internal {

// This class is used to read from the main section of capture file.
class CaptureSectionInputStreamImpl : public orbit_capture_file::CaptureSectionInputStream {
 public:
  explicit CaptureSectionInputStreamImpl(orbit_base::unique_fd& fd, uint64_t capture_section_offset,
                                         uint64_t capture_section_size)
      : fd_{fd},
        file_fragment_input_stream_{fd_, capture_section_offset, capture_section_size},
        coded_input_stream_(&file_fragment_input_stream_) {}

  ErrorMessageOr<orbit_grpc_protos::ClientCaptureEvent> ReadEvent() override;

 private:
  orbit_base::unique_fd& fd_;
  FileFragmentInputStream file_fragment_input_stream_;
  google::protobuf::io::CodedInputStream coded_input_stream_;
};

}  // namespace orbit_capture_file_internal
#endif  // CAPTURE_SECTION_INPUT_STREAM_IMPL_H_
