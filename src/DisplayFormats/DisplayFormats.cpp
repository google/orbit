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

std::string ToStringAtLeastTwoDigits(int number) {
  return std::to_string(number / 10) + std::to_string(number % 10);
}

std::string GetDisplayISOTimestamp(absl::Duration timestamp, int num_digits_precision,
                                   absl::Duration total_capture_duration) {
  std::string label;
  // Hours, minutes and seconds
  if (total_capture_duration >= absl::Hours(1)) {
    int64_t hours = absl::ToInt64Hours(timestamp);
    label += ToStringAtLeastTwoDigits(hours) + ":";
    timestamp -= absl::Hours(hours);
  }

  if (total_capture_duration >= absl::Minutes(1)) {
    int64_t minutes = absl::ToInt64Minutes(timestamp);
    label += ToStringAtLeastTwoDigits(minutes) + ":";
    timestamp -= absl::Minutes(minutes);
  }

  int64_t seconds = absl::ToInt64Seconds(timestamp);
  label += ToStringAtLeastTwoDigits(seconds);
  timestamp -= absl::Seconds(seconds);

  if (num_digits_precision == 0) {
    // Special case. If we are only showing seconds, let's add an 's';
    if (label.size() <= 2) {
      label += 's';
    }
    return label;
  }

  // Parts of a second
  label += ".";
  absl::Duration precision_level = absl::Seconds(1);
  for (int i = 0; i < num_digits_precision; i++) {
    precision_level /= 10;
    label += std::to_string(absl::IDivDuration(timestamp, precision_level, &timestamp));
  }
  return label;
}

std::string GetDisplayISOTimestamp(absl::Duration timestamp, int num_digits_precision) {
  return GetDisplayISOTimestamp(timestamp, num_digits_precision, timestamp);
}

}  // namespace orbit_display_formats
