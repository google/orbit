// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LibunwindstackMaps.h"

#include "OrbitBase/Logging.h"

namespace orbit_linux_tracing {

namespace {

class LibunwindstackMapsImpl : public LibunwindstackMaps {
 public:
  explicit LibunwindstackMapsImpl(std::unique_ptr<unwindstack::BufferMaps> maps)
      : maps_{std::move(maps)} {}

  std::shared_ptr<unwindstack::MapInfo> Find(uint64_t pc) override { return maps_->Find(pc); }

  unwindstack::Maps* Get() override { return maps_.get(); }

  void AddAndSort(uint64_t start, uint64_t end, uint64_t offset, uint64_t flags,
                  const std::string& name) override;

 private:
  std::unique_ptr<unwindstack::BufferMaps> maps_;
};

void LibunwindstackMapsImpl::AddAndSort(uint64_t start, uint64_t end, uint64_t offset,
                                        uint64_t flags, const std::string& name) {
  // First, remove existing maps that are fully contained in the new map, and resize or split
  // existing maps that intersect with the new map. This is how the kernel handles memory mappings
  // when using `mmap` with `MAP_FIXED` causes a new map that overlaps with existing ones. From the
  // manpage of `mmap`: "`MAP_FIXED` [...] If the memory region specified by `addr` and `length`
  // overlaps pages of any existing mapping(s), then the overlapped part of the existing mapping(s)
  // will be discarded."
  for (auto map_info_it = maps_->begin(); map_info_it != maps_->end();) {
    std::shared_ptr<unwindstack::MapInfo> map_info = *map_info_it;
    if (end <= map_info->start() || start >= map_info->end()) {
      // The new map does not intersect map_info. Keep map_info untouched (do nothing).
      ++map_info_it;

    } else if (start <= map_info->start() && end >= map_info->end()) {
      // The new map encloses map_info. Remove map_info.
      map_info_it = maps_->erase(map_info_it);

    } else if (start <= map_info->start()) {
      // The new map intersects the first part of map_info. Keep the second part of map_info.
      uint64_t new_offset = 0;
      if (!map_info->name().empty() && map_info->name().c_str()[0] != '[') {
        // This was a file mapping: update the offset.
        new_offset = map_info->offset() + (end - map_info->start());
      }
      map_info_it = maps_->erase(map_info_it);
      map_info_it = maps_->Insert(map_info_it, end, map_info->end(), new_offset, map_info->flags(),
                                  map_info->name());
      ++map_info_it;

    } else if (end >= map_info->end()) {
      // The new map intersects the second part of map_info. Keep the first part of map_info.
      map_info_it = maps_->erase(map_info_it);
      map_info_it = maps_->Insert(map_info_it, map_info->start(), start, map_info->offset(),
                                  map_info->flags(), map_info->name());
      ++map_info_it;

    } else {
      // The new map intersects the central part of map_info.
      ORBIT_CHECK(start > map_info->start() && end < map_info->end());
      map_info_it = maps_->erase(map_info_it);
      {
        // Keep the first part of map_info.
        map_info_it = maps_->Insert(map_info_it, map_info->start(), start, map_info->offset(),
                                    map_info->flags(), map_info->name());
        ++map_info_it;
      }
      {
        // Keep the last part of map_info.
        uint64_t new_offset = 0;
        if (!map_info->name().empty() && map_info->name().c_str()[0] != '[') {
          // This was a file mapping: update the offset.
          new_offset = map_info->offset() + (end - map_info->start());
        }
        map_info_it = maps_->Insert(map_info_it, end, map_info->end(), new_offset,
                                    map_info->flags(), map_info->name());
        ++map_info_it;
      }
    }
  }

  // Now add the new map. For simplicity, add at the end and sort.
  maps_->Add(start, end, offset, flags, name);
  maps_->Sort();
}

}  // namespace

std::unique_ptr<LibunwindstackMaps> LibunwindstackMaps::ParseMaps(const std::string& maps_buffer) {
  auto maps = std::make_unique<unwindstack::BufferMaps>(maps_buffer.c_str());
  if (!maps->Parse()) {
    return nullptr;
  }
  return std::make_unique<LibunwindstackMapsImpl>(std::move(maps));
}

}  // namespace orbit_linux_tracing