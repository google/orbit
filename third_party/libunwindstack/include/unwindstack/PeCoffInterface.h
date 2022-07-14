/*
 * Copyright (C) 2021 The Android Open Source Project
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

#pragma once

#include <array>
#include <optional>
#include <vector>

#include <unwindstack/DwarfSection.h>
#include <unwindstack/Error.h>
#include <unwindstack/Memory.h>
#include <unwindstack/PeCoffNativeUnwinder.h>

namespace unwindstack {

struct DosHeader {
  uint16_t e_magic;  // MS-DOS 2.0 magic value
  // 29 x uint16_t values that we don't (yet) need.
  uint32_t e_lfanew;  // File offset to new exe header.
};

struct CoffHeader {
  uint16_t machine;
  uint16_t nsects;
  uint32_t modtime;
  uint32_t symoff;
  uint32_t nsyms;
  uint16_t hdrsize;
  uint16_t flags;
};

struct DataDirectory {
  uint32_t vm_addr;
  uint32_t vm_size;
};

// Note: The "optional" header is actually required in PE/COFF.
struct CoffOptionalHeader {
  uint16_t magic = 0;
  uint8_t major_linker_version = 0;
  uint8_t minor_linker_version = 0;
  uint32_t code_size = 0;
  uint32_t data_size = 0;
  uint32_t bss_size = 0;
  uint32_t entry = 0;
  uint32_t code_offset = 0;
  uint32_t data_offset = 0;
  uint64_t image_base = 0;
  uint32_t sect_alignment = 0;
  uint32_t file_alignment = 0;
  uint16_t major_os_system_version = 0;
  uint16_t minor_os_system_version = 0;
  uint16_t major_image_version = 0;
  uint16_t minor_image_version = 0;
  uint16_t major_subsystem_version = 0;
  uint16_t minor_subsystem_version = 0;
  uint32_t reserved1 = 0;
  uint32_t image_size = 0;
  uint32_t header_size = 0;
  uint32_t checksum = 0;
  uint16_t subsystem = 0;
  uint16_t dll_flags = 0;
  uint64_t stack_reserve_size = 0;
  uint64_t stack_commit_size = 0;
  uint64_t heap_reserve_size = 0;
  uint64_t heap_commit_size = 0;
  uint32_t loader_flags = 0;
  uint32_t num_data_dir_entries;
  std::vector<DataDirectory> data_dirs;  // will contain `num_data_dir_entries` entries
};

constexpr size_t kSectionNameInHeaderSize = 8;

// Represents data precisely as parsed from the file.
struct SectionHeader {
  std::array<char, kSectionNameInHeaderSize> name;
  uint32_t vmsize;
  uint32_t vmaddr;
  uint32_t size;
  uint32_t offset;
  uint32_t reloff;
  uint32_t lineoff;
  uint16_t nrel;
  uint16_t nline;
  uint32_t flags;
};

// Represents preprocessed data of a section that we need for further processing. Section name
// is already looked up in the string table.
struct Section {
  Section() : name(""), vmsize(0), vmaddr(0), size(0), offset(0), flags(0) {}
  Section(std::string name_arg, uint32_t vmsize_arg, uint32_t vmaddr_arg, uint32_t size_arg,
          uint32_t offset_arg, uint32_t flags_arg)
      : name(std::move(name_arg)),
        vmsize(vmsize_arg),
        vmaddr(vmaddr_arg),
        size(size_arg),
        offset(offset_arg),
        flags(flags_arg) {}
  std::string name;
  uint32_t vmsize;
  uint32_t vmaddr;
  uint32_t size;    // Size in file
  uint32_t offset;  // Offset in file
  uint32_t flags;
};

class PeCoffMemory {
 public:
  explicit PeCoffMemory(Memory* memory) : memory_(memory), cur_offset_(0) {}

  bool Get8(uint8_t* value);
  bool Get16(uint16_t* value);
  bool Get32(uint32_t* value);
  bool Get64(uint64_t* value);
  bool GetMax64(uint64_t* value, uint64_t size);
  bool GetFully(void* dst, size_t size);

  bool ReadString(uint64_t offset, std::string* dst, uint64_t max_read) {
    return memory_->ReadString(offset, dst, max_read);
  }

  uint64_t cur_offset() { return cur_offset_; }
  void set_cur_offset(uint64_t cur_offset) { cur_offset_ = cur_offset; }

 private:
  Memory* memory_;
  uint64_t cur_offset_;
};

class PeCoffInterface {
 public:
  virtual ~PeCoffInterface() = default;

  virtual bool Init(int64_t* load_bias) = 0;
  virtual const ErrorData& last_error() = 0;
  virtual ErrorCode LastErrorCode() = 0;
  virtual uint64_t LastErrorAddress() = 0;
  virtual DwarfSection* DebugFrameSection() = 0;
  virtual uint64_t GetRelPcWithMapOffset(uint64_t pc, uint64_t map_start,
                                         uint64_t map_object_offset) = 0;
  virtual uint64_t GetRelPcWithMapRva(uint64_t pc, uint64_t map_start, uint64_t map_object_rva) = 0;
  virtual bool GetTextRange(uint64_t* addr, uint64_t* size) const = 0;
  virtual uint64_t GetTextOffsetInFile() const = 0;
  virtual uint64_t GetSizeOfImage() const = 0;
  virtual bool Step(uint64_t rel_pc, uint64_t pc_adjustment, Regs* regs, Memory* process_memory,
                    bool* finished, bool* is_signal_frame) = 0;
};

template <typename AddressTypeArg>
class PeCoffInterfaceImpl : public PeCoffInterface {
  static_assert(std::is_same_v<AddressTypeArg, uint32_t> ||
                    std::is_same_v<AddressTypeArg, uint64_t>,
                "AddressTypeArg must be an unsigned integer and either 4-bytes or 8-bytes");

 public:
  explicit PeCoffInterfaceImpl(Memory* memory) : memory_(memory), coff_memory_(memory) {}
  ~PeCoffInterfaceImpl() override = default;
  bool Init(int64_t* load_bias) override;

  const ErrorData& last_error() override { return last_error_; }
  ErrorCode LastErrorCode() override { return last_error_.code; }
  uint64_t LastErrorAddress() override { return last_error_.address; }

  DwarfSection* DebugFrameSection() override { return debug_frame_.get(); }
  uint64_t GetRelPcWithMapOffset(uint64_t pc, uint64_t map_start,
                                 uint64_t map_object_offset) override;
  uint64_t GetRelPcWithMapRva(uint64_t pc, uint64_t map_start, uint64_t map_object_rva) override;
  bool GetTextRange(uint64_t* addr, uint64_t* size) const override;
  uint64_t GetTextOffsetInFile() const override;
  uint64_t GetSizeOfImage() const override;
  bool Step(uint64_t rel_pc, uint64_t pc_adjustment, Regs* regs, Memory* process_memory,
            bool* finished, bool* is_signal_frame) override;

  using AddressType = AddressTypeArg;

 protected:
  Memory* memory_;
  PeCoffMemory coff_memory_;

  // Parsed data
  DosHeader dos_header_;
  CoffHeader coff_header_;
  CoffOptionalHeader optional_header_;
  std::vector<SectionHeader> parsed_section_headers_;

  // Initialized section data
  std::vector<Section> sections_;

  // Data about the .text section. Assumption: There is only a single .text section.
  struct TextSectionData {
    uint64_t memory_size = 0;
    uint64_t memory_offset = 0;
    uint64_t file_offset = 0;
  };
  std::optional<TextSectionData> text_section_data_;

  // If a .debug_frame section is present, these are initialized.
  struct DebugFrameSectionData {
    uint64_t file_offset = 0;
    uint64_t size = 0;
    int64_t section_bias = 0;
  };
  std::unique_ptr<DwarfSection> debug_frame_;
  std::optional<DebugFrameSectionData> debug_frame_section_data_;

  std::unique_ptr<PeCoffNativeUnwinder> native_unwinder_;

  ErrorData last_error_{ERROR_NONE, 0};

  bool ParseDosHeader(uint64_t offset);
  bool ParseNewHeader(uint64_t offset);
  bool ParseCoffHeader(uint64_t offset);
  bool ParseOptionalHeader(uint64_t offset);
  bool ParseSectionHeaders(uint64_t offset);

  bool ParseAllHeaders();

  bool GetSectionName(const std::string& parsed_section_name_string, std::string* result);
  bool InitSections();
  bool InitDebugFrameSection();
  bool MapFromRvaToFileOffset(uint64_t rva, uint64_t* file_offset);
  bool MapFromFileOffsetToRva(uint64_t file_offset, uint64_t* rva);
  bool InitNativeUnwinder();
};

using PeCoffInterface32 = PeCoffInterfaceImpl<uint32_t>;
using PeCoffInterface64 = PeCoffInterfaceImpl<uint64_t>;

}  // namespace unwindstack
