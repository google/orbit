// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CORE_UTILS_H_
#define ORBIT_CORE_CORE_UTILS_H_

#include <absl/strings/str_format.h>
#include <absl/time/time.h>

inline std::string GetPrettySize(uint64_t size) {
  constexpr double KB = 1024.0;
  constexpr double MB = 1024.0 * KB;
  constexpr double GB = 1024.0 * MB;
  constexpr double TB = 1024.0 * GB;

  if (size < KB) return absl::StrFormat("%u B", size);
  if (size < MB) return absl::StrFormat("%.2f KB", size / KB);
  if (size < GB) return absl::StrFormat("%.2f MB", size / MB);
  if (size < TB) return absl::StrFormat("%.2f GB", size / GB);

  return absl::StrFormat("%.2f TB", size / TB);
}

inline std::string GetPrettyTime(absl::Duration duration) {
  constexpr double Day = 24;

  std::string res;

  if (duration < absl::Microseconds(1)) {
    res = absl::StrFormat("%.3f ns", absl::ToDoubleNanoseconds(duration));
  } else if (duration < absl::Milliseconds(1)) {
    res = absl::StrFormat("%.3f us", absl::ToDoubleMicroseconds(duration));
  } else if (duration < absl::Seconds(1)) {
    res = absl::StrFormat("%.3f ms", absl::ToDoubleMilliseconds(duration));
  } else if (duration < absl::Minutes(1)) {
    res = absl::StrFormat("%.3f s", absl::ToDoubleSeconds(duration));
  } else if (duration < absl::Hours(1)) {
    res = absl::StrFormat("%.3f min", absl::ToDoubleMinutes(duration));
  } else if (duration < absl::Hours(Day)) {
    res = absl::StrFormat("%.3f h", absl::ToDoubleHours(duration));
  } else {
    res = absl::StrFormat("%.3f days", absl::ToDoubleHours(duration) / Day);
  }

  return res;
}

#endif  // ORBIT_CORE_CORE_UTILS_H_
