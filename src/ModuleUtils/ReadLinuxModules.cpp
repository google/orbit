// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ModuleUtils/ReadLinuxModules.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/types/span.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "ModuleUtils/ReadLinuxMaps.h"
#include "ObjectUtils/ElfFile.h"
#include "ObjectUtils/ObjectFile.h"
#include "OrbitBase/Align.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_object_utils::CreateObjectFile;
using orbit_object_utils::ObjectFile;

namespace orbit_module_utils {

ErrorMessageOr<ModuleInfo> CreateModule(const std::filesystem::path& module_path,
                                        uint64_t start_address, uint64_t end_address) {
  // This excludes mapped character or block devices.
  if (absl::StartsWith(module_path.string(), "/dev/")) {
    return ErrorMessage(absl::StrFormat(
        "The module \"%s\" is a character or block device (is in /dev/)", module_path));
  }

  if (!std::filesystem::exists(module_path)) {
    return ErrorMessage(absl::StrFormat("The module file \"%s\" does not exist", module_path));
  }

  std::error_code error;
  uint64_t file_size = std::filesystem::file_size(module_path, error);
  if (error) {
    return ErrorMessage(
        absl::StrFormat("Unable to get size of \"%s\": %s", module_path, error.message()));
  }

  auto object_file_or_error = CreateObjectFile(module_path);
  if (object_file_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Unable to create module from object file: %s",
                                        object_file_or_error.error().message()));
  }

  ModuleInfo module_info;
  module_info.set_file_path(module_path);
  module_info.set_file_size(file_size);
  module_info.set_address_start(start_address);
  module_info.set_address_end(end_address);
  module_info.set_name(object_file_or_error.value()->GetName());
  module_info.set_load_bias(object_file_or_error.value()->GetLoadBias());
  module_info.set_build_id(object_file_or_error.value()->GetBuildId());
  module_info.set_executable_segment_offset(
      object_file_or_error.value()->GetExecutableSegmentOffset());
  for (const orbit_grpc_protos::ModuleInfo::ObjectSegment& segment :
       object_file_or_error.value()->GetObjectSegments()) {
    *module_info.add_object_segments() = segment;
  }

  if (object_file_or_error.value()->IsElf()) {
    auto* elf_file =
        dynamic_cast<orbit_object_utils::ElfFile*>((object_file_or_error.value().get()));
    ORBIT_CHECK(elf_file != nullptr);
    module_info.set_soname(elf_file->GetSoname());
    module_info.set_object_file_type(ModuleInfo::kElfFile);
  } else if (object_file_or_error.value()->IsCoff()) {
    // Apart from this, all fields we need to set for COFF files are already set.
    module_info.set_object_file_type(ModuleInfo::kCoffFile);
  }

  return module_info;
}

ErrorMessageOr<std::vector<ModuleInfo>> ReadModules(pid_t pid) {
  OUTCOME_TRY(auto&& maps, ReadAndParseMaps(pid));
  return ReadModulesFromMaps(maps);
}

namespace {

// We observed that in some cases, in particular for binaries running under Wine, a single loadable
// executable segment of an ELF file or a single executable section of a PE can be loaded into
// memory with multiple adjacent file mappings. In addition, some PEs can have multiple executable
// sections. Therefore, simply detecting modules loaded by a process from individual executable file
// mappings won't work.
//
// Instead, while scanning the /proc/[pid]/maps file, we can keep track of the module whose mappings
// we are currently processing. We consider all executable mappings that belong to this module. In
// the end, we build a ModuleInfo that spans the memory region from the start of the first
// executable mapping to the end of the last executable mapping for this module.
//
// Such ModuleInfo will carry executable_segment_offset with the assumption that the value of
// ObjectFile::GetExecutableSegmentOffset correspond to the *first* executable mapping. In the
// normal case of a single executable section and a single executable mapping, ModuleInfo will
// simply carry the address range of that one mapping.
//
// Note that, in the case of multiple executable sections, these are not necessarily adjacent, while
// ModuleInfo as constructed will represent a single contiguous address range. We believe this is
// fine, as the address range should still completely belong to the module, even if it can now
// include non-executable parts, and as the additional complexity of keeping track of multiple
// executable sections for a single module is not justified.
//
// In addition: All loadable sections of an ELF file, including the .text section, are always
// aligned in the file such that the loader can create a file mapping for them. But in the case of
// Portable Executables, the executable sections (and all the other sections) can have an offset in
// the file (PointerToRawData, multiple of FileAlignment) that is not congruent to the offset of
// that section when loaded into memory (VirtualAddress, multiple of SectionAlignment) modulo the
// page size. This doesn't fulfill the requirements on the arguments of mmap, so in these cases Wine
// cannot create a file-backed mapping for a section, and resorts to creating an anonymous mapping
// and copying the section into it. This means that, for PE binaries with this property, we cannot
// simply associate an executable mapping to the corresponding file using the path in the mapping.
//
// However, we can make an educated guess. The path of the PE will at least appear in the read-only
// mapping that corresponds to the beginning of the file, which contains the headers (because the
// offset in the file is zero and the address chosen for this mapping should always be a multiple of
// the page size). We consider the anonymous executable mappings after the first file mapping for
// this PE: if the address range of such a mapping is fully contained in the address range that we
// expect contains the PE (based on the start address of the file mapping that contains the headers
// and based on the PE's SizeOfImage), we can be confident that this mapping also belongs to the PE.
//
// This class contains logic to help ReadModulesFromMaps with keeping track of the executable maps
// of a module, and with detecting anonymous executable maps that belong to a PE. The intended usage
// is as follows:
// - Create a new instance of this class when a new file is encountered while parsing
//   `/proc/[pid]/maps`;
// - Call `AddExecFileMap` when encountering an executable file mapping for the file this instance
//   was created for.
// - Call `AddAnonExecMapIfCoffTextSection` when encountering an anonymous executable mapping.
//   Internally, this will decide whether it's likely that this map belong to the file this instance
//   was created for.
// - Finally, call `MaybeCreateModule` when encountering a file mapping for a file different than
//   the file this instance was created for, or when reaching the end of `/proc/[pid]/maps`. This
//   will create the `ModuleInfo` if the file this instance was created for is an object file.
class FileMappedIntoMemory {
 public:
  FileMappedIntoMemory(std::string file_path, uint64_t first_map_start, uint64_t first_map_offset)
      : file_path_{std::move(file_path)},
        first_map_start_{first_map_start},
        first_map_offset_{first_map_offset} {
    if (absl::StartsWith(file_path_, "/dev/")) {
      // This is a device file.
      return;
    }

    ErrorMessageOr<std::unique_ptr<ObjectFile>> object_file_or_error = CreateObjectFile(file_path_);
    if (object_file_or_error.has_error()) {
      return;
    }

    object_file_ = std::move(object_file_or_error.value());
  }

  [[nodiscard]] const std::string& GetFilePath() const { return file_path_; }

  void AddExecFileMap(uint64_t map_start, uint64_t map_end) {
    if (object_file_ == nullptr) {
      return;
    }

    if (HasAtLeastOneExecutableMap()) {
      // Note that for ELF files we always assume a single executable segment. We never observed
      // otherwise, and we wouldn't be able to handle more because the load bias can be different
      // for each segment. Hence, if there are multiple executable maps for an ELF file, we will
      // simply assume that they belong to the same executable segment (or at least that they belong
      // to executable segments with the same load bias).
      ORBIT_LOG("Adding another executable map at %#x-%#x for \"%s\"", map_start, map_end,
                file_path_);
    }

    min_exec_map_start = std::min(min_exec_map_start, map_start);
    max_exec_map_end = std::max(max_exec_map_end, map_end);
  }

  void AddAnonExecMapIfCoffTextSection(uint64_t map_start, uint64_t map_end) {
    if (object_file_ == nullptr) {
      return;
    }

    ORBIT_LOG("Trying if anonymous executable map at %#x-%#x belongs to \"%s\"", map_start, map_end,
              file_path_);
    const std::string error_message =
        absl::StrFormat("No, anonymous executable map at %#x-%#x does NOT belong to \"%s\"",
                        map_start, map_end, file_path_);

    // Remember: we are only detecting anonymous maps that correspond to executable sections of PEs,
    // because loadable segments of ELF files can always be file-mapped.
    if (!object_file_->IsCoff()) {
      ORBIT_LOG("%s: object file is not a PE", error_message);
      return;
    }

    if (first_map_offset_ != 0) {
      // We expect the first mapping for this PE to have offset zero, as the headers are also mapped
      // into memory, and they are at the beginning of the file.
      ORBIT_ERROR("%s: a map with offset 0 where the headers would be mapped is not present",
                  error_message);
      return;
    }

    // The start address of the map in which the first byte of the PE is mapped.
    // It is page-aligned because it is the start address of a map.
    const uint64_t base_address = first_map_start_;
    constexpr uint64_t kPageSize = 0x1000;
    // The end address of the map in which the last byte of the PE is mapped.
    const uint64_t end_address =
        base_address + orbit_base::AlignUp<kPageSize>(object_file_->GetImageSize());
    // We validate that the executable map is fully contained in the address range at which the PE
    // is supposed to be mapped.
    if (map_end > end_address) {
      ORBIT_LOG("%s: map is not contained in the absolute address range %#x-%#x of the PE",
                error_message, base_address, end_address);
      return;
    }

    ORBIT_LOG("Guessing that anonymous executable map at %#x-%#x belongs to \"%s\"", map_start,
              map_end, file_path_);
    min_exec_map_start = std::min(min_exec_map_start, map_start);
    max_exec_map_end = std::max(max_exec_map_end, map_end);
  }

  [[nodiscard]] std::optional<ModuleInfo> MaybeCreateModule() {
    if (!HasAtLeastOneExecutableMap()) {
      return std::nullopt;
    }

    ErrorMessageOr<ModuleInfo> module_info_or_error =
        CreateModule(file_path_, min_exec_map_start, max_exec_map_end);
    if (module_info_or_error.has_error()) {
      ORBIT_ERROR("Unable to create module: %s", module_info_or_error.error().message());
      return std::nullopt;
    }
    return std::move(module_info_or_error.value());
  }

 private:
  std::string file_path_;
  uint64_t first_map_start_;
  uint64_t first_map_offset_;
  std::unique_ptr<ObjectFile> object_file_;

  uint64_t min_exec_map_start = std::numeric_limits<uint64_t>::max();
  uint64_t max_exec_map_end = 0;
  [[nodiscard]] bool HasAtLeastOneExecutableMap() const {
    return min_exec_map_start < max_exec_map_end;
  }
};
}  // namespace

std::vector<ModuleInfo> ReadModulesFromMaps(absl::Span<const LinuxMemoryMapping> maps) {
  std::vector<ModuleInfo> result;

  std::optional<FileMappedIntoMemory> last_file_mapped_into_memory;

  for (const LinuxMemoryMapping& map : maps) {
    const uint64_t start = map.start_address();
    const uint64_t end = map.end_address();
    const uint64_t perms = map.perms();
    const uint64_t offset = map.offset();
    const uint64_t inode = map.inode();
    const std::string& pathname = map.pathname();

    // If inode equals 0, then the memory is not backed by a file.
    // If a map not backed by a file has a name, it's a special one like [stack], [heap], etc.
    if (inode == 0 && !pathname.empty()) continue;

    std::string module_path;
    if (inode != 0) {          // The mapping is file-backed.
      if (pathname.empty()) {  // Unexpected: the mapping is file-backed but no path is present.
        ORBIT_ERROR("Map at %#x-%#x has inode %u (not 0) but no path", start, end, inode);
        last_file_mapped_into_memory.reset();
        continue;
      }

      module_path = pathname;
      // Keep track of the last file we encountered. Only create a new FileMappedIntoMemory if this
      // file mapping is backed by a different file than the previous file mapping.
      if (!last_file_mapped_into_memory.has_value()) {
        last_file_mapped_into_memory.emplace(module_path, start, offset);
      } else if (module_path != last_file_mapped_into_memory->GetFilePath()) {
        std::optional<ModuleInfo> module_info = last_file_mapped_into_memory->MaybeCreateModule();
        if (module_info.has_value()) {
          result.emplace_back(std::move(module_info.value()));
        }
        last_file_mapped_into_memory.emplace(module_path, start, offset);
      }
    }

    const bool is_executable = (perms & PROT_EXEC) != 0;
    // Never create modules from non-executable mappings.
    if (!is_executable) continue;

    if (!module_path.empty()) {
      ORBIT_CHECK(last_file_mapped_into_memory.has_value());
      last_file_mapped_into_memory->AddExecFileMap(start, end);
    } else if (last_file_mapped_into_memory.has_value()) {
      last_file_mapped_into_memory->AddAnonExecMapIfCoffTextSection(start, end);
    }
  }

  if (last_file_mapped_into_memory.has_value()) {
    std::optional<ModuleInfo> module_info = last_file_mapped_into_memory->MaybeCreateModule();
    if (module_info.has_value()) {
      result.emplace_back(std::move(module_info.value()));
    }
  }

  return result;
}

}  // namespace orbit_module_utils
