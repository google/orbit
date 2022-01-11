// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_CAPTURE_SERVICE_MEMORY_WATCHDOG_H_
#define LINUX_CAPTURE_SERVICE_MEMORY_WATCHDOG_H_

#include <stdint.h>

#include <optional>
#include <string_view>

namespace orbit_linux_capture_service {

[[nodiscard]] uint64_t GetPhysicalMemoryInBytes();

// In header file for testing.
[[nodiscard]] std::optional<uint64_t> ExtractRssInPagesFromProcPidStat(
    std::string_view proc_pid_stat);

[[nodiscard]] std::optional<uint64_t> ReadRssInBytesFromProcPidStat();

}  // namespace orbit_linux_capture_service

#endif  // LINUX_CAPTURE_SERVICE_MEMORY_WATCHDOG_H_
