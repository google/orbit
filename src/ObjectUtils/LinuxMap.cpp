// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/LinuxMap.h"

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

#include "ObjectUtils/CoffFile.h"
#include "ObjectUtils/ElfFile.h"
#include "ObjectUtils/ObjectFile.h"
#include "OrbitBase/Align.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

using orbit_grpc_protos::ModuleInfo;
using orbit_object_utils::CreateObjectFile;
using orbit_object_utils::ElfFile;
using orbit_object_utils::ObjectFile;

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

  if (object_file_or_error.value()->IsElf()) {
    auto* elf_file = dynamic_cast<ElfFile*>((object_file_or_error.value().get()));
    ORBIT_CHECK(elf_file != nullptr);
    module_info.set_soname(elf_file->GetSoname());
    module_info.set_object_file_type(ModuleInfo::kElfFile);
  } else if (object_file_or_error.value()->IsCoff()) {
    module_info.set_object_file_type(ModuleInfo::kCoffFile);
  }

  // All fields we need to set for COFF files are already set, no need to handle COFF
  // specifically here.
  return module_info;
}

ErrorMessageOr<std::vector<ModuleInfo>> ReadModules(int32_t pid) {
  std::filesystem::path proc_maps_path{absl::StrFormat("/proc/%i/maps", pid)};
  OUTCOME_TRY(auto&& proc_maps_data, orbit_base::ReadFileToString(proc_maps_path));
  return ParseMaps(proc_maps_data);
}

namespace {

// Loadable sections of an ELF file, including the .text section, are always aligned in the file
// such that the loader can create a file mapping for them. We can therefore simply detect modules
// loaded by a process from the executable file mappings.
//
// But in the case of Portable Executables, the .text section (and other sections) can have an
// offset in the file (PointerToRawData, multiple of FileAlignment) that is not congruent to the
// offset of that section when loaded into memory (VirtualAddress, multiple of SectionAlignment)
// modulo the page size. This doesn't fulfill the requirements on the arguments of mmap, so in these
// cases Wine cannot create a file-backed mapping for the .text section, and resorts to creating an
// anonymous mapping and copying the .text section into it. This means that, for PE binaries with
// this property, we cannot simply associate an executable mapping to the corresponding file using
// the path in the mapping.
//
// However, we can make an educated guess. The path of the PE will at least appear in the read-only
// mapping that corresponds to the beginning of the file, which contains the headers (because the
// offset in the file is zero and the address chosen for this mapping should always be a multiple of
// the page size). If the executable file mapping for the .text section is not present, we consider
// the anonymous executable mappings after the first file mapping for this PE: if the offset and
// size of such a mapping are compatible with the address range where the .text section would be
// loaded based on the header for the section (in particular, VirtualAddress and VirtualSize), we
// can be quite sure that this is the mapping we are looking for.
// Note that we assume that a PE (or an ELF file) only has one .text section and one executable
// mapping: this is what we observed and is what we support.
//
// This class contains logic to help ParseMaps with the detection mechanism. The intended usage is
// as follows:
// - Create a new instance of this class when a new file is encountered while parsing
//   `/proc/[pid]/maps`;
// - Call `MarkExecutableMapEncountered` when encountering an executable file mapping for the file
//   this instance was created for.
// - Use `TryIfAnonExecMapIsCoffTextSection` to query if an anonymous executable mapping is actually
//   the PE .text section of the file this instance was created for. This will only return true once
//   for each instance of this class, as it calls `MarkExecutableMapEncountered` on success.
class FileMappedIntoMemory {
 public:
  FileMappedIntoMemory(std::string file_path, uint64_t first_map_start, uint64_t first_map_offset)
      : file_path_{std::move(file_path)}, base_address_{first_map_start - first_map_offset} {
    if (first_map_start < first_map_offset) {
      // In this case, base_address_ is the result of an underflow. This shouldn't normally happen,
      // so immediately set coff_text_section_map_might_be_encountered_ to false and never use
      // base_address_.
      coff_text_section_map_might_be_encountered_ = false;
    }
  }

  [[nodiscard]] const std::string& GetFilePath() const { return file_path_; }

  void MarkExecutableMapEncountered() {
    coff_text_section_map_might_be_encountered_ = false;
    cached_coff_file_ = nullptr;
  }

  [[nodiscard]] bool TryIfAnonExecMapIsCoffTextSection(uint64_t map_start, uint64_t map_end) {
    if (!coff_text_section_map_might_be_encountered_) {
      ORBIT_CHECK(cached_coff_file_ == nullptr);
      return false;
    }

    ORBIT_LOG("Trying if executable map at %#x-%#x belongs to \"%s\"", map_start, map_end,
              file_path_);
    std::string error_message = absl::StrFormat(
        "Executable map at %#x-%#x does NOT belong to \"%s\"", map_start, map_end, file_path_);

    if (cached_coff_file_ == nullptr) {
      // Don't even try to create an ObjectFile from character or block devices.
      if (absl::StartsWith(file_path_, "/dev/")) {
        ORBIT_LOG("%s", error_message);
        coff_text_section_map_might_be_encountered_ = false;
        return false;
      }

      auto object_file_or_error = CreateObjectFile(file_path_);
      if (object_file_or_error.has_error()) {
        ORBIT_LOG("%s", error_message);
        coff_text_section_map_might_be_encountered_ = false;
        return false;
      }

      // Remember: we are only detecting anonymous maps that correspond to .text sections of PEs,
      // because loadable sections of ELF files can always be file-mapped.
      if (!object_file_or_error.value()->IsCoff()) {
        ORBIT_LOG("%s", error_message);
        coff_text_section_map_might_be_encountered_ = false;
        return false;
      }

      cached_coff_file_ = std::move(object_file_or_error.value());
    }

    ORBIT_CHECK(cached_coff_file_ != nullptr);

    if (map_end <= base_address_ + cached_coff_file_->GetExecutableSegmentOffset()) {
      ORBIT_LOG("%s", error_message);
      // Don't set coff_text_section_map_might_be_encountered_ to false in this case, the entry we
      // are looking for could come later.
      return false;
    }

    // We validate that the executable map fully contains the address range at which the .text
    // section of the PE is supposed to be mapped. We consider the address at which the first byte
    // of this file is mapped (base_address_), and the address range of the .text section relative
    // to the image base when loaded into memory (determined by VirtualAddress and VirtualSize).
    if (map_start <= base_address_ + cached_coff_file_->GetExecutableSegmentOffset() &&
        map_end >= base_address_ + cached_coff_file_->GetExecutableSegmentOffset() +
                       cached_coff_file_->GetExecutableSegmentSize()) {
      ORBIT_LOG("Guessing that executable map at %#x-%#x belongs to \"%s\"", map_start, map_end,
                file_path_);
      MarkExecutableMapEncountered();
      return true;
    }

    ORBIT_LOG("%s", error_message);
    coff_text_section_map_might_be_encountered_ = false;
    cached_coff_file_ = nullptr;
    return false;
  }

 private:
  std::string file_path_;
  // The address at which the first byte of the file is (or would be) mapped.
  uint64_t base_address_;
  // False if not a PE, if the .text segment has already been found, or if we are already past the
  // address at which we could find the .text segment.
  bool coff_text_section_map_might_be_encountered_ = true;
  std::unique_ptr<ObjectFile> cached_coff_file_;
};
}  // namespace

ErrorMessageOr<std::vector<ModuleInfo>> ParseMaps(std::string_view proc_maps_data) {
  const std::vector<std::string> proc_maps = absl::StrSplit(proc_maps_data, '\n');

  std::vector<ModuleInfo> result;

  // This is used to detect mappings that correspond to the .text section of a PE but that are not
  // file-backed because the file alignment doesn't satisfy the requirements of mmap.
  std::optional<FileMappedIntoMemory> last_file_mapped_into_memory;

  for (const std::string& line : proc_maps) {
    // The number of spaces from the inode to the path is variable, and the path can contain spaces,
    // so we need to limit the number of splits and remove leading spaces from the path separately.
    std::vector<std::string> tokens = absl::StrSplit(line, absl::MaxSplits(' ', 5));
    if (tokens.size() < 5) continue;

    if (tokens.size() == 6) {
      absl::StripLeadingAsciiWhitespace(&tokens[5]);
      if (tokens[5].empty()) {
        tokens.pop_back();
      }
    }

    const std::string& inode = tokens[4];
    // If inode equals 0, then the memory is not backed by a file.
    // If a map not backed by a file has a name, it's a special one like [stack], [heap], etc.
    if (inode == "0" && tokens.size() == 6) continue;

    const std::vector<std::string> addresses = absl::StrSplit(tokens[0], '-');
    if (addresses.size() != 2) continue;

    const uint64_t start = std::stoull(addresses[0], nullptr, 16);
    const uint64_t end = std::stoull(addresses[1], nullptr, 16);
    const uint64_t offset = std::stoull(tokens[2], nullptr, 16);

    std::string module_path;
    if (inode != "0") {  // The mapping is file-backed.
      if (tokens.size() == 6) {
        module_path = tokens[5];
        // Keep track of the last file we encountered. Only create a new FileMappedIntoMemory if
        // this file mapping is backed by a different file than the previous file mapping.
        if (!last_file_mapped_into_memory.has_value() ||
            module_path != last_file_mapped_into_memory->GetFilePath()) {
          last_file_mapped_into_memory.emplace(module_path, start, offset);
        }
      } else {  // Unexpected: the mapping is file-backed but no path is present.
        ORBIT_ERROR("Map at %#x-%#x has inode %s (not 0) but no path", start, end, inode);
        last_file_mapped_into_memory.reset();
        continue;
      }
    }

    const bool is_executable = tokens[1].size() == 4 && tokens[1][2] == 'x';
    // Never create modules from non-executable mappings.
    if (!is_executable) continue;

    if (!module_path.empty()) {
      ORBIT_CHECK(last_file_mapped_into_memory.has_value());
      last_file_mapped_into_memory->MarkExecutableMapEncountered();
    } else if (last_file_mapped_into_memory.has_value() &&
               last_file_mapped_into_memory->TryIfAnonExecMapIsCoffTextSection(start, end)) {
      module_path = last_file_mapped_into_memory->GetFilePath();
    } else {
      continue;
    }

    ErrorMessageOr<ModuleInfo> module_info_or_error = CreateModule(module_path, start, end);

    if (module_info_or_error.has_error()) {
      ORBIT_ERROR("Unable to create module: %s", module_info_or_error.error().message());
      continue;
    }

    result.emplace_back(std::move(module_info_or_error.value()));
  }

  return result;
}

}  // namespace orbit_object_utils
