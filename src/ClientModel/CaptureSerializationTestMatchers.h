// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_MODEL_CAPTURE_SERIALIZATION_TEST_MATCHERS_H_
#define CLIENT_MODEL_CAPTURE_SERIALIZATION_TEST_MATCHERS_H_

#include <gmock/gmock-generated-matchers.h>

#include "capture_data.pb.h"

namespace orbit_client_model {

MATCHER(ThreadStateSliceInfoEq, "") {
  const orbit_client_protos::ThreadStateSliceInfo& a = std::get<0>(arg);
  const orbit_client_protos::ThreadStateSliceInfo& b = std::get<1>(arg);
  return a.tid() == b.tid() && a.thread_state() == b.thread_state() &&
         a.begin_timestamp_ns() == b.begin_timestamp_ns() &&
         a.end_timestamp_ns() == b.end_timestamp_ns();
}

MATCHER_P(ThreadStateSliceInfoEq, that, "") {
  const orbit_client_protos::ThreadStateSliceInfo& a = arg;
  const orbit_client_protos::ThreadStateSliceInfo& b = that;
  return a.tid() == b.tid() && a.thread_state() == b.thread_state() &&
         a.begin_timestamp_ns() == b.begin_timestamp_ns() &&
         a.end_timestamp_ns() == b.end_timestamp_ns();
}

}  // namespace orbit_client_model

#endif  // CLIENT_MODEL_CAPTURE_SERIALIZATION_TEST_MATCHERS_H_
