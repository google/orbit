// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_DESERIALIZER_H_
#define ORBIT_GL_CAPTURE_DESERIALIZER_H_

#include <iosfwd>
#include <outcome.hpp>
#include <string>

#include "CaptureData.h"
#include "OrbitBase/Result.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "capture_data.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"

namespace capture_deserializer {

ErrorMessageOr<void> Load(std::istream& stream, CaptureListener* capture_listener,
                          bool* cancelation_requested);
ErrorMessageOr<void> Load(const std::string& filename, CaptureListener* capture_listener,
                          bool* cancelation_requested);

namespace internal {

bool ReadMessage(google::protobuf::Message* message, google::protobuf::io::CodedInputStream* input);

void LoadCaptureInfo(const orbit_client_protos::CaptureInfo& capture_info,
                     CaptureListener* capture_listener,
                     google::protobuf::io::CodedInputStream* coded_input,
                     bool* cancelation_requested);

inline const std::string kRequiredCaptureVersion = "1.52";

}  // namespace internal

}  // namespace capture_deserializer

#endif  // ORBIT_GL_CAPTURE_DESERIALIZER_H_
