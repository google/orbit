// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <outcome.hpp>
#include <string>
#include <unordered_map>

#include "OrbitType.h"
#include "SerializationMacros.h"

class CaptureSerializer {
 public:
  CaptureSerializer();
  outcome::result<void, std::string> Save(const std::string& filename);
  outcome::result<void, std::string> Load(const std::string& filename);

  template <class T>
  void Save(T& archive);

  class TimeGraph* time_graph_;
  class SamplingProfiler* m_SamplingProfiler;

  std::string m_CaptureName;
  int m_Version;
  int m_TimerVersion;
  int m_NumTimers;
  int m_SizeOfTimer;

  ORBIT_SERIALIZABLE;
};
