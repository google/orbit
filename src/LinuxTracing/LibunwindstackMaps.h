// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_LIBUNWINDSTACK_MAPS_H_
#define LINUX_TRACING_LIBUNWINDSTACK_MAPS_H_

#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>

namespace orbit_linux_tracing {

// Wrapper around unwindstack::Maps that simplifies keeping the initial snapshot up to date when new
// mappings are created. It also handles the case of new mappings overlapping existing ones.
class LibunwindstackMaps {
 public:
  virtual ~LibunwindstackMaps() = default;

  virtual std::shared_ptr<unwindstack::MapInfo> Find(uint64_t pc) = 0;
  virtual unwindstack::Maps* Get() = 0;
  virtual void AddAndSort(uint64_t start, uint64_t end, uint64_t offset, uint64_t flags,
                          const std::string& name) = 0;

  static std::unique_ptr<LibunwindstackMaps> ParseMaps(const std::string& maps_buffer);
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_LIBUNWINDSTACK_MAPS_H_
