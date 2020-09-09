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
#include "TimeGraph.h"
#include "capture_data.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"

namespace capture_deserializer {

ErrorMessageOr<void> Load(std::istream& stream, TimeGraph* time_graph);
ErrorMessageOr<void> Load(const std::string& filename, TimeGraph* time_graph);

namespace internal {

bool ReadMessage(google::protobuf::Message* message, google::protobuf::io::CodedInputStream* input);

[[nodiscard]] CaptureData GenerateCaptureData(const orbit_client_protos::CaptureInfo& capture_info,
                                              StringManager* string_manager);

inline const std::string kRequiredCaptureVersion = "1.52";

}  // namespace internal

}  // namespace capture_deserializer

#endif  // ORBIT_GL_CAPTURE_DESERIALIZER_H_
