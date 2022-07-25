/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <mutex>
#include <string>

#include <android-base/stringprintf.h>
#include <android-base/strings.h>

#include <unwindstack/Elf.h>
#include <unwindstack/Log.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Object.h>
#include <unwindstack/PeCoff.h>

#include "Check.h"
#include "MemoryFileAtOffset.h"
#include "MemoryRange.h"

namespace unwindstack {

bool MapInfo::ObjectFileNotReadable() {
  const std::string& map_name = name();
  return memory_backed_object() && !map_name.empty() && map_name[0] != '[' &&
         !android::base::StartsWith(map_name, "/memfd:");
}

std::shared_ptr<MapInfo> MapInfo::GetPrevRealMap() {
  if (name().empty()) {
    return nullptr;
  }

  for (auto prev = prev_map(); prev != nullptr; prev = prev->prev_map()) {
    if (!prev->IsBlank()) {
      if (prev->name() == name()) {
        return prev;
      }
      return nullptr;
    }
  }
  return nullptr;
}

std::shared_ptr<MapInfo> MapInfo::GetNextRealMap() {
  if (name().empty()) {
    return nullptr;
  }

  for (auto next = next_map(); next != nullptr; next = next->next_map()) {
    if (!next->IsBlank()) {
      if (next->name() == name()) {
        return next;
      }
      return nullptr;
    }
  }
  return nullptr;
}

bool MapInfo::InitFileMemoryFromPreviousReadOnlyMap(MemoryFileAtOffset* memory) {
  // One last attempt, see if the previous map is read-only with the
  // same name and stretches across this map.
  auto prev_real_map = GetPrevRealMap();
  if (prev_real_map == nullptr || prev_real_map->flags() != PROT_READ ||
      prev_real_map->offset() >= offset()) {
    return false;
  }

  uint64_t map_size = end() - prev_real_map->end();
  if (!memory->Init(name(), prev_real_map->offset(), map_size)) {
    return false;
  }

  uint64_t max_size;
  if (!Elf::GetInfo(memory, &max_size) || max_size < map_size) {
    return false;
  }

  if (!memory->Init(name(), prev_real_map->offset(), max_size)) {
    return false;
  }

  set_object_offset(offset() - prev_real_map->offset());
  set_object_start_offset(prev_real_map->offset());
  return true;
}

Memory* MapInfo::GetFileMemory() {
  // Fail on device maps.
  if (flags() & MAPS_FLAGS_DEVICE_MAP) {
    return nullptr;
  }

  std::unique_ptr<MemoryFileAtOffset> memory(new MemoryFileAtOffset);
  if (offset() == 0) {
    if (memory->Init(name(), 0)) {
      return memory.release();
    }
    return nullptr;
  }

  if (IsPotentiallyPeCoffFile(name())) {
    // Always map the whole PE, as we always need the headers.
    // This is similar to "No elf at offset, try to init as if the whole file is an elf" below.
    if (memory->Init(name(), 0)) {
      set_object_offset(offset());
      // We only support PEs mapped from file, and by PE specification the headers are always at the
      // beginning of the file (offset zero in the file). Therefore, keep it simple and assume that
      // the PE is always mapped starting from offset zero, where the headers are.
      set_object_start_offset(0);
      return memory.release();
    }
    return nullptr;
  }

  // These are the possibilities when the offset is non-zero.
  // - There is an elf file embedded in a file, and the offset is the
  //   the start of the elf in the file.
  // - There is an elf file embedded in a file, and the offset is the
  //   the start of the executable part of the file. The actual start
  //   of the elf is in the read-only segment preceeding this map.
  // - The whole file is an elf file, and the offset needs to be saved.
  //
  // Map in just the part of the file for the map. If this is not
  // a valid elf, then reinit as if the whole file is an elf file.
  // If the offset is a valid elf, then determine the size of the map
  // and reinit to that size. This is needed because the dynamic linker
  // only maps in a portion of the original elf, and never the symbol
  // file data.
  //
  // For maps with MAPS_FLAGS_JIT_SYMFILE_MAP, the map range is for a JIT function,
  // which can be smaller than elf header size. So make sure map_size is large enough
  // to read elf header.
  uint64_t map_size = std::max<uint64_t>(end() - start(), sizeof(ElfTypes64::Ehdr));
  if (!memory->Init(name(), offset(), map_size)) {
    return nullptr;
  }

  // Check if the start of this map is an embedded elf.
  uint64_t max_size = 0;
  if (Elf::GetInfo(memory.get(), &max_size)) {
    set_object_start_offset(offset());
    if (max_size > map_size) {
      if (memory->Init(name(), offset(), max_size)) {
        return memory.release();
      }
      // Try to reinit using the default map_size.
      if (memory->Init(name(), offset(), map_size)) {
        return memory.release();
      }
      set_object_start_offset(0);
      return nullptr;
    }
    return memory.release();
  }

  // No elf at offset, try to init as if the whole file is an elf.
  if (memory->Init(name(), 0) && Elf::IsValidElf(memory.get())) {
    set_object_offset(offset());
    return memory.release();
  }

  // See if the map previous to this one contains a read-only map
  // that represents the real start of the elf data.
  if (InitFileMemoryFromPreviousReadOnlyMap(memory.get())) {
    return memory.release();
  }

  // Failed to find elf at start of file or at read-only map, return
  // file object from the current map.
  if (memory->Init(name(), offset(), map_size)) {
    return memory.release();
  }
  return nullptr;
}

// Unlike loadable sections of ELF files, the .text section (and other sections) of a Portable
// Executable can have an offset in the file (PointerToRawData, multiple of FileAlignment) that is
// not congruent to the offset of that section when loaded into memory (VirtualAddress, multiple of
// SectionAlignment) modulo the page size. This doesn't fulfill the requirements on the arguments of
// mmap, so in these cases Wine cannot create a file-backed mapping for the .text section, and
// resorts to creating an anonymous mapping and copying the .text section into it. This means that,
// for PE binaries with this property, we cannot simply associate an executable mapping to the
// corresponding file using the path in the mapping.
//
// However, we can make an educated guess. The path of a PE will at least appear in the read-only
// mapping that corresponds to the beginning of the file, which contains the headers (because the
// offset in the file is zero and the address chosen for this mapping should always be a multiple of
// the page size). Therefore, let's consider the file of the last file mapping that precedes this
// anonymous mapping. If this file is a PE, and the address range of this anonymous mapping is fully
// contained in the address range that we expect contains the PE (based on the start address of the
// file mapping that contains the headers and based on the PE's SizeOfImage), then we can be
// confident that this anonymous mapping also belongs to the PE. Note how this considers the case we
// observed of PEs with multiple executable sections.
//
// This method is the counterpart of GetFileMemory for anonymous executable mappings and contains
// the detection mechanism described.
Memory* MapInfo::GetFileMemoryFromAnonExecMapIfPeCoffTextSection() {
  // We expect this method to be called only for non-special anonymous executable mappings.
  CHECK(name().empty());
  CHECK((flags() & PROT_EXEC) != 0);

  std::shared_ptr<MapInfo> map_info_it = prev_map();

  // Find the previous file mapping.
  // Note that when the first character of a map name is '[', the mapping is a special one like
  // [stack], [heap], etc. Even if such mapping has a name, it's not file-backed. An alternative way
  // of detecting that would be to check if the inode is 0 (but that's not available here).
  while (map_info_it != nullptr &&
         (map_info_it->name().empty() || map_info_it->name().c_str()[0] == '[')) {
    map_info_it = map_info_it->prev_map();
  }
  if (map_info_it == nullptr) {
    // There is no previous file mapping.
    return nullptr;
  }

  if ((map_info_it->flags() & MAPS_FLAGS_DEVICE_MAP) != 0) {
    // The previous file mapping corresponds to a character or block device.
    return nullptr;
  }

  // This is the candidate name (file path).
  std::string prev_name{map_info_it->name()};
  CHECK(!prev_name.empty());

  // Find the first file mapping for the same name (file path) as the file mapping we just found.
  // This is because, even if the .text section is mapped with an anonymous mapping, it could happen
  // that not only the header but also other sections preceding the .text (and following the header)
  // be mapped with a file mapping, if their offset in the file allows it.
  std::shared_ptr<MapInfo> first_map_info_with_prev_name = nullptr;
  while (map_info_it != nullptr &&
         (map_info_it->name().empty() || map_info_it->name().c_str()[0] == '[' ||
          map_info_it->name() == prev_name)) {
    if (map_info_it->name() == prev_name) {
      first_map_info_with_prev_name = map_info_it;
    }
    map_info_it = map_info_it->prev_map();
  }
  // The first iteration of the previous loop assigned map_info_it to this.
  CHECK(first_map_info_with_prev_name != nullptr);

  Log::Info("Trying if anonymous executable map at %#lx-%#lx belongs to \"%s\"", start(), end(),
            prev_name.c_str());
  const std::string error_message = android::base::StringPrintf(
      "No, anonymous executable map at %#lx-%#lx does NOT belong to \"%s\"", start(), end(),
      prev_name.c_str());

  if (first_map_info_with_prev_name->offset() != 0) {
    // We expect the first mapping for this PE to have offset zero, as the headers are also mapped
    // into memory, and they are at the beginning of the file.
    Log::Info("%s: a map with offset 0 for this file was not found", error_message.c_str());
    return nullptr;
  }

  auto file_memory = std::make_unique<MemoryFileAtOffset>();
  // Always use zero as offset because the headers of a PE should always be right at the beginning
  // of the file.
  if (!file_memory->Init(prev_name, 0)) {
    Log::Info("%s: could not open the file", error_message.c_str());
    return nullptr;
  }

  // Ideally we would use first_map_info_with_prev_name->GetObject(), but that's not right because
  // it also requires an expected architecture. Since this function is called at most once for each
  // map, we are fine with temporarily creating a new PeCoff.
  PeCoff pe{file_memory.release()};
  if (!pe.Init()) {
    // The candidate file path doesn't correspond to a PE/COFF.
    Log::Info("%s: this is not a PE", error_message.c_str());
    return nullptr;
  }

  // The address range at which the PE is supposed to be mapped.
  const uint64_t base_address = first_map_info_with_prev_name->start();
  const uint64_t end_address = base_address + pe.GetSizeOfImage();

  constexpr uint64_t kPageSize = 0x1000;
  // end_address is generally already aligned as SizeOfImage must be a multiple of SectionAlignment,
  // which by default is a multiple of the page size, but let's enforce it.
  const uint64_t end_address_aligned_up = (end_address + (kPageSize - 1)) & ~(kPageSize - 1);

  // We validate that the executable map is fully contained in the address range at which the PE is
  // supposed to be mapped.
  if (end() > end_address_aligned_up) {
    Log::Info("%s: map is not contained in the absolute address range %#lx-%#lx of the PE",
              error_message.c_str(), base_address, end_address_aligned_up);
    return nullptr;
  }

  auto memory = std::make_unique<MemoryFileAtOffset>();
  // Always map the whole PE, as we always need the headers.
  if (!memory->Init(prev_name, 0)) {
    Log::Info("%s: could not open the file again", error_message.c_str());
    return nullptr;
  }

  // Compute the RVA of the start of this map.
  const uint64_t map_start_rva = start() - base_address;

  Log::Info("Guessing that anonymous executable map at %#lx-%#lx belongs to \"%s\" with RVA %#lx",
            start(), end(), prev_name.c_str(), map_start_rva);

  // Set some ObjectFields, like GetFileMemory does.
  set_memory_backed_object(false);
  // If the section is not mapped from the start of the map, the first address of the map could
  // correspond to no actual offset in the file (and the beginning of the map would be padded with
  // zeros). Therefore, we set zero here, and we use object_rva instead.
  set_object_offset(0);
  set_object_rva(map_start_rva);
  // Again, assume that the headers of the PE are always at the beginning of the file (offset zero
  // in the file) and hence are mapped with offset zero.
  set_object_start_offset(0);

  return memory.release();
}

Memory* MapInfo::CreateMemory(const std::shared_ptr<Memory>& process_memory) {
  if (end() <= start()) {
    return nullptr;
  }

  set_object_offset(0);

  // Fail on device maps.
  if (flags() & MAPS_FLAGS_DEVICE_MAP) {
    return nullptr;
  }

  if (!name().empty() && IsPotentiallyPeCoffFile(name())) {
    Memory* memory = GetFileMemory();
    // Return even if this is a nullptr: for MapInfo instances with a filename of a PE/COFF file,
    // we only support creating the memory from file. Anonymously mapped sections of a PE/COFF
    // file are handled below.
    return memory;
  }

  // First try and use the file associated with the info.
  if (!name().empty()) {
    Memory* memory = GetFileMemory();
    if (memory != nullptr) {
      return memory;
    }
  }

  // Try if this is the .text section of a PE mapped anonymously by Wine because the offset in the
  // file and the offset in memory relative to the image base are not compatible with mmap.
  if (name().empty() && (flags() & PROT_EXEC) != 0) {
    Memory* memory = GetFileMemoryFromAnonExecMapIfPeCoffTextSection();
    if (memory != nullptr) {
      return memory;
    }
  }

  if (process_memory == nullptr) {
    return nullptr;
  }

  set_memory_backed_object(true);

  // Need to verify that this elf is valid. It's possible that
  // only part of the elf file to be mapped into memory is in the executable
  // map. In this case, there will be another read-only map that includes the
  // first part of the elf file. This is done if the linker rosegment
  // option is used.
  std::unique_ptr<MemoryRange> memory(new MemoryRange(process_memory, start(), end() - start(), 0));
  if (Elf::IsValidElf(memory.get())) {
    set_object_start_offset(offset());

    auto next_real_map = GetNextRealMap();

    // Might need to peek at the next map to create a memory object that
    // includes that map too.
    if (offset() != 0 || next_real_map == nullptr || offset() >= next_real_map->offset()) {
      return memory.release();
    }

    // There is a possibility that the elf object has already been created
    // in the next map. Since this should be a very uncommon path, just
    // redo the work. If this happens, the elf for this map will eventually
    // be discarded.
    MemoryRanges* ranges = new MemoryRanges;
    ranges->Insert(new MemoryRange(process_memory, start(), end() - start(), 0));
    ranges->Insert(new MemoryRange(process_memory, next_real_map->start(),
                                   next_real_map->end() - next_real_map->start(),
                                   next_real_map->offset() - offset()));

    return ranges;
  }

  auto prev_real_map = GetPrevRealMap();

  // Find the read-only map by looking at the previous map. The linker
  // doesn't guarantee that this invariant will always be true. However,
  // if that changes, there is likely something else that will change and
  // break something.
  if (offset() == 0 || prev_real_map == nullptr || prev_real_map->offset() >= offset()) {
    set_memory_backed_object(false);
    return nullptr;
  }

  // Make sure that relative pc values are corrected properly.
  set_object_offset(offset() - prev_real_map->offset());
  // Use this as the elf start offset, otherwise, you always get offsets into
  // the r-x section, which is not quite the right information.
  set_object_start_offset(prev_real_map->offset());

  std::unique_ptr<MemoryRanges> ranges(new MemoryRanges);
  if (!ranges->Insert(new MemoryRange(process_memory, prev_real_map->start(),
                                      prev_real_map->end() - prev_real_map->start(), 0))) {
    return nullptr;
  }
  if (!ranges->Insert(new MemoryRange(process_memory, start(), end() - start(), object_offset()))) {
    return nullptr;
  }
  return ranges.release();
}

class ScopedObjectCacheLock {
 public:
  ScopedObjectCacheLock() {
    if (Object::CachingEnabled()) Object::CacheLock();
  }
  ~ScopedObjectCacheLock() {
    if (Object::CachingEnabled()) Object::CacheUnlock();
  }
};

Object* MapInfo::GetObject(const std::shared_ptr<Memory>& process_memory, ArchEnum expected_arch) {
  // Make sure no other thread is trying to add the object to this map.
  std::lock_guard<std::mutex> guard(object_mutex());

  if (object().get() != nullptr) {
    return object().get();
  }

  ScopedObjectCacheLock object_cache_lock;
  if (Object::CachingEnabled() && !name().empty()) {
    if (Object::CacheGet(this)) {
      return object().get();
    }
  }

  Memory* memory = CreateMemory(process_memory);
  if (IsPotentiallyPeCoffFile(memory)) {
    object().reset(new PeCoff(memory));
  } else {
    object().reset(new Elf(memory));
  }

  // If the init fails, keep the object around as an invalid object so we
  // don't try to reinit the object.
  object()->Init();
  if (object()->valid() && expected_arch != object()->arch()) {
    // Make the object invalid, mismatch between arch and expected arch.
    object()->Invalidate();
  }

  if (!object()->valid()) {
    set_object_start_offset(offset());
  } else if (auto prev_real_map = GetPrevRealMap(); prev_real_map != nullptr &&
                                                    prev_real_map->flags() == PROT_READ &&
                                                    prev_real_map->offset() < offset()) {
    // If there is a read-only map then a read-execute map that represents the
    // same elf object, make sure the previous map is using the same elf
    // object if it hasn't already been set. Locking this should not result
    // in a deadlock as long as the invariant that the code only ever tries
    // to lock the previous real map holds true.
    std::lock_guard<std::mutex> guard(prev_real_map->object_mutex());
    if (prev_real_map->object() == nullptr) {
      // Need to verify if the map is the previous read-only map.
      prev_real_map->set_object(object());
      prev_real_map->set_memory_backed_object(memory_backed_object());
      prev_real_map->set_object_start_offset(object_start_offset());
      prev_real_map->set_object_offset(prev_real_map->offset() - object_start_offset());
    } else if (prev_real_map->object_start_offset() == object_start_offset()) {
      // Discard this elf, and use the elf from the previous map instead.
      set_object(prev_real_map->object());
    }
  }

  // Cache the elf only after all of the above checks since we might
  // discard the original elf we created.
  if (Elf::CachingEnabled()) {
    Elf::CacheAdd(this);
  }
  return object().get();
}

bool MapInfo::GetFunctionName(uint64_t addr, SharedString* name, uint64_t* func_offset) {
  {
    // Make sure no other thread is trying to update this elf object.
    std::lock_guard<std::mutex> guard(object_mutex());
    if (object() == nullptr) {
      return false;
    }
  }
  // No longer need the lock, once the elf object is created, it is not deleted
  // until this object is deleted.
  return object()->GetFunctionName(addr, name, func_offset);
}

uint64_t MapInfo::GetLoadBias() {
  uint64_t cur_load_bias = load_bias().load();
  if (cur_load_bias != UINT64_MAX) {
    return cur_load_bias;
  }

  Object* cached_obj = GetCachedObj();
  if (cached_obj == nullptr) {
    return UINT64_MAX;
  }

  if (cached_obj->valid()) {
    cur_load_bias = cached_obj->GetLoadBias();
    set_load_bias(cur_load_bias);
    return cur_load_bias;
  }

  set_load_bias(0);
  return 0;
}

uint64_t MapInfo::GetLoadBias(const std::shared_ptr<Memory>& process_memory) {
  uint64_t cur_load_bias = GetLoadBias();
  if (cur_load_bias != UINT64_MAX) {
    return cur_load_bias;
  }

  if (IsElf()) {
    // Call lightweight static function that will only read enough of the
    // object data to get the load bias.
    std::unique_ptr<Memory> memory(CreateMemory(process_memory));
    cur_load_bias = Elf::GetLoadBias(memory.get());
    set_load_bias(cur_load_bias);
    return cur_load_bias;
  } else {
    // TODO: Handle other object file types.
    return cur_load_bias;
  }
}

MapInfo::~MapInfo() {
  ObjectFields* object_fields = object_fields_.load();
  if (object_fields != nullptr) {
    delete object_fields->build_id_.load();
    delete object_fields;
  }
}

std::string MapInfo::GetFullName() {
  Object* cached_obj = GetCachedObj();
  if (cached_obj == nullptr || object_start_offset() == 0 || name().empty()) {
    return name();
  }

  std::string soname = cached_obj->GetSoname();
  if (soname.empty()) {
    return name();
  }

  std::string full_name(name());
  full_name += '!';
  full_name += soname;
  return full_name;
}

SharedString MapInfo::GetBuildID() {
  SharedString* id = build_id().load();
  if (id != nullptr) {
    return *id;
  }

  // No need to lock, at worst if multiple threads do this at the same
  // time it should be detected and only one thread should win and
  // save the data.

  std::string result;
  Object* cached_obj = GetCachedObj();
  if (cached_obj != nullptr) {
    result = cached_obj->GetBuildID();
  } else {
    // This will only work if we can get the file associated with this memory.
    // If this is only available in memory, then the section name information
    // is not present and we will not be able to find the build id info.
    std::unique_ptr<Memory> memory(GetFileMemory());
    if (memory != nullptr) {
      if (IsElf()) {
        result = Elf::GetBuildID(memory.get());
      } else {
        // TODO: Handle other object file types.
        result = "";
      }
    }
  }
  return SetBuildID(std::move(result));
}

SharedString MapInfo::SetBuildID(std::string&& new_build_id) {
  std::unique_ptr<SharedString> new_build_id_ptr(new SharedString(std::move(new_build_id)));
  SharedString* expected_id = nullptr;
  // Strong version since we need to reliably return the stored pointer.
  if (build_id().compare_exchange_strong(expected_id, new_build_id_ptr.get())) {
    // Value saved, so make sure the memory is not freed.
    return *new_build_id_ptr.release();
  } else {
    // The expected value is set to the stored value on failure.
    return *expected_id;
  }
}

MapInfo::ObjectFields& MapInfo::GetObjectFields() {
  ObjectFields* object_fields = object_fields_.load(std::memory_order_acquire);
  if (object_fields != nullptr) {
    return *object_fields;
  }
  // Allocate and initialize the field in thread-safe way.
  std::unique_ptr<ObjectFields> desired(new ObjectFields());
  ObjectFields* expected = nullptr;
  // Strong version is reliable. Weak version might randomly return false.
  if (object_fields_.compare_exchange_strong(expected, desired.get())) {
    return *desired.release();  // Success: we transferred the pointer ownership to the field.
  } else {
    return *expected;  // Failure: 'expected' is updated to the value set by the other thread.
  }
}

std::string MapInfo::GetPrintableBuildID() {
  std::string raw_build_id = GetBuildID();
  return Elf::GetPrintableBuildID(raw_build_id);
}

}  // namespace unwindstack
