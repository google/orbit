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

#include <unwindstack/PeCoffInterface.h>

#include <cstring>
#include <limits>

#include <android-base/file.h>

#include <gtest/gtest.h>

#include <Check.h>
#include <MemoryBuffer.h>
#include "utils/MemoryFake.h"

#include <unwindstack/Log.h>

namespace unwindstack {

constexpr size_t kDosHeaderSizeInBytes = 0x40;
constexpr uint64_t kDebugFrameSectionFileOffset = 0x2000;
constexpr uint64_t kDebugFrameSectionSize = 0x200;
constexpr int64_t kLoadBiasFake = 0x3100;

// Needed for static_asserts that depend on the template parameter. Just a
// plain `false` in the static_assert will cause compilation to fail.
template <class>
inline constexpr bool kDependentFalse = false;

template <typename PeCoffInterfaceType>
class PeCoffInterfaceTest : public ::testing::Test {
 public:
  PeCoffInterfaceTest() : memory_(nullptr) {}
  ~PeCoffInterfaceTest() {}

  static std::unique_ptr<MemoryBuffer> ReadFile(const char* filename) {
    std::string dir = android::base::GetExecutableDirectory() + "/tests/files/";
    std::string data;
    EXPECT_TRUE(android::base::ReadFileToString(dir + filename, &data)) << filename;
    EXPECT_GT(data.size(), 0u);
    auto memory = std::make_unique<MemoryBuffer>();
    EXPECT_TRUE(memory->Resize(data.size()));
    std::memcpy(memory->GetPtr(0), data.data(), data.size());
    return memory;
  }

  void InitPeCoffInterfaceFromFile() {
    if constexpr (sizeof(typename PeCoffInterfaceType::AddressType) == 8) {
      memory_ = ReadFile("libtest.dll");
      expected_load_bias_in_file_ = 0x62640000;
    } else if constexpr (sizeof(typename PeCoffInterfaceType::AddressType) == 4) {
      memory_ = ReadFile("libtest32.dll");
      expected_load_bias_in_file_ = 0x67b40000;
    } else {
      static_assert(kDependentFalse<PeCoffInterfaceType>, "AddressType size must be 4 or 8 bytes");
    }
  }

  void InitPeCoffInterfaceFake() {
    memory_fake_.Clear();
    uint64_t offset = SetDosHeader(0x1000);
    offset = SetNewHeaderAtOffset(offset);
    offset = SetCoffHeaderAtOffset(offset);
    offset = SetOptionalHeaderAtOffset(offset);
    offset = SetSectionHeadersAtOffset(offset);

    SetDebugFrameSectionAtOffset(kDebugFrameSectionFileOffset);
  }

  uint64_t InitPeCoffInterfaceFakeNoSectionHeaders() {
    memory_fake_.Clear();
    uint64_t offset = SetDosHeader(0x1000);
    offset = SetNewHeaderAtOffset(offset);
    offset = SetCoffHeaderAtOffset(offset);
    offset = SetOptionalHeaderAtOffset(offset);
    return offset;
  }

  uint64_t SetData8(uint64_t offset, uint8_t value) {
    memory_fake_.SetData8(offset, value);
    return offset + sizeof(uint8_t);
  }
  uint64_t SetData16(uint64_t offset, uint16_t value) {
    memory_fake_.SetData16(offset, value);
    return offset + sizeof(uint16_t);
  }
  uint64_t SetData32(uint64_t offset, uint32_t value) {
    memory_fake_.SetData32(offset, value);
    return offset + sizeof(uint32_t);
  }
  uint64_t SetData64(uint64_t offset, uint64_t value) {
    memory_fake_.SetData64(offset, value);
    return offset + sizeof(uint64_t);
  }

  uint64_t SetMax64(uint64_t offset, uint64_t value, uint64_t size) {
    switch (size) {
      case 1:
        return SetData8(offset, static_cast<uint8_t>(value));
      case 2:
        return SetData16(offset, static_cast<uint16_t>(value));
      case 4:
        return SetData32(offset, static_cast<uint32_t>(value));
      case 8:
        return SetData64(offset, value);
      default:
        return offset;
    }
    return offset;
  }

  void SetDosHeaderMagicValue() {
    // This must be at offset 0.
    SetData16(0x0, 0x5a4d);
  }

  void SetDosHeaderOffsetToNewHeader(uint32_t offset_value) {
    // This must be at offset 0x3c.
    memory_fake_.SetMemory(0x3c, &offset_value, sizeof(uint32_t));
  }

  uint64_t SetDosHeader(uint32_t new_header_offset_value) {
    std::vector<uint8_t> zero_data(kDosHeaderSizeInBytes, 0);
    memory_fake_.SetMemory(0, zero_data);

    SetDosHeaderMagicValue();
    SetDosHeaderOffsetToNewHeader(new_header_offset_value);
    return new_header_offset_value;
  }

  uint64_t SetNewHeaderAtOffset(uint64_t offset) { return SetData32(offset, 0x00004550); }

  uint64_t SetCoffHeaderAtOffset(uint64_t offset) {
    offset = SetData16(offset, 0);  // machine

    // We remember the location of the number of sections here, so we can set it correctly later
    // when we initialize the sections.
    coff_header_nsects_offset_ = offset;
    offset = SetData16(offset, 0);  // nsects

    offset = SetData32(offset, 0);  // modtime

    coff_header_symoff_offset_ = offset;
    offset = SetData32(offset, 0);  // symoff

    offset = SetData32(offset, 0);  // nsyms

    // We remember the location of the header size (which is actually the size of the optional
    // header) here so that we can set it correctly later when we know the size of the optional
    // header (which depends on target address size and the number of directory entries).
    optional_header_size_offset_ = offset;
    offset = SetData16(offset, 0);  // hdrsize, to be set correctly later

    offset = SetData16(offset, 0);  // flags
    return offset;
  }

  uint64_t SetOptionalHeaderMagicPE32AtOffset(uint64_t offset) {
    constexpr uint16_t kOptionalHeaderMagicPE32 = 0x010b;
    return SetData16(offset, kOptionalHeaderMagicPE32);
  }

  uint64_t SetOptionalHeaderMagicPE32PlusAtOffset(uint64_t offset) {
    constexpr uint16_t kOptionalHeaderMagicPE32Plus = 0x020b;
    return SetData16(offset, kOptionalHeaderMagicPE32Plus);
  }

  uint64_t SetOptionalHeaderAtOffset(uint64_t offset) {
    optional_header_start_offset_ = offset;

    if constexpr (sizeof(typename PeCoffInterfaceType::AddressType) == 4) {
      offset = SetOptionalHeaderMagicPE32AtOffset(offset);
    } else if constexpr (sizeof(typename PeCoffInterfaceType::AddressType) == 8) {
      offset = SetOptionalHeaderMagicPE32PlusAtOffset(offset);
    } else {
      static_assert(kDependentFalse<PeCoffInterfaceType>, "AddressType size must be 4 or 8 bytes");
    }
    offset = SetData8(offset, 0);   // major_linker_version
    offset = SetData8(offset, 0);   // minor_linker_version
    offset = SetData32(offset, 0);  // code_size
    offset = SetData32(offset, 0);  // data_size
    offset = SetData32(offset, 0);  // bss_size
    offset = SetData32(offset, 0);  // entry
    offset = SetData32(offset, 0);  // code_offset

    if constexpr (sizeof(typename PeCoffInterfaceType::AddressType) == 4) {
      // Data offset only exists in 32-bit PE/COFF.
      offset = SetData32(offset, 0);
    }

    // image_base
    offset = SetMax64(offset, kLoadBiasFake, sizeof(typename PeCoffInterfaceType::AddressType));

    offset = SetData32(offset, 0x1000);  // sect_alignment
    offset = SetData32(offset, 0x200);   // file_alignment
    offset = SetData16(offset, 0);       // major_os_system_version
    offset = SetData16(offset, 0);       // minor_os_system_version
    offset = SetData16(offset, 0);       // major_image_version
    offset = SetData16(offset, 0);       // minor_image_version
    offset = SetData16(offset, 0);       // major_subsystem_version
    offset = SetData16(offset, 0);       // minor_subsystem_version
    offset = SetData32(offset, 0);       // reserved1
    offset = SetData32(offset, 0);       // image_size
    offset = SetData32(offset, 0);       // header_size
    offset = SetData32(offset, 0);       // checksum
    offset = SetData16(offset, 0);       // subsystem
    offset = SetData16(offset, 0);       // dll_flags

    // stack_reserve_size
    offset = SetMax64(offset, 0, sizeof(typename PeCoffInterfaceType::AddressType));
    // stack_commit_size
    offset = SetMax64(offset, 0, sizeof(typename PeCoffInterfaceType::AddressType));
    // heap_reserve_size
    offset = SetMax64(offset, 0, sizeof(typename PeCoffInterfaceType::AddressType));
    // heap_commit_size
    offset = SetMax64(offset, 0, sizeof(typename PeCoffInterfaceType::AddressType));

    offset = SetData32(offset, 0);  // loader_flags

    optional_header_num_data_dirs_offset_ = offset;
    constexpr uint32_t kNumDirDataEntries = 10;
    offset = SetData32(offset, kNumDirDataEntries);  // num_dir_data_entries

    for (uint32_t i = 0; i < kNumDirDataEntries; ++i) {
      offset = SetData32(offset, 0);
      offset = SetData32(offset, 0);
    }

    SetData16(optional_header_size_offset_, offset - optional_header_start_offset_);

    return offset;
  }

  uint64_t SetSectionHeaderAtOffset(uint64_t offset, std::string section_name, uint64_t vmsize = 0,
                                    uint64_t vmaddr = 0, uint64_t size = 0,
                                    uint64_t file_offset = 0) {
    std::string name_in_header = section_name;
    if (section_name.size() > kSectionNameInHeaderSize) {
      const uint64_t previous_offset =
          section_names_in_string_table_.empty() ? 0 : section_names_in_string_table_.back().first;
      const uint64_t previous_size = section_names_in_string_table_.empty()
                                         ? 0
                                         : section_names_in_string_table_.back().second.size() + 1;
      const uint64_t current_offset = previous_offset + previous_size;
      name_in_header = std::string("/") + std::to_string(current_offset);
      section_names_in_string_table_.emplace_back(current_offset, section_name);
    }

    std::vector<uint8_t> zeros(kSectionNameInHeaderSize, 0);
    memory_fake_.SetMemory(offset, zeros.data(), zeros.size());
    memory_fake_.SetMemory(offset, name_in_header);
    offset += kSectionNameInHeaderSize;
    offset = SetData32(offset, vmsize);
    offset = SetData32(offset, vmaddr);
    offset = SetData32(offset, size);
    offset = SetData32(offset, file_offset);
    offset = SetData32(offset, 0);  // reloff
    offset = SetData32(offset, 0);  // lineoff
    offset = SetData16(offset, 0);  // nrel
    offset = SetData16(offset, 0);  // nline
    offset = SetData32(offset, 0);  // flagsd
    return offset;
  };

  uint64_t SetSectionStringsAtOffset(uint64_t offset) {
    const uint64_t string_table_base_offset = offset;
    for (const auto& [string_table_offset, name] : section_names_in_string_table_) {
      memory_fake_.SetMemory(string_table_base_offset + string_table_offset, name);

      // Strings written to memory are null-terminated, so we need to add "1" to the size.
      offset += name.size() + 1;
    }
    return offset;
  }

  uint64_t SetSectionHeadersAtOffset(uint64_t offset) {
    // Shorter than kSectionNameInHeaderSize (== 8) characters
    offset = SetSectionHeaderAtOffset(offset, ".text", 0, 0, 0, 0);
    // Longer than kSectionNameInHeaderSize (== 8) characters
    offset = SetSectionHeaderAtOffset(offset, ".debug_frame", kDebugFrameSectionSize,
                                      kDebugFrameSectionFileOffset, kDebugFrameSectionSize,
                                      kDebugFrameSectionFileOffset);
    SetData16(coff_header_nsects_offset_, 2);

    CHECK(offset <= std::numeric_limits<uint32_t>::max());
    const uint32_t actual_symoff = static_cast<uint32_t>(offset);
    SetData32(coff_header_symoff_offset_, actual_symoff);

    offset = SetSectionStringsAtOffset(offset);

    return offset;
  }

  uint64_t SetDebugFrameSectionAtOffset(uint64_t offset) {
    uint64_t initial_offset = offset;
    // CIE 32
    offset = SetData32(offset, 0xfc);        // length
    offset = SetData32(offset, 0xffffffff);  // CIE_id
    memory_fake_.SetMemory(offset, std::vector<uint8_t>{1, '\0', 0, 0, 1});

    // FDE 32
    offset = initial_offset + 0x100;
    offset = SetData32(offset, 0xfc);
    offset = SetData32(offset, 0);
    offset = SetData32(offset, 0x2100);
    offset = SetData32(offset, 0x400);
    return offset;
  }

  std::unique_ptr<Memory> memory_;
  MemoryFake memory_fake_;

  uint64_t coff_header_nsects_offset_;
  uint64_t coff_header_symoff_offset_;

  uint64_t optional_header_size_offset_;
  uint64_t optional_header_start_offset_;

  uint64_t optional_header_num_data_dirs_offset_;

  int64_t expected_load_bias_in_file_;

  std::vector<std::pair<uint64_t, std::string>> section_names_in_string_table_;
};

// Many tests equally apply to the 32-bit and the 64-bit case, so we implement these
// as TYPED_TESTs.
typedef ::testing::Types<PeCoffInterface32, PeCoffInterface64> Implementations;
TYPED_TEST_SUITE(PeCoffInterfaceTest, Implementations);

TYPED_TEST(PeCoffInterfaceTest, init_for_coff_file) {
  this->InitPeCoffInterfaceFromFile();
  TypeParam coff(this->memory_.get());
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));
  EXPECT_EQ(load_bias, this->expected_load_bias_in_file_);
}

TYPED_TEST(PeCoffInterfaceTest, init_for_coff_file_fake) {
  this->InitPeCoffInterfaceFake();
  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));
  EXPECT_EQ(load_bias, kLoadBiasFake);
}

TYPED_TEST(PeCoffInterfaceTest, dos_header_parsing_fails_empty_memory) {
  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, dos_header_parsing_fails_invalid_memory_at_unused_data) {
  this->InitPeCoffInterfaceFake();
  this->memory_fake_.ClearMemory(0x30, 1);
  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, dos_header_parsing_fails_invalid_memory_at_new_header_offset) {
  this->InitPeCoffInterfaceFake();
  this->memory_fake_.ClearMemory(0x3c, 1);
  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, dos_header_parsing_fails_wrong_magic_number) {
  this->InitPeCoffInterfaceFake();
  // Correct magic number is 0x5a4d
  this->memory_fake_.SetData16(0, 0x5a4c);
  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, new_header_parsing_fails_invalid_memory) {
  this->InitPeCoffInterfaceFake();
  this->memory_fake_.ClearMemory(0x1000, 1);
  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, new_header_parsing_fails_wrong_pe_signature) {
  this->InitPeCoffInterfaceFake();
  // Correct PE signature is 0x00004550
  this->SetData32(0x1000, 0x00004551);
  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, coff_header_parsing_fails_invalid_memory) {
  this->InitPeCoffInterfaceFake();
  // COFF headers starts 4 bytes after the new header, which we have set at 0x1000.
  constexpr uint16_t kCoffHeaderStart = 0x1004;
  this->memory_fake_.ClearMemory(kCoffHeaderStart, 1);
  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, optional_header_parsing_fails_wrong_magic_number) {
  this->InitPeCoffInterfaceFake();
  // 0x010b would be a correct choice
  this->SetData16(this->optional_header_start_offset_, 0x010a);
  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, optional_header_parsing_fails_invalid_memory_start) {
  this->InitPeCoffInterfaceFake();

  this->memory_fake_.ClearMemory(this->optional_header_start_offset_, 1);

  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, optional_header_parsing_fails_invalid_memory_end) {
  this->InitPeCoffInterfaceFake();

  this->memory_fake_.ClearMemory(this->optional_header_start_offset_, 1);

  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, optional_header_parsing_fails_incorrect_header_size) {
  this->InitPeCoffInterfaceFake();

  uint16_t correct_header_size;
  this->memory_fake_.Read16(this->optional_header_size_offset_, &correct_header_size);
  this->SetData16(this->optional_header_size_offset_, correct_header_size + 1);

  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, optional_header_parsing_fails_incorrect_num_data_dir_entries) {
  this->InitPeCoffInterfaceFake();

  uint32_t correct_num;
  this->memory_fake_.Read32(this->optional_header_num_data_dirs_offset_, &correct_num);
  this->SetData32(this->optional_header_num_data_dirs_offset_, correct_num + 1);

  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, section_headers_parsing_fails_invalid_memory) {
  this->InitPeCoffInterfaceFakeNoSectionHeaders();
  this->SetData16(this->coff_header_nsects_offset_, 1);

  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest,
           section_headers_parsing_fails_invalid_memory_middle_of_section_header) {
  const uint64_t section_headers_offset = this->InitPeCoffInterfaceFakeNoSectionHeaders();
  this->SetSectionHeaderAtOffset(section_headers_offset, ".text");
  this->SetData16(this->coff_header_nsects_offset_, 1);
  // We want to catch the second failure case, which is after the initial section name string of
  // length kSectionNameInHeaderSize
  this->memory_fake_.ClearMemory(section_headers_offset + kSectionNameInHeaderSize, 1);

  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest,
           section_headers_parsing_fails_section_string_name_offset_not_an_integer) {
  const uint64_t section_headers_offset = this->InitPeCoffInterfaceFakeNoSectionHeaders();
  this->SetSectionHeaderAtOffset(section_headers_offset, "/abc");
  this->SetData16(this->coff_header_nsects_offset_, 1);

  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, section_headers_parsing_fails_missing_string_table) {
  uint64_t offset = this->InitPeCoffInterfaceFakeNoSectionHeaders();
  // The "/0" indicates that the section name has to be read at offset 0 in the string table,
  // however for this test, the string table is not set up at all, so it must fail.
  offset = this->SetSectionHeaderAtOffset(offset, "/0");
  this->SetData32(this->coff_header_symoff_offset_, offset);
  this->SetData16(this->coff_header_nsects_offset_, 1);

  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, section_headers_parsing_fails_no_text_section) {
  uint64_t offset = this->InitPeCoffInterfaceFakeNoSectionHeaders();
  offset = this->SetSectionHeaderAtOffset(offset, ".no_text");
  this->SetData16(this->coff_header_nsects_offset_, 1);

  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_INVALID_COFF, coff.LastError().code);
}

TYPED_TEST(PeCoffInterfaceTest, debug_frame_section_parsed_correctly) {
  this->InitPeCoffInterfaceFake();
  TypeParam coff(&this->memory_fake_);
  int64_t load_bias;
  ASSERT_TRUE(coff.Init(&load_bias));

  const DwarfFde* dwarf_fde = coff.DebugFrameSection()->GetFdeFromPc(0x2100);
  ASSERT_NE(dwarf_fde, nullptr);
  EXPECT_EQ(0x2100, dwarf_fde->pc_start);
  EXPECT_EQ(0x2500, dwarf_fde->pc_end);
}

// The remaining tests are not TYPED_TESTs, because they are specific to either the 32-bit or 64-bit
// version of the PE/COFF interface class, such as testing if a missing data offset for the 32-bit
// instance (this data offset does not exist in the 64-bit case), or initializing a 32-bit instance
// with a 64-bit PE/COFF file cause `Init()` to fail.

using PeCoffInterface32Test = PeCoffInterfaceTest<PeCoffInterface32>;
using PeCoffInterface64Test = PeCoffInterfaceTest<PeCoffInterface64>;

TEST_F(PeCoffInterface32Test, init_64_fails_for_coff_32_fake) {
  InitPeCoffInterfaceFake();
  PeCoffInterface64 coff(&memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_UNSUPPORTED, coff.LastError().code);
}

TEST_F(PeCoffInterface64Test, init_32_fails_for_coff_64_fake) {
  InitPeCoffInterfaceFake();
  PeCoffInterface32 coff(&memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_UNSUPPORTED, coff.LastError().code);
}

TEST_F(PeCoffInterface32Test, optional_header_parsing_fails_invalid_memory_at_data_offset_32_only) {
  InitPeCoffInterfaceFake();
  const uint64_t kDataOffsetAddress = optional_header_start_offset_ + 0x0018;
  this->memory_fake_.ClearMemory(kDataOffsetAddress, 1);

  PeCoffInterface32 coff(&memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TEST_F(PeCoffInterface32Test, optional_header_parsing_fails_invalid_memory_end_32) {
  InitPeCoffInterfaceFake();
  const uint64_t kAfterDataOffset = optional_header_start_offset_ + 0x0018 + 0x0004;
  this->memory_fake_.ClearMemory(kAfterDataOffset, 1);

  PeCoffInterface32 coff(&memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TEST_F(PeCoffInterface64Test, optional_header_parsing_fails_invalid_memory_end_64) {
  InitPeCoffInterfaceFake();
  const uint64_t kAfterDataOffset = optional_header_start_offset_ + 0x0018;
  this->memory_fake_.ClearMemory(kAfterDataOffset, 1);

  PeCoffInterface64 coff(&memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TEST_F(PeCoffInterface32Test, optional_header_parsing_fails_invalid_memory_data_dirs_32) {
  InitPeCoffInterfaceFake();
  const uint64_t kDataDirOffset = optional_header_start_offset_ + 0x0018 + 0x0004 + 0x0044;
  this->memory_fake_.ClearMemory(kDataDirOffset, 1);

  PeCoffInterface32 coff(&memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

TEST_F(PeCoffInterface64Test, optional_header_parsing_fails_invalid_memory_data_dirs_64) {
  InitPeCoffInterfaceFake();
  const uint64_t kDataDirOffset = optional_header_start_offset_ + 0x0018 + 0x0058;
  this->memory_fake_.ClearMemory(kDataDirOffset, 1);

  PeCoffInterface64 coff(&memory_fake_);
  int64_t load_bias;
  ASSERT_FALSE(coff.Init(&load_bias));
  EXPECT_EQ(ERROR_MEMORY_INVALID, coff.LastError().code);
}

}  // namespace unwindstack