// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_SERIALIZER_H_
#define ORBIT_GL_CAPTURE_SERIALIZER_H_

#include <iosfwd>
#include <outcome.hpp>
#include <string>

#include "OrbitBase/Result.h"
#include "capture.pb.h"

class CaptureSerializer {
 public:
  void Save(std::ostream& stream);
  ErrorMessageOr<void> Save(const std::string& filename);

  ErrorMessageOr<void> Load(std::istream& stream);
  ErrorMessageOr<void> Load(const std::string& filename);

  class TimeGraph* time_graph_;

 private:
  template <class T>
  void SaveImpl(T& archive);

  CaptureHeader header;

  static const uint32_t CAPTURE_VERSION = 1;
  static const uint32_t MIN_CAPTURE_VERSION = 1;
};

#endif  // ORBIT_GL_CAPTURE_SERIALIZER_H_
