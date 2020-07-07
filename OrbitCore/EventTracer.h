// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "EventBuffer.h"

class EventTracer {
 public:
  EventBuffer& GetEventBuffer() { return m_EventBuffer; }

 private:
  EventBuffer m_EventBuffer;
};

extern EventTracer GEventTracer;
