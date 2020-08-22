// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TRACEPOINTDATA_H
#define ORBIT_TRACEPOINTDATA_H

#include "tracepoint.pb.h"

class TracepointData final {
 public:
  explicit TracepointData(orbit_grpc_protos::TracepointInfo info)
      : Tracepoint_info_(std::move(info)) {}

  void SetTracepointInfo(const orbit_grpc_protos::TracepointInfo& info) { Tracepoint_info_ = info; }

  const std::string& name() const { return Tracepoint_info_.name(); }
  const std::string& category() const { return Tracepoint_info_.category(); }

 private:
  orbit_grpc_protos::TracepointInfo Tracepoint_info_;

};

#endif  // ORBIT_TRACEPOINTDATA_H
