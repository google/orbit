// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CORE_UTILS_H_
#define ORBIT_CORE_CORE_UTILS_H_

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <absl/time/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <codecvt>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iterator>
#include <map>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

template <class T>
inline void Append(std::vector<T>& dest, const std::vector<T>& source) {
  dest.insert(std::end(dest), std::begin(source), std::end(source));
}

inline std::string GetPrettySize(uint64_t size) {
  constexpr double KB = 1024.0;
  constexpr double MB = 1024.0 * KB;
  constexpr double GB = 1024.0 * MB;
  constexpr double TB = 1024.0 * GB;

  if (size < KB) return absl::StrFormat("%" PRIu64 " B", size);
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
