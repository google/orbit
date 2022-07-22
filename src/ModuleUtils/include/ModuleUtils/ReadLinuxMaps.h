// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MODULE_UTILS_READ_LINUX_MAPS_H_
#define MODULE_UTILS_READ_LINUX_MAPS_H_

#ifdef __linux

#include <stdint.h>
#include <unistd.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_module_utils {

// Represents an entry in /proc/[pid]/maps.
class LinuxMemoryMapping {
 public:
  LinuxMemoryMapping(uint64_t start_address, uint64_t end_address, uint64_t perms, uint64_t offset,
                     uint64_t inode, std::string pathname)
      : start_address_{start_address},
        end_address_{end_address},
        perms_{perms},
        offset_{offset},
        inode_{inode},
        pathname_{std::move(pathname)} {}

  [[nodiscard]] uint64_t start_address() const { return start_address_; }
  [[nodiscard]] uint64_t end_address() const { return end_address_; }
  [[nodiscard]] uint64_t perms() const { return perms_; }
  [[nodiscard]] uint64_t offset() const { return offset_; }
  [[nodiscard]] uint64_t inode() const { return inode_; }
  [[nodiscard]] const std::string& pathname() const { return pathname_; }

 private:
  uint64_t start_address_;
  uint64_t end_address_;
  uint64_t perms_;
  uint64_t offset_;
  uint64_t inode_;
  std::string pathname_;
};

ErrorMessageOr<std::string> ReadMaps(pid_t pid);

[[nodiscard]] std::vector<LinuxMemoryMapping> ParseMaps(std::string_view proc_pid_maps_content);

ErrorMessageOr<std::vector<LinuxMemoryMapping>> ReadAndParseMaps(pid_t pid);

}  // namespace orbit_module_utils

#endif  // __linux

#endif  // MODULE_UTILS_READ_LINUX_MAPS_H_
