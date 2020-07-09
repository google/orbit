// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <iosfwd>
#include <outcome.hpp>
#include <string>

#include "OrbitBase/Result.h"
#include "SerializationMacros.h"

class CaptureSerializer {
 public:
  void Save(std::ostream& stream);
  Result<void, ErrorMessage> Save(const std::string& filename);

  Result<void, ErrorMessage> Load(std::istream& stream);
  Result<void, ErrorMessage> Load(const std::string& filename);

  class TimeGraph* time_graph_;

  std::string m_CaptureName;
  int m_NumTimers;

  ORBIT_SERIALIZABLE;

 private:
  template <class T>
  void SaveImpl(T& archive);
};
