// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DISPLAY_FORMATS_DISPLAY_FORMATS_H_
#define DISPLAY_FORMATS_DISPLAY_FORMATS_H_

#include <absl/time/time.h>
#include <stdint.h>

#include <string>

namespace orbit_display_formats {

[[nodiscard]] std::string GetDisplaySize(uint64_t size_bytes);

enum class TimeUnit { kNanosecond, kMicrosecod, kMillisecond, kSecond, kMinute, kHour, kDay };
[[nodiscard]] std::string GetDisplayTimeUnit(TimeUnit unit);
[[nodiscard]] TimeUnit ChooseUnitForDisplayTime(absl::Duration duration);
[[nodiscard]] double ToDoubleInGivenTimeUnits(absl::Duration duration, TimeUnit unit);
[[nodiscard]] std::string GetDisplayTime(absl::Duration duration);
[[nodiscard]] std::string GetDisplayISOTimestamp(absl::Duration timestamp, int num_digits_precision,
                                                 absl::Duration total_capture_duration);
[[nodiscard]] std::string GetDisplayISOTimestamp(absl::Duration timestamp,
                                                 int num_digits_precision);

}  // namespace orbit_display_formats

#endif  // DISPLAY_FORMATS_DISPLAY_FORMATS_H_
