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

#include <MemoryBuffer.h>
#include "utils/MemoryFake.h"

#include <android-base/file.h>
#include "Check.h"

namespace unwindstack {

template <typename PeCoffInterfaceType>
class PeCoffFake {
 public:
  // The following constants determine the basic layout and locations of the data
  // in the fake PE/COFF file.

  // This must be at 0x0.
  static constexpr uint64_t kDosHeaderOffset = 0x0;
  static constexpr size_t kDosHeaderSizeInBytes = 0x40;
  // File offset for the new header.
  static constexpr uint64_t kNewHeaderOffsetValue = 0xF8;

  // Size of the whole PE when loaded into memory.
  static constexpr uint64_t kSizeOfImage = 0x10000;

  // Offset and size in the file of the .text section.
  static constexpr uint64_t kTextSectionFileOffset = 0x400;
  static constexpr uint64_t kTextSectionFileSize = 0x1000;
  // Offset relative to the image base and size of the .text section when loaded into memory.
  static constexpr uint64_t kTextSectionMemoryOffset = 0x2000;
  static constexpr uint64_t kTextSectionMemorySize = 0xFFF;

  static constexpr uint32_t kTextSectionFlags = 0x20000020;

  // File offset for the .debug_frame section.
  static constexpr uint64_t kDebugFrameSectionFileOffset = 0x3000;

  static constexpr uint32_t kDebugFrameSectionFlags = 0x40000040;

  // File offset for the exception table, equivalent to the .pdata section, which consists
  // of a list of RUNTIME_FUNCTION entries.
  static constexpr uint64_t kExceptionTableFileOffset = 0x4000;
  // This is the number of bytes for the RUNTIME_FUNCTION entries, which needs to be
  // divisible by 12. This number here is 12 * 100 == 0x4b0.
  static constexpr uint64_t kExceptionTableSize = 0x4b0;

  // While this value determines the memory layout, our code only looks at the file content,
  // so this value is only used in arithmetic converting virtual addresses to file offset.
  static constexpr uint64_t kExceptionTableVmAddr = 0x5000;

  static constexpr uint32_t kPdataSectionFlags = 0x40000040;

  // Fake load bias, does not change the layout of the file data.
  static constexpr int64_t kLoadBiasFake = 0x31000;

  PeCoffFake() : memory_(new MemoryFake) {}
  ~PeCoffFake() = default;
  MemoryFake* GetMemoryFake() { return memory_.get(); }

  // Some tests require to take ownership over the memory object. Calls to set anything
  // on the memory, which will then be a nullptr, will segfault.
  MemoryFake* ReleaseMemoryFake() { return memory_.release(); };

  void Init();

  // Returns the offset where the section *would* go, so tests can add data there as desired.
  uint64_t InitNoSectionHeaders();
  uint64_t SetSectionHeaderAtOffset(uint64_t offset, const std::string& section_name,
                                    uint64_t vmsize = 0, uint64_t vmaddr = 0, uint64_t size = 0,
                                    uint64_t file_offset = 0, uint32_t flags = 0);

  uint64_t coff_header_nsects_offset() const { return coff_header_nsects_offset_; };
  uint64_t coff_header_symoff_offset() const { return coff_header_symoff_offset_; };
  uint64_t optional_header_size_offset() const { return optional_header_size_offset_; };
  uint64_t optional_header_start_offset() const { return optional_header_start_offset_; };
  uint64_t optional_header_num_data_dirs_offset() const {
    return optional_header_num_data_dirs_offset_;
  };

 private:
  uint64_t coff_header_nsects_offset_;
  uint64_t coff_header_symoff_offset_;
  uint64_t optional_header_size_offset_;
  uint64_t optional_header_start_offset_;
  uint64_t optional_header_num_data_dirs_offset_;
  std::unique_ptr<MemoryFake> memory_;

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
  uint64_t SetSectionHeadersAtOffset(uint64_t offset, uint32_t debug_frame_vmsize,
                                     uint32_t debug_frame_filesize);
  uint64_t SetDebugFrameEntryAtOffset(uint64_t offset, uint32_t pc_start);
  uint64_t SetRuntimeFunctionsAtOffset(uint64_t offset, uint64_t size);

  std::vector<std::pair<uint64_t, std::string>> section_names_in_string_table_;
};

}  // namespace unwindstack
