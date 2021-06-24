// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DisplayFormats/DisplayFormats.h"

#include <absl/strings/str_format.h>
#include <absl/time/time.h>

namespace orbit_display_formats {

std::string GetDisplaySize(uint64_t size_bytes) {
  constexpr double kBytesInKb = 1024.0;
  constexpr double kBytesInMb = 1024.0 * kBytesInKb;
  constexpr double kBytesInGb = 1024.0 * kBytesInMb;
  constexpr double kBytesInTb = 1024.0 * kBytesInGb;

  if (size_bytes < kBytesInKb) return absl::StrFormat("%u B", size_bytes);
  if (size_bytes < kBytesInMb) return absl::StrFormat("%.2f KB", size_bytes / kBytesInKb);
  if (size_bytes < kBytesInGb) return absl::StrFormat("%.2f MB", size_bytes / kBytesInMb);
  if (size_bytes < kBytesInTb) return absl::StrFormat("%.2f GB", size_bytes / kBytesInGb);
  return absl::StrFormat("%.2f TB", size_bytes / kBytesInTb);
}

std::string GetDisplayTime(absl::Duration duration) {
  if (duration < absl::Microseconds(1)) {
    return absl::StrFormat("%.3f ns", absl::ToDoubleNanoseconds(duration));
  }
  if (duration < absl::Milliseconds(1)) {
    return absl::StrFormat("%.3f us", absl::ToDoubleMicroseconds(duration));
  }
  if (duration < absl::Seconds(1)) {
    return absl::StrFormat("%.3f ms", absl::ToDoubleMilliseconds(duration));
  }
  if (duration < absl::Minutes(1)) {
    return absl::StrFormat("%.3f s", absl::ToDoubleSeconds(duration));
  }
  if (duration < absl::Hours(1)) {
    return absl::StrFormat("%.3f min", absl::ToDoubleMinutes(duration));
  }
  constexpr double kHoursInOneDay = 24;
  if (duration < absl::Hours(kHoursInOneDay)) {
    return absl::StrFormat("%.3f h", absl::ToDoubleHours(duration));
  }
  return absl::StrFormat("%.3f days", absl::ToDoubleHours(duration) / kHoursInOneDay);
}

}  // namespace orbit_display_formats
