// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CGROUP_AND_PROCESS_MEMORY_INFO_H_
#define CLIENT_DATA_CGROUP_AND_PROCESS_MEMORY_INFO_H_

#include <cstdint>
#include <optional>

namespace orbit_client_data {

struct CgroupAndProcessMemoryInfo {
  uint64_t timestamp_ns;
  uint64_t cgroup_name_hash;
  int64_t cgroup_limit_bytes;
  int64_t cgroup_rss_bytes;
  int64_t cgroup_mapped_file_bytes;
  int64_t process_rss_anon_kb;

  [[nodiscard]] bool HasMissingInfo() const {
    constexpr int64_t kMissingInfo = -1;
    return cgroup_limit_bytes == kMissingInfo || cgroup_rss_bytes == kMissingInfo ||
           cgroup_mapped_file_bytes == kMissingInfo || process_rss_anon_kb == kMissingInfo;
  }
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CGROUP_AND_PROCESS_MEMORY_INFO_H_
