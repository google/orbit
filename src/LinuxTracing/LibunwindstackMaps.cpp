// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LibunwindstackMaps.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "OrbitBase/Logging.h"
#include "unwindstack/MapInfo.h"
#include "unwindstack/Maps.h"
#include "unwindstack/SharedString.h"

namespace orbit_linux_tracing {

namespace {

class LibunwindstackMapsImpl : public LibunwindstackMaps {
 public:
  explicit LibunwindstackMapsImpl(std::unique_ptr<unwindstack::BufferMaps> maps)
      : maps_{std::move(maps)} {}

  std::shared_ptr<unwindstack::MapInfo> Find(uint64_t pc) override { return maps_->Find(pc); }

  unwindstack::Maps* Get() override { return maps_.get(); }

  void AddAndSort(uint64_t start, uint64_t end, uint64_t offset, uint64_t flags,
                  std::string_view name) override;

 private:
  std::unique_ptr<unwindstack::BufferMaps> maps_;
};

void LibunwindstackMapsImpl::AddAndSort(uint64_t start, uint64_t end, uint64_t offset,
                                        uint64_t flags, std::string_view name) {
  // First, remove existing maps that are fully contained in the new map, and resize or split
  // existing maps that intersect with the new map. This is how the kernel handles memory mappings
  // when using `mmap` with `MAP_FIXED` causes a new map that overlaps with existing ones. From the
  // manpage of `mmap`: "`MAP_FIXED` [...] If the memory region specified by `addr` and `length`
  // overlaps pages of any existing mapping(s), then the overlapped part of the existing mapping(s)
  // will be discarded."
  // Only then we can add the new map, knowing it will not overlap with any existing one.

  // Start from the first existing MapInfo that ends after the start of the new map. All MapInfos
  // before that should remain untouched.
  auto map_info_it =
      std::upper_bound(maps_->begin(), maps_->end(), start,
                       [](uint64_t address, const std::shared_ptr<unwindstack::MapInfo>& map_info) {
                         return address < map_info->end();
                       });

  while (map_info_it != maps_->end()) {
    std::shared_ptr<unwindstack::MapInfo> map_info = *map_info_it;
    // Because of how we initialized map_info_it.
    ORBIT_CHECK(map_info->end() > start);

    if (end <= map_info->start()) {
      // The new map does not intersect map_info and is before it. Keep map_info untouched but add
      // the new map before it.
      maps_->Insert(map_info_it, start, end, offset, flags, std::string{name});
      // The new map will not intersect any other existing map, so stop.
      return;
    }

    if (start <= map_info->start() && end >= map_info->end()) {
      // The new map encloses map_info. Remove map_info.
      map_info_it = maps_->erase(map_info_it);

    } else if (start <= map_info->start()) {
      uint64_t new_offset = 0;
      if (!map_info->name().empty() && map_info->name().c_str()[0] != '[') {
        // This was a file mapping: update the offset.
        new_offset = map_info->offset() + (end - map_info->start());
      }
      // The new map intersects the first part of map_info. Keep the second part of map_info but add
      // the new map before it.
      map_info_it = maps_->erase(map_info_it);
      map_info_it = maps_->Insert(map_info_it, start, end, offset, flags, std::string{name});
      ++map_info_it;
      maps_->Insert(map_info_it, end, map_info->end(), new_offset, map_info->flags(),
                    map_info->name());
      // The new map will not intersect any other existing map, so stop.
      return;

    } else if (end >= map_info->end()) {
      // The new map intersects the second part of map_info. Keep the first part of map_info.
      map_info_it = maps_->erase(map_info_it);
      map_info_it = maps_->Insert(map_info_it, map_info->start(), start, map_info->offset(),
                                  map_info->flags(), map_info->name());
      ++map_info_it;

    } else {
      // The new map intersects the central part of map_info. Keep the first and last part of
      // map_info but add the new map in between.
      ORBIT_CHECK(start > map_info->start() && end < map_info->end());
      map_info_it = maps_->erase(map_info_it);
      {
        // Keep the first part of map_info.
        map_info_it = maps_->Insert(map_info_it, map_info->start(), start, map_info->offset(),
                                    map_info->flags(), map_info->name());
        ++map_info_it;
      }
      {
        // Add the new map.
        map_info_it = maps_->Insert(map_info_it, start, end, offset, flags, std::string{name});
        ++map_info_it;
      }
      {
        // Keep the last part of map_info.
        uint64_t new_offset = 0;
        if (!map_info->name().empty() && map_info->name().c_str()[0] != '[') {
          // This was a file mapping: update the offset.
          new_offset = map_info->offset() + (end - map_info->start());
        }
        maps_->Insert(map_info_it, end, map_info->end(), new_offset, map_info->flags(),
                      map_info->name());
      }
      // The new map will not intersect any other existing map, so stop.
      return;
    }
  }

  // If the new map has not been added yet, it goes at the end.
  ORBIT_CHECK(maps_->Total() == 0 || (*(maps_->end() - 1))->end() <= start);
  maps_->Insert(maps_->end(), start, end, offset, flags, std::string{name});
}

}  // namespace

std::unique_ptr<LibunwindstackMaps> LibunwindstackMaps::ParseMaps(std::string_view maps_buffer) {
  std::string buffer{maps_buffer};  // Note that BufferMaps implicitly assume this string stays
                                    // alive until Parse was called.
  auto maps = std::make_unique<unwindstack::BufferMaps>(buffer.c_str());
  if (!maps->Parse()) {
    return nullptr;
  }
  return std::make_unique<LibunwindstackMapsImpl>(std::move(maps));
}

}  // namespace orbit_linux_tracing