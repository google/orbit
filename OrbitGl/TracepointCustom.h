// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TRACEPOINTCUSTOM_H
#define ORBIT_TRACEPOINTCUSTOM_H

#include "tracepoint.pb.h"

struct HashTracepointInfo {
  size_t operator()(const orbit_grpc_protos::TracepointInfo& info) const {
    return std::hash<std::string>{}(info.category()) * 37 + std::hash<std::string>{}(info.name());
  }
};

struct EqualTracepointInfo {
  bool operator()(const orbit_grpc_protos::TracepointInfo& left,
                  const orbit_grpc_protos::TracepointInfo& right) const {
    return left.category().compare(right.category()) == 0 && left.name().compare(right.name()) == 0;
  }
};

#endif  // ORBIT_TRACEPOINTCUSTOM_H