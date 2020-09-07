// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_DESERIALIZER_H_
#define ORBIT_GL_CAPTURE_DESERIALIZER_H_

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>

#include <iosfwd>
#include <outcome.hpp>
#include <string>

#include "CaptureData.h"
#include "OrbitBase/Result.h"
#include "TimeGraph.h"
#include "capture_data.pb.h"

class CaptureDeserializer {
 public:
  ErrorMessageOr<void> Load(std::istream& stream);
  ErrorMessageOr<void> Load(const std::string& filename);

  TimeGraph* time_graph_;

  static bool ReadMessage(google::protobuf::Message* message,
                          google::protobuf::io::CodedInputStream* input);

 private:
  [[nodiscard]] CaptureData GenerateCaptureData(
      const orbit_client_protos::CaptureInfo& capture_info);

  orbit_client_protos::CaptureHeader header_;

  static inline const std::string kRequiredCaptureVersion = "1.52";
};

#endif  // ORBIT_GL_CAPTURE_DESERIALIZER_H_
