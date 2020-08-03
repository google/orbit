// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_SERIALIZER_H_
#define ORBIT_GL_CAPTURE_SERIALIZER_H_

#include <iosfwd>
#include <outcome.hpp>
#include <string>

#include "OrbitBase/Result.h"
#include "capture_data.pb.h"

class CaptureSerializer {
 public:
  void Save(std::ostream& stream);
  ErrorMessageOr<void> Save(const std::string& filename);

  ErrorMessageOr<void> Load(std::istream& stream);
  ErrorMessageOr<void> Load(const std::string& filename);

  class TimeGraph* time_graph_;

 private:
  void FillCaptureData(orbit_client_protos::CaptureInfo* capture_info);
  void ProcessCaptureData(const orbit_client_protos::CaptureInfo& capture_info);

  orbit_client_protos::CaptureHeader header;

  const std::string kRequiredCaptureVersion = "1.51";
};

#endif  // ORBIT_GL_CAPTURE_SERIALIZER_H_
