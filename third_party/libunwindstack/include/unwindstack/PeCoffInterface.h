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

#ifndef _LIBUNWINDSTACK_PE_COFF_INTERFACE_H
#define _LIBUNWINDSTACK_PE_COFF_INTERFACE_H

#include <vector>

#include <unwindstack/Error.h>
#include <unwindstack/Memory.h>

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

class PeCoffMemory {
 public:
  PeCoffMemory(Memory* memory) : memory_(memory), cur_offset_(0) {}

  bool Get8(uint8_t* value);
  bool Get16(uint16_t* value);
  bool Get32(uint32_t* value);
  bool Get64(uint64_t* value);
  bool GetMax64(uint64_t* value, uint64_t size);

  bool ReadFully(uint64_t addr, void* dst, size_t size) {
    return memory_->ReadFully(addr, dst, size);
  }

  uint64_t cur_offset() { return cur_offset_; }
  void set_cur_offset(uint64_t cur_offset) { cur_offset_ = cur_offset; }

 private:
  Memory* memory_;
  uint64_t cur_offset_;
};

template <typename AddressTypeArg>
class PeCoffInterface {
  static_assert(std::is_same_v<AddressTypeArg, uint32_t> ||
                    std::is_same_v<AddressTypeArg, uint64_t>,
                "AddressTypeArg must be an unsigned integer and either 4-bytes or 8-bytes");

 public:
  explicit PeCoffInterface(Memory* memory) : coff_memory_(memory) {}
  virtual ~PeCoffInterface() {}
  bool Init();
  ErrorData LastError() const { return last_error_; }

  using AddressType = AddressTypeArg;

 private:
  PeCoffMemory coff_memory_;
  DosHeader dos_header_;
  CoffHeader coff_header_;
  CoffOptionalHeader optional_header_;

  ErrorData last_error_{ERROR_NONE, 0};

  bool ParseDosHeader(uint64_t offset);
  bool ParseNewHeader(uint64_t offset);
  bool ParseCoffHeader(uint64_t offset);
  bool ParseOptionalHeader(uint64_t offset);

  bool ParseAllHeaders();
};

using PeCoffInterface32 = PeCoffInterface<uint32_t>;
using PeCoffInterface64 = PeCoffInterface<uint64_t>;

}  // namespace unwindstack

#endif  // _LIBUNWINDSTACK_PE_COFF_INTERFACE_H