// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "TracepointEventBuffer.h"

class TracepointEventTracer {
 public:
  TracepointEventBuffer& GetTracepointEventBuffer() { return m_TracepointEventBuffer; }

 private:
  TracepointEventBuffer m_TracepointEventBuffer;
};

extern TracepointEventTracer GTracepointEventTracer;
