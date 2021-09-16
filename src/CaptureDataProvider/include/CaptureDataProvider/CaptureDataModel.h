// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_DATA_PROVIDER_CAPTURE_DATA_MODEL_H_
#define CAPTURE_DATA_PROVIDER_CAPTURE_DATA_MODEL_H_

#include <stdint.h>

#include <vector>

namespace orbit_capture_data_provider {

struct EventPane final {
  uint64_t id = 0;
  uint64_t type = 0;
};

struct Track final {
  uint64_t track_id = 0;
  uint64_t label_id = 0;
  std::vector<EventPane> event_panes;
  std::vector<Track> subtracks;
};

// The following structures contain information necessary to draw timers and samples in
// ThreadTrack. This is data used on the hot-path. Everything else is hidden behind timer_id
// and sample_id.

// The label is a special case where it is used on hot-path but still needs to be requested
// separately, this is done to avoid duplication which we usually have a lot of since
// set of timers in ThreadTrack are instrumented functions. CaptureDataProvider should
// ensure that for hot timers it has labels prefetched and cashed for fast lookup.

struct Timer final {
  uint64_t start_ns;
  uint64_t duration_ns;
  // This is a label to display
  uint64_t label_id;
  // We can fetch additional information about the timer using this id
  uint64_t timer_id;
};

struct Sample {
  uint64_t timestamp_ns;
  uint64_t sample_id;
};

struct TimerBlock final {
  uint64_t start_timestamp_ns = 0;
  uint64_t end_timestamp_ns = 0;
  std::vector<Timer> timers;
};

struct SampleBlock final {
  uint64_t start_timestamp_ns = 0;
  uint64_t end_timestamp_ns = 0;
  std::vector<Sample> samples;
};

}  // namespace orbit_capture_data_provider

#endif  // CAPTURE_DATA_PROVIDER_CAPTURE_DATA_MODEL_H_
