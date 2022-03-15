// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DisplayFormats/DisplayFormats.h"

#include <absl/strings/str_format.h>
#include <absl/time/time.h>

#include "OrbitBase/Logging.h"

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

constexpr double kHoursInOneDay = 24;

[[nodiscard]] TimeUnit ChooseUnitForDisplayTime(absl::Duration duration) {
  if (duration < absl::Microseconds(1)) {
    return TimeUnit::kNanosecond;
  }
  if (duration < absl::Milliseconds(1)) {
    return TimeUnit::kMicrosecod;
  }
  if (duration < absl::Seconds(1)) {
    return TimeUnit::kMillisecond;
  }
  if (duration < absl::Minutes(1)) {
    return TimeUnit::kSecond;
  }
  if (duration < absl::Hours(1)) {
    return TimeUnit::kMinute;
  }
  if (duration < absl::Hours(kHoursInOneDay)) {
    return TimeUnit::kHour;
  }
  return TimeUnit::kDay;
}

[[nodiscard]] std::string GetDisplayTimeUnit(TimeUnit unit) {
  switch (unit) {
    case TimeUnit::kNanosecond:
      return "ns";
    case TimeUnit::kMicrosecod:
      return "us";
    case TimeUnit::kMillisecond:
      return "ms";
    case TimeUnit::kSecond:
      return "s";
    case TimeUnit::kMinute:
      return "min";
    case TimeUnit::kHour:
      return "h";
    case TimeUnit::kDay:
      return "days";
  }
  ORBIT_UNREACHABLE();
}

[[nodiscard]] double ToDoubleInGivenTimeUnits(absl::Duration duration, TimeUnit unit) {
  switch (unit) {
    case TimeUnit::kNanosecond:
      return absl::ToDoubleNanoseconds(duration);
    case TimeUnit::kMicrosecod:
      return absl::ToDoubleMicroseconds(duration);
    case TimeUnit::kMillisecond:
      return absl::ToDoubleMilliseconds(duration);
    case TimeUnit::kSecond:
      return absl::ToDoubleSeconds(duration);
    case TimeUnit::kMinute:
      return absl::ToDoubleMinutes(duration);
    case TimeUnit::kHour:
      return absl::ToDoubleHours(duration);
    case TimeUnit::kDay:
      return absl::ToDoubleHours(duration) / kHoursInOneDay;
  }
  ORBIT_UNREACHABLE();
}

std::string GetDisplayTime(absl::Duration duration) {
  const TimeUnit unit = ChooseUnitForDisplayTime(duration);
  return absl::StrFormat("%.3f %s", ToDoubleInGivenTimeUnits(duration, unit),
                         GetDisplayTimeUnit(unit));
}

[[nodiscard]] std::string ToStringAtLeastTwoDigits(int number) {
  return number < 10 ? absl::StrFormat("0%i", number) : std::to_string(number);
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

  if (num_digits_precision != 0) {
    // Parts of a second
    label += ".";
    absl::Duration precision_level = absl::Seconds(1);
    for (int i = 0; i < num_digits_precision; ++i) {
      precision_level /= 10;
      label += std::to_string(absl::IDivDuration(timestamp, precision_level, &timestamp));
    }
  }
  return label;
}

std::string GetDisplayISOTimestamp(absl::Duration timestamp, int num_digits_precision) {
  return GetDisplayISOTimestamp(timestamp, num_digits_precision, timestamp);
}

}  // namespace orbit_display_formats
