// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LogInterface.h"

#include "Log.h"

std::vector<std::string> LogInterface::GetOutput() {
  return GLogger.ConsumeEntries(OrbitLog::Viz);
}