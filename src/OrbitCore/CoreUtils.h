// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CORE_UTILS_H_
#define ORBIT_CORE_CORE_UTILS_H_

#include <absl/strings/str_format.h>
#include <absl/time/time.h>

std::string GetPrettySize(uint64_t size);

std::string GetPrettyTime(absl::Duration duration);

#endif  // ORBIT_CORE_CORE_UTILS_H_
