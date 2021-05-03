// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureSectionInputStreamImpl.h"

#include "OrbitBase/MakeUniqueForOverwrite.h"

namespace orbit_capture_file_internal {
using orbit_grpc_protos::ClientCaptureEvent;

ErrorMessageOr<ClientCaptureEvent> CaptureSectionInputStreamImpl::ReadEvent() {
  uint64_t message_size = 0;

  // Note that in case there was an error CodedInputStream does not provide error messages/codes.
  // We need to go to underlying stream (file_fragment_input_stream_ in this case) to get the error
  // message in case of a failure.
  if (!coded_input_stream_.ReadVarint64(&message_size)) {
    return file_fragment_input_stream_.GetLastError().value_or(
        ErrorMessage{"Unexpected end of section while reading message size"});
  }

  auto buf = make_unique_for_overwrite<uint8_t[]>(message_size);
  if (!coded_input_stream_.ReadRaw(buf.get(), message_size)) {
    return file_fragment_input_stream_.GetLastError().value_or(
        ErrorMessage{"Unexpected end of section while reading the message"});
  }

  ClientCaptureEvent result;
  result.ParseFromArray(buf.get(), message_size);
  return result;
}

}  // namespace orbit_capture_file_internal