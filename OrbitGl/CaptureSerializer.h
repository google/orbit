// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_SERIALIZER_H_
#define ORBIT_GL_CAPTURE_SERIALIZER_H_

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/message.h>

#include <iosfwd>
#include <outcome.hpp>
#include <string>

#include "CaptureData.h"
#include "OrbitBase/Result.h"
#include "capture_data.pb.h"

class CaptureSerializer {
 public:
  void Save(std::ostream& stream, const CaptureData& capture_data);
  ErrorMessageOr<void> Save(const std::string& filename, const CaptureData& capture_data);

  ErrorMessageOr<void> Load(std::istream& stream);
  ErrorMessageOr<void> Load(const std::string& filename);

  class TimeGraph* time_graph_;

  static bool ReadMessage(google::protobuf::Message* message,
                          google::protobuf::io::CodedInputStream* input);
  static void WriteMessage(const google::protobuf::Message* message,
                           google::protobuf::io::CodedOutputStream* output);

 private:
  orbit_client_protos::CaptureInfo GenerateCaptureInfo(const CaptureData& capture_data);
  [[nodiscard]] CaptureData GenerateCaptureData(
      const orbit_client_protos::CaptureInfo& capture_info);

  orbit_client_protos::CaptureHeader header;

  const std::string kRequiredCaptureVersion = "1.52";
};

#endif  // ORBIT_GL_CAPTURE_SERIALIZER_H_
