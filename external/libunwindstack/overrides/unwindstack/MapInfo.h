/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LIBUNWINDSTACK_MAP_INFO_H
#define _LIBUNWINDSTACK_MAP_INFO_H

#include <stdint.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include <unwindstack/Elf.h>

namespace unwindstack {

class MemoryFileAtOffset;

struct MapInfo {
  MapInfo(MapInfo* map_info, uint64_t start, uint64_t end, uint64_t offset, uint64_t flags,
          const char* name)
      : start(start),
        end(end),
        offset(offset),
        flags(flags),
        name(name),
        prev_map(map_info),
        load_bias(INT64_MAX),
        build_id(0) {}
  MapInfo(MapInfo* map_info, uint64_t start, uint64_t end, uint64_t offset, uint64_t flags,
          const std::string& name)
      : start(start),
        end(end),
        offset(offset),
        flags(flags),
        name(name),
        prev_map(map_info),
        load_bias(INT64_MAX),
        build_id(0) {}
  ~MapInfo();

  uint64_t start = 0;
  uint64_t end = 0;
  uint64_t offset = 0;
  uint16_t flags = 0;
  std::string name;
  std::shared_ptr<Elf> elf;
  // The offset of the beginning of this mapping to the beginning of the
  // ELF file.
  // elf_offset == offset - elf_start_offset.
  // This value is only non-zero if the offset is non-zero but there is
  // no elf signature found at that offset.
  uint64_t elf_offset = 0;
  // This value is the offset into the file of the map in memory that is the
  // start of the elf. This is not equal to offset when the linker splits
  // shared libraries into a read-only and read-execute map.
  uint64_t elf_start_offset = 0;

  MapInfo* prev_map = nullptr;

  /* ORBIT CHANGE START */ std::atomic<std::int64_t> load_bias; /* ORBIT CHANGE END */

  // This is a pointer to a new'd std::string.
  // Using an atomic value means that we don't need to lock and will
  // make it easier to move to a fine grained lock in the future.
  /* ORBIT CHANGE START */ std::atomic<std::uintptr_t> build_id; /* ORBIT CHANGE END */

  // Set to true if the elf file data is coming from memory.
  bool memory_backed_elf = false;

  // This function guarantees it will never return nullptr.
  Elf* GetElf(const std::shared_ptr<Memory>& process_memory, ArchEnum expected_arch);

  uint64_t GetLoadBias(const std::shared_ptr<Memory>& process_memory);

  Memory* CreateMemory(const std::shared_ptr<Memory>& process_memory);

  bool GetFunctionName(uint64_t addr, std::string* name, uint64_t* func_offset);

  // Returns the raw build id read from the elf data.
  std::string GetBuildID();

  // Returns the printable version of the build id (hex dump of raw data).
  std::string GetPrintableBuildID();

 private:
  MapInfo(const MapInfo&) = delete;
  void operator=(const MapInfo&) = delete;

  Memory* GetFileMemory();
  bool InitFileMemoryFromPreviousReadOnlyMap(MemoryFileAtOffset* memory);

  // Protect the creation of the elf object.
  std::mutex mutex_;
};

}  // namespace unwindstack

#endif  // _LIBUNWINDSTACK_MAP_INFO_H
