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

#ifndef _LIBUNWINDSTACK_PE_COFF_FAKE_H
#define _LIBUNWINDSTACK_PE_COFF_FAKE_H

#include <MemoryBuffer.h>
#include "utils/MemoryFake.h"

#include <android-base/file.h>
#include "Check.h"

namespace unwindstack {

template <typename PeCoffInterfaceType>
class PeCoffFake {
 public:
  static constexpr size_t kDosHeaderSizeInBytes = 0x40;
  static constexpr uint64_t kDebugFrameSectionFileOffset = 0x2000;
  static constexpr uint64_t kDebugFrameSectionSize = 0x200;
  static constexpr int64_t kLoadBiasFake = 0x3100;
  static constexpr uint64_t kTextSectionOffsetFake = 0x200;

  PeCoffFake() : memory_(new MemoryFake) {}
  ~PeCoffFake() = default;
  MemoryFake* GetMemoryFake() { return memory_.get(); }

  // Some tests require to take ownership over the memory object. Calls to set anything
  // on the memory, which will then be a nullptr, will segfault.
  MemoryFake* ReleaseMemoryFake() { return memory_.release(); };

  void Init();

  // Returns the offset where the section *would* go, so tests can add data there as desired.
  uint64_t InitNoSectionHeaders();
  uint64_t SetSectionHeaderAtOffset(uint64_t offset, std::string section_name, uint64_t vmsize = 0,
                                    uint64_t vmaddr = 0, uint64_t size = 0,
                                    uint64_t file_offset = 0);

  uint64_t coff_header_nsects_offset() const { return coff_header_nsects_offset_; };
  uint64_t coff_header_symoff_offset() const { return coff_header_symoff_offset_; };
  uint64_t optional_header_size_offset() const { return optional_header_size_offset_; };
  uint64_t optional_header_start_offset() const { return optional_header_start_offset_; };
  uint64_t optional_header_num_data_dirs_offset() const {
    return optional_header_num_data_dirs_offset_;
  };
  uint64_t executable_section_offset() const { return executable_section_offset_; };

 private:
  uint64_t coff_header_nsects_offset_;
  uint64_t coff_header_symoff_offset_;
  uint64_t optional_header_size_offset_;
  uint64_t optional_header_start_offset_;
  uint64_t optional_header_num_data_dirs_offset_;
  uint64_t executable_section_offset_;
  std::unique_ptr<MemoryFake> memory_;

  uint64_t InitPeCoffInterfaceFakeNoSectionHeaders();
  uint64_t SetData8(uint64_t offset, uint8_t value);
  uint64_t SetData16(uint64_t offset, uint16_t value);
  uint64_t SetData32(uint64_t offset, uint32_t value);
  uint64_t SetData64(uint64_t offset, uint64_t value);
  uint64_t SetMax64(uint64_t offset, uint64_t value, uint64_t size);
  void SetDosHeaderMagicValue();
  void SetDosHeaderOffsetToNewHeader(uint32_t offset_value);
  uint64_t SetDosHeader(uint32_t new_header_offset_value);
  uint64_t SetNewHeaderAtOffset(uint64_t offset);
  uint64_t SetCoffHeaderAtOffset(uint64_t offset);
  uint64_t SetOptionalHeaderMagicPE32AtOffset(uint64_t offset);
  uint64_t SetOptionalHeaderMagicPE32PlusAtOffset(uint64_t offset);
  uint64_t SetOptionalHeaderAtOffset(uint64_t offset);
  uint64_t SetSectionStringsAtOffset(uint64_t offset);
  uint64_t SetSectionHeadersAtOffset(uint64_t offset);
  uint64_t SetDebugFrameSectionAtOffset(uint64_t offset);

  std::unique_ptr<MemoryFake> memory_fake_;
  std::vector<std::pair<uint64_t, std::string>> section_names_in_string_table_;
};

}  // namespace unwindstack

#endif  // _LIBUNWINDSTACK_PE_COFF_FAKE_H