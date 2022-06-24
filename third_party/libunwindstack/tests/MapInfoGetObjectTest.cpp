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

#include <elf.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include <android-base/file.h>
#include <gtest/gtest.h>

#include <unwindstack/Elf.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Object.h>
#include <unwindstack/PeCoff.h>
#include <unwindstack/PeCoffInterface.h>

#include "ElfTestUtils.h"
#include "utils/MemoryFake.h"

namespace unwindstack {

class MapInfoGetObjectTest : public ::testing::Test {
 protected:
  void SetUp() override {
    memory_ = new MemoryFake;
    process_memory_.reset(memory_);
  }

  template <typename Ehdr, typename Shdr>
  static void InitElf(uint64_t sh_offset, Ehdr* ehdr, uint8_t class_type, uint8_t machine_type) {
    memset(ehdr, 0, sizeof(*ehdr));
    memcpy(ehdr->e_ident, ELFMAG, SELFMAG);
    ehdr->e_ident[EI_CLASS] = class_type;
    ehdr->e_machine = machine_type;
    ehdr->e_shoff = sh_offset;
    ehdr->e_shentsize = sizeof(Shdr) + 100;
    ehdr->e_shnum = 4;
  }

  void InitMapInfo(std::vector<std::shared_ptr<MapInfo>>& maps, bool in_memory);

  const size_t kMapSize = 4096;

  std::shared_ptr<Memory> process_memory_;
  MemoryFake* memory_;

  TemporaryFile elf_;
};

TEST_F(MapInfoGetObjectTest, invalid) {
  auto info = MapInfo::Create(0x1000, 0x2000, 0, PROT_READ, "");

  // The map is empty, but this should still create an invalid elf object.
  Object* object = info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_FALSE(object->valid());
}

TEST_F(MapInfoGetObjectTest, valid32) {
  Elf32_Ehdr ehdr;
  TestInitEhdr<Elf32_Ehdr>(&ehdr, ELFCLASS32, EM_ARM);
  memory_->SetMemory(0x3000, &ehdr, sizeof(ehdr));

  auto info = MapInfo::Create(0x3000, 0x4000, 0, PROT_READ, "");
  Object* object = info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());

  // The tested fields (machine type, class type) are ELF specific, so we have
  // to cast to an Elf object here.
  Elf* elf = dynamic_cast<Elf*>(object);
  ASSERT_TRUE(elf != nullptr);
  EXPECT_EQ(static_cast<uint32_t>(EM_ARM), elf->machine_type());
  EXPECT_EQ(ELFCLASS32, elf->class_type());

  // Now verify that an empty process memory returns an invalid object instance.
  info->set_object(nullptr);
  object = info->GetObject(std::shared_ptr<Memory>(), ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_FALSE(object->valid());
}

TEST_F(MapInfoGetObjectTest, valid64) {
  Elf64_Ehdr ehdr;
  TestInitEhdr<Elf64_Ehdr>(&ehdr, ELFCLASS64, EM_AARCH64);
  memory_->SetMemory(0x8000, &ehdr, sizeof(ehdr));

  auto info = MapInfo::Create(0x8000, 0x9000, 0, PROT_READ, "");
  Object* object = info->GetObject(process_memory_, ARCH_ARM64);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());

  // The tested fields (machine type, class type) are ELF specific, so we have
  // to cast to an Elf object here.
  Elf* elf = dynamic_cast<Elf*>(object);
  EXPECT_EQ(static_cast<uint32_t>(EM_AARCH64), elf->machine_type());
  EXPECT_EQ(ELFCLASS64, elf->class_type());
}

TEST_F(MapInfoGetObjectTest, invalid_arch_mismatch) {
  Elf32_Ehdr ehdr;
  TestInitEhdr<Elf32_Ehdr>(&ehdr, ELFCLASS32, EM_ARM);
  memory_->SetMemory(0x3000, &ehdr, sizeof(ehdr));

  auto info = MapInfo::Create(0x3000, 0x4000, 0, PROT_READ, "");
  Object* object = info->GetObject(process_memory_, ARCH_X86);
  ASSERT_TRUE(object != nullptr);
  ASSERT_FALSE(object->valid());
}

TEST_F(MapInfoGetObjectTest, gnu_debugdata_init32) {
  TestInitGnuDebugdata<Elf32_Ehdr, Elf32_Shdr>(ELFCLASS32, EM_ARM, true,
                                               [&](uint64_t offset, const void* ptr, size_t size) {
                                                 memory_->SetMemory(0x2000 + offset, ptr, size);
                                               });

  auto info = MapInfo::Create(0x2000, 0x3000, 0, PROT_READ, "");
  Object* object = info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());

  // The tested fields (machine type, class type) are ELF specific, so we have
  // to cast to an Elf object here.
  Elf* elf = dynamic_cast<Elf*>(object);
  EXPECT_EQ(static_cast<uint32_t>(EM_ARM), elf->machine_type());
  EXPECT_EQ(ELFCLASS32, elf->class_type());
  EXPECT_TRUE(elf->gnu_debugdata_interface() != nullptr);
}

TEST_F(MapInfoGetObjectTest, gnu_debugdata_init64) {
  TestInitGnuDebugdata<Elf64_Ehdr, Elf64_Shdr>(ELFCLASS64, EM_AARCH64, true,
                                               [&](uint64_t offset, const void* ptr, size_t size) {
                                                 memory_->SetMemory(0x5000 + offset, ptr, size);
                                               });

  auto info = MapInfo::Create(0x5000, 0x8000, 0, PROT_READ, "");
  Object* object = info->GetObject(process_memory_, ARCH_ARM64);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());

  // The tested fields (machine type, class type) are ELF specific, so we have
  // to cast to an Elf object here.
  Elf* elf = dynamic_cast<Elf*>(object);
  EXPECT_EQ(static_cast<uint32_t>(EM_AARCH64), elf->machine_type());
  EXPECT_EQ(ELFCLASS64, elf->class_type());
  EXPECT_TRUE(elf->gnu_debugdata_interface() != nullptr);
}

TEST_F(MapInfoGetObjectTest, end_le_start) {
  Elf32_Ehdr ehdr;
  TestInitEhdr<Elf32_Ehdr>(&ehdr, ELFCLASS32, EM_ARM);
  ASSERT_TRUE(android::base::WriteFully(elf_.fd, &ehdr, sizeof(ehdr)));

  auto info = MapInfo::Create(0x1000, 0x1000, 0, PROT_READ, elf_.path);
  Object* object = info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_FALSE(object->valid());

  info->set_object(nullptr);
  info->set_end(0xfff);
  object = info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_FALSE(object->valid());

  // Make sure this test is valid.
  info->set_object(nullptr);
  info->set_end(0x2000);
  object = info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());
}

// Verify that if the offset is non-zero but there is no elf at the offset,
// that the full file is used.
TEST_F(MapInfoGetObjectTest, file_backed_non_zero_offset_full_file) {
  std::vector<uint8_t> buffer(0x1000);
  memset(buffer.data(), 0, buffer.size());
  Elf32_Ehdr ehdr;
  TestInitEhdr<Elf32_Ehdr>(&ehdr, ELFCLASS32, EM_ARM);
  memcpy(buffer.data(), &ehdr, sizeof(ehdr));
  ASSERT_TRUE(android::base::WriteFully(elf_.fd, buffer.data(), buffer.size()));

  auto info = MapInfo::Create(0x1000, 0x2000, 0x100, PROT_READ, elf_.path);
  Object* object = info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());
  ASSERT_TRUE(object->memory() != nullptr);
  ASSERT_EQ(0x100U, info->object_offset());

  // Read the entire file.
  memset(buffer.data(), 0, buffer.size());
  ASSERT_TRUE(object->memory()->ReadFully(0, buffer.data(), buffer.size()));
  ASSERT_EQ(0, memcmp(buffer.data(), &ehdr, sizeof(ehdr)));
  for (size_t i = sizeof(ehdr); i < buffer.size(); i++) {
    ASSERT_EQ(0, buffer[i]) << "Failed at byte " << i;
  }

  ASSERT_FALSE(object->memory()->ReadFully(buffer.size(), buffer.data(), 1));
}

// Verify that if the offset is non-zero and there is an elf at that
// offset, that only part of the file is used.
TEST_F(MapInfoGetObjectTest, file_backed_non_zero_offset_partial_file) {
  auto info = MapInfo::Create(0x1000, 0x2000, 0x2000, PROT_READ, elf_.path);

  std::vector<uint8_t> buffer(0x4000);
  memset(buffer.data(), 0, buffer.size());
  Elf32_Ehdr ehdr;
  TestInitEhdr<Elf32_Ehdr>(&ehdr, ELFCLASS32, EM_ARM);
  memcpy(&buffer[info->offset()], &ehdr, sizeof(ehdr));
  ASSERT_TRUE(android::base::WriteFully(elf_.fd, buffer.data(), buffer.size()));

  Object* object = info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());
  ASSERT_TRUE(object->memory() != nullptr);
  ASSERT_EQ(0U, info->object_offset());

  // Read the valid part of the file.
  ASSERT_TRUE(object->memory()->ReadFully(0, buffer.data(), 0x1000));
  ASSERT_EQ(0, memcmp(buffer.data(), &ehdr, sizeof(ehdr)));
  for (size_t i = sizeof(ehdr); i < 0x1000; i++) {
    ASSERT_EQ(0, buffer[i]) << "Failed at byte " << i;
  }

  ASSERT_FALSE(object->memory()->ReadFully(0x1000, buffer.data(), 1));
}

// Verify that if the offset is non-zero and there is an elf at that
// offset, that only part of the file is used. Further verify that if the
// embedded elf is bigger than the initial map, the new object is larger
// than the original map size. Do this for a 32 bit elf and a 64 bit elf.
TEST_F(MapInfoGetObjectTest, file_backed_non_zero_offset_partial_file_whole_elf32) {
  auto info = MapInfo::Create(0x5000, 0x6000, 0x1000, PROT_READ, elf_.path);

  std::vector<uint8_t> buffer(0x4000);
  memset(buffer.data(), 0, buffer.size());
  Elf32_Ehdr ehdr;
  TestInitEhdr<Elf32_Ehdr>(&ehdr, ELFCLASS32, EM_ARM);
  ehdr.e_shoff = 0x2000;
  ehdr.e_shentsize = sizeof(Elf32_Shdr) + 100;
  ehdr.e_shnum = 4;
  memcpy(&buffer[info->offset()], &ehdr, sizeof(ehdr));
  ASSERT_TRUE(android::base::WriteFully(elf_.fd, buffer.data(), buffer.size()));

  Object* object = info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());
  ASSERT_TRUE(object->memory() != nullptr);
  ASSERT_EQ(0U, info->object_offset());

  // Verify the memory is a valid elf.
  memset(buffer.data(), 0, buffer.size());
  ASSERT_TRUE(object->memory()->ReadFully(0, buffer.data(), 0x1000));
  ASSERT_EQ(0, memcmp(buffer.data(), &ehdr, sizeof(ehdr)));

  // Read past the end of what would normally be the size of the map.
  ASSERT_TRUE(object->memory()->ReadFully(0x1000, buffer.data(), 1));
}

TEST_F(MapInfoGetObjectTest, file_backed_non_zero_offset_partial_file_whole_elf64) {
  auto info = MapInfo::Create(0x7000, 0x8000, 0x1000, PROT_READ, elf_.path);

  std::vector<uint8_t> buffer(0x4000);
  memset(buffer.data(), 0, buffer.size());
  Elf64_Ehdr ehdr;
  TestInitEhdr<Elf64_Ehdr>(&ehdr, ELFCLASS64, EM_AARCH64);
  ehdr.e_shoff = 0x2000;
  ehdr.e_shentsize = sizeof(Elf64_Shdr) + 100;
  ehdr.e_shnum = 4;
  memcpy(&buffer[info->offset()], &ehdr, sizeof(ehdr));
  ASSERT_TRUE(android::base::WriteFully(elf_.fd, buffer.data(), buffer.size()));

  Object* object = info->GetObject(process_memory_, ARCH_ARM64);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());
  ASSERT_TRUE(object->memory() != nullptr);
  ASSERT_EQ(0U, info->object_offset());

  // Verify the memory is a valid elf.
  memset(buffer.data(), 0, buffer.size());
  ASSERT_TRUE(object->memory()->ReadFully(0, buffer.data(), 0x1000));
  ASSERT_EQ(0, memcmp(buffer.data(), &ehdr, sizeof(ehdr)));

  // Read past the end of what would normally be the size of the map.
  ASSERT_TRUE(object->memory()->ReadFully(0x1000, buffer.data(), 1));
}

// Verify that if the offset is non-zero and there is an elf at that
// offset, that only part of the file is used. Further verify that if the
// the initial map is smaller than elf header size, we can still read the elf.
TEST_F(MapInfoGetObjectTest, file_backed_non_zero_offset_partial_file_whole_elf64_small_map_range) {
  auto info = MapInfo::Create(0x7000, 0x7004, 0x1000, PROT_READ, elf_.path);

  std::vector<uint8_t> buffer(0x4000);
  memset(buffer.data(), 0, buffer.size());
  Elf64_Ehdr ehdr;
  TestInitEhdr<Elf64_Ehdr>(&ehdr, ELFCLASS64, EM_AARCH64);
  ehdr.e_shoff = 0x2000;
  ehdr.e_shentsize = sizeof(Elf64_Shdr) + 100;
  ehdr.e_shnum = 4;
  memcpy(&buffer[info->offset()], &ehdr, sizeof(ehdr));
  ASSERT_TRUE(android::base::WriteFully(elf_.fd, buffer.data(), buffer.size()));

  Object* object = info->GetObject(process_memory_, ARCH_ARM64);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());
  ASSERT_TRUE(object->memory() != nullptr);
  ASSERT_EQ(0U, info->object_offset());

  // Verify the memory is a valid elf.
  memset(buffer.data(), 0, buffer.size());
  ASSERT_TRUE(object->memory()->ReadFully(0, buffer.data(), 0x1000));
  ASSERT_EQ(0, memcmp(buffer.data(), &ehdr, sizeof(ehdr)));

  // Read past the end of what would normally be the size of the map.
  ASSERT_TRUE(object->memory()->ReadFully(0x1000, buffer.data(), 1));
}

TEST_F(MapInfoGetObjectTest, check_device_maps) {
  // Create valid elf data in process memory for this to verify that only
  // the name is causing invalid elf data.
  Elf64_Ehdr ehdr;
  TestInitEhdr<Elf64_Ehdr>(&ehdr, ELFCLASS64, EM_X86_64);
  ehdr.e_shoff = 0x2000;
  ehdr.e_shentsize = sizeof(Elf64_Shdr) + 100;
  ehdr.e_shnum = 0;
  memory_->SetMemory(0x7000, &ehdr, sizeof(ehdr));

  auto info =
      MapInfo::Create(0x7000, 0x8000, 0x1000, PROT_READ | MAPS_FLAGS_DEVICE_MAP, "/dev/something");
  Object* object = info->GetObject(process_memory_, ARCH_X86_64);
  ASSERT_TRUE(object != nullptr);
  ASSERT_FALSE(object->valid());

  // Set the name to nothing to verify that it still fails.
  info->set_object(nullptr);
  info->set_name("");
  object = info->GetObject(process_memory_, ARCH_X86_64);
  ASSERT_FALSE(object->valid());

  // Change the flags and verify the elf is valid now.
  info->set_object(nullptr);
  info->set_flags(PROT_READ);
  object = info->GetObject(process_memory_, ARCH_X86_64);
  ASSERT_TRUE(object->valid());
}

TEST_F(MapInfoGetObjectTest, multiple_thread_get_object) {
  static constexpr size_t kNumConcurrentThreads = 100;

  Elf64_Ehdr ehdr;
  TestInitEhdr<Elf64_Ehdr>(&ehdr, ELFCLASS64, EM_X86_64);
  ehdr.e_shoff = 0x2000;
  ehdr.e_shentsize = sizeof(Elf64_Shdr) + 100;
  ehdr.e_shnum = 0;
  memory_->SetMemory(0x7000, &ehdr, sizeof(ehdr));

  Object* object_in_threads[kNumConcurrentThreads];
  std::vector<std::thread*> threads;

  std::atomic_bool wait;
  wait = true;
  // Create all of the threads and have them do the GetObject at the same time
  // to make it likely that a race will occur.
  auto info = MapInfo::Create(0x7000, 0x8000, 0x1000, PROT_READ, "");
  for (size_t i = 0; i < kNumConcurrentThreads; i++) {
    std::thread* thread = new std::thread([i, this, &wait, &info, &object_in_threads]() {
      while (wait)
        ;
      Object* object = info->GetObject(process_memory_, ARCH_X86_64);
      object_in_threads[i] = object;
    });
    threads.push_back(thread);
  }
  ASSERT_TRUE(info->object() == nullptr);

  // Set them all going and wait for the threads to finish.
  wait = false;
  for (auto thread : threads) {
    thread->join();
    delete thread;
  }

  // Now verify that all of the object files are exactly the same and valid.
  Object* object = info->object().get();
  ASSERT_TRUE(object != nullptr);
  EXPECT_TRUE(object->valid());
  for (size_t i = 0; i < kNumConcurrentThreads; i++) {
    EXPECT_EQ(object, object_in_threads[i]) << "Thread " << i << " mismatched.";
  }
}

// Verify that previous maps don't automatically get the same elf object.
TEST_F(MapInfoGetObjectTest, prev_map_elf_not_set) {
  auto info1 = MapInfo::Create(0x1000, 0x2000, 0, PROT_READ, "/not/present");
  auto info2 = MapInfo::Create(info1, 0x2000, 0x3000, 0, PROT_READ, elf_.path);

  Elf32_Ehdr ehdr;
  TestInitEhdr<Elf32_Ehdr>(&ehdr, ELFCLASS32, EM_ARM);
  memory_->SetMemory(0x2000, &ehdr, sizeof(ehdr));
  Object* object = info2->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());

  ASSERT_NE(object, info1->GetObject(process_memory_, ARCH_ARM));
}

void MapInfoGetObjectTest::InitMapInfo(std::vector<std::shared_ptr<MapInfo>>& maps,
                                       bool in_memory) {
  maps.resize(2);
  maps[0] = MapInfo::Create(0x1000, 0x2000, 0, PROT_READ, elf_.path);
  maps[1] = MapInfo::Create(maps[0], 0x2000, 0x3000, 0x1000, PROT_READ | PROT_EXEC, elf_.path);

  Elf32_Ehdr ehdr;
  TestInitEhdr<Elf32_Ehdr>(&ehdr, ELFCLASS32, EM_ARM);
  if (in_memory) {
    memory_->SetMemory(0x1000, &ehdr, sizeof(ehdr));
  } else {
    ASSERT_TRUE(android::base::WriteFully(elf_.fd, &ehdr, sizeof(ehdr)));
  }
}

// Verify that a read-only map followed by a read-execute map will result
// in the same elf object in both maps.
TEST_F(MapInfoGetObjectTest, read_only_followed_by_read_exec_share_elf_exec_first) {
  std::vector<std::shared_ptr<MapInfo>> maps;

  // First use in memory maps.
  InitMapInfo(maps, true);
  ASSERT_EQ(2U, maps.size());
  MapInfo* r_map_info = maps[0].get();
  MapInfo* rx_map_info = maps[1].get();

  // Get the object from the read-exec map first.
  Object* object = rx_map_info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());

  ASSERT_EQ(object, r_map_info->GetObject(process_memory_, ARCH_ARM));

  // Now use file maps.
  maps.clear();
  InitMapInfo(maps, false);
  ASSERT_EQ(2U, maps.size());
  r_map_info = maps[0].get();
  rx_map_info = maps[1].get();

  // Get the object from the read-exec map first.
  object = rx_map_info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());

  ASSERT_EQ(object, r_map_info->GetObject(process_memory_, ARCH_ARM));
}

// Verify that a read-only map followed by a read-execute map will result
// in the same elf object in both maps.
TEST_F(MapInfoGetObjectTest, read_only_followed_by_read_exec_share_elf_read_only_first) {
  std::vector<std::shared_ptr<MapInfo>> maps;

  // First use in memory maps.
  InitMapInfo(maps, true);
  ASSERT_EQ(2U, maps.size());
  MapInfo* r_map_info = maps[0].get();
  MapInfo* rx_map_info = maps[1].get();

  // Get the elf from the read-only map first.
  Object* object = r_map_info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());

  ASSERT_EQ(object, rx_map_info->GetObject(process_memory_, ARCH_ARM));

  // Now use file maps.
  maps.clear();
  InitMapInfo(maps, false);
  ASSERT_EQ(2U, maps.size());
  r_map_info = maps[0].get();
  rx_map_info = maps[1].get();

  // Get the elf from the read-only map first.
  object = r_map_info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());

  ASSERT_EQ(object, rx_map_info->GetObject(process_memory_, ARCH_ARM));
}

// Verify that a read-only map followed by an empty map, then followed by
// a read-execute map will result in the same elf object in both maps.
TEST_F(MapInfoGetObjectTest, read_only_followed_by_empty_then_read_exec_share_elf) {
  auto r_info = MapInfo::Create(0x1000, 0x2000, 0, PROT_READ, elf_.path);
  auto empty = MapInfo::Create(r_info, 0x2000, 0x3000, 0, 0, "");
  auto rw_info = MapInfo::Create(empty, 0x3000, 0x4000, 0x2000, PROT_READ | PROT_EXEC, elf_.path);

  Elf32_Ehdr ehdr;
  TestInitEhdr<Elf32_Ehdr>(&ehdr, ELFCLASS32, EM_ARM);
  memory_->SetMemory(0x1000, &ehdr, sizeof(ehdr));
  Object* object = rw_info->GetObject(process_memory_, ARCH_ARM);
  ASSERT_TRUE(object != nullptr);
  ASSERT_TRUE(object->valid());

  ASSERT_EQ(object, r_info->GetObject(process_memory_, ARCH_ARM));
}

template <class>
inline constexpr bool kAlwaysFalseV = false;

template <typename PeCoffInterfaceType>
class MapInfoGetObjectPeCoffTest : public ::testing::Test {
 public:
  MapInfoGetObjectPeCoffTest() {
    std::string file_name;
    if constexpr (std::is_same_v<PeCoffInterfaceType, PeCoffInterface32>) {
      // The .text section of libtest32.dll has:
      //   VirtualSize: 0x13b4
      //   VirtualAddress: 0x1000
      //   PointerToRawData: 0x600
      file_name = "libtest32.dll";
      arch_ = ArchEnum::ARCH_X86;
    } else if constexpr (std::is_same_v<PeCoffInterfaceType, PeCoffInterface64>) {
      // The .text section of libtest.dll has:
      //   VirtualSize: 0x1338
      //   VirtualAddress: 0x1000
      //   PointerToRawData: 0x600
      file_name = "libtest.dll";
      arch_ = ArchEnum::ARCH_X86_64;
    } else {
      static_assert(kAlwaysFalseV<PeCoffInterfaceType>, "non-exhaustive if");
    }
    file_path_ = android::base::GetExecutableDirectory() + "/tests/files/" + file_name;
  }

  const std::string& GetFilePath() const { return this->file_path_; }

  const std::shared_ptr<Memory>& GetProcessMemory() { return this->process_memory_; }

  ArchEnum GetArch() const { return arch_; }

 private:
  std::string file_path_;
  ArchEnum arch_;
  std::shared_ptr<Memory> process_memory_ = std::make_shared<MemoryFake>();
};

using PeCoffInterfaceTypes = ::testing::Types<PeCoffInterface32, PeCoffInterface64>;
TYPED_TEST_SUITE(MapInfoGetObjectPeCoffTest, PeCoffInterfaceTypes);

TYPED_TEST(MapInfoGetObjectPeCoffTest, correctly_gets_pe_from_file_mapping) {
  Maps maps;
  maps.Add(0x100000, 0x101000, 0, PROT_READ, this->GetFilePath(), 0);
  maps.Add(0x101000, 0x103000, 0x1000, PROT_READ | PROT_EXEC, this->GetFilePath(), 0);

  {
    std::shared_ptr<MapInfo> map_info = maps.Get(0);

    Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
    ASSERT_NE(object, nullptr);
    EXPECT_NE(dynamic_cast<PeCoff*>(map_info->object().get()), nullptr);
    EXPECT_TRUE(object->valid());

    EXPECT_EQ(map_info->object_offset(), 0);
    EXPECT_EQ(map_info->object_start_offset(), 0);
    EXPECT_EQ(map_info->object_rva(), 0);
    EXPECT_EQ(map_info->memory_backed_object(), false);
  }
  {
    std::shared_ptr<MapInfo> map_info = maps.Get(1);

    Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
    ASSERT_NE(object, nullptr);
    EXPECT_NE(dynamic_cast<PeCoff*>(map_info->object().get()), nullptr);
    EXPECT_TRUE(object->valid());

    EXPECT_EQ(map_info->object_offset(), 0x1000);
    EXPECT_EQ(map_info->object_start_offset(), 0);
    EXPECT_EQ(map_info->object_rva(), 0);
    EXPECT_EQ(map_info->memory_backed_object(), false);
  }
}

TYPED_TEST(MapInfoGetObjectPeCoffTest, gets_invalid_pe_from_file_mapping_if_wrong_arch) {
  Maps maps;
  maps.Add(0x100000, 0x101000, 0, PROT_READ, this->GetFilePath(), 0);
  maps.Add(0x101000, 0x103000, 0x1000, PROT_READ | PROT_EXEC, this->GetFilePath(), 0);

  {
    std::shared_ptr<MapInfo> map_info = maps.Get(0);

    Object* object = map_info->GetObject(this->GetProcessMemory(), ArchEnum::ARCH_ARM64);
    ASSERT_NE(object, nullptr);
    EXPECT_NE(dynamic_cast<PeCoff*>(object), nullptr);
    EXPECT_FALSE(object->valid());  // Invalidated by GetObject.
  }
  {
    std::shared_ptr<MapInfo> map_info = maps.Get(1);

    Object* object = map_info->GetObject(this->GetProcessMemory(), ArchEnum::ARCH_ARM64);
    ASSERT_NE(object, nullptr);
    EXPECT_NE(dynamic_cast<PeCoff*>(object), nullptr);
    EXPECT_FALSE(object->valid());  // Invalidated by GetObject.
  }
}

TYPED_TEST(MapInfoGetObjectPeCoffTest, correctly_gets_pe_from_anon_exec_map) {
  Maps maps;
  maps.Add(0x100000, 0x101000, 0x0, PROT_READ, this->GetFilePath(), 0);
  maps.Add(0x101000, 0x103000, 0x0, PROT_READ | PROT_EXEC, "", 0);

  std::shared_ptr<MapInfo> map_info = maps.Get(1);

  Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
  ASSERT_NE(object, nullptr);
  EXPECT_NE(dynamic_cast<PeCoff*>(object), nullptr);
  EXPECT_TRUE(object->valid());

  // Verify the ObjectFields that have been set.
  EXPECT_EQ(map_info->object_offset(), 0);
  EXPECT_EQ(map_info->object_start_offset(), 0);
  EXPECT_EQ(map_info->object_rva(), 0x1000);
  EXPECT_FALSE(map_info->memory_backed_object());
}

TYPED_TEST(MapInfoGetObjectPeCoffTest, correctly_gets_pe_from_multiple_anon_exec_maps) {
  Maps maps;
  maps.Add(0x100000, 0x101000, 0x0, PROT_READ, this->GetFilePath(), 0);
  maps.Add(0x101000, 0x103000, 0x0, PROT_READ | PROT_EXEC, "", 0);
  maps.Add(0x103000, 0x106000, 0x0, PROT_READ | PROT_EXEC, "", 0);

  {
    std::shared_ptr<MapInfo> map_info = maps.Get(1);

    Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
    ASSERT_NE(object, nullptr);
    EXPECT_NE(dynamic_cast<PeCoff*>(object), nullptr);
    EXPECT_TRUE(object->valid());

    EXPECT_EQ(map_info->object_offset(), 0);
    EXPECT_EQ(map_info->object_start_offset(), 0);
    EXPECT_EQ(map_info->object_rva(), 0x1000);
    EXPECT_FALSE(map_info->memory_backed_object());
  }
  {
    std::shared_ptr<MapInfo> map_info = maps.Get(2);

    Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
    ASSERT_NE(object, nullptr);
    EXPECT_NE(dynamic_cast<PeCoff*>(object), nullptr);
    EXPECT_TRUE(object->valid());

    EXPECT_EQ(map_info->object_offset(), 0);
    EXPECT_EQ(map_info->object_start_offset(), 0);
    EXPECT_EQ(map_info->object_rva(), 0x3000);
    EXPECT_FALSE(map_info->memory_backed_object());
  }
}

TYPED_TEST(MapInfoGetObjectPeCoffTest,
           correctly_gets_pe_from_anon_exec_maps_in_more_complex_example) {
  // The addresses in these maps are not page-aligned, but it doesn't matter for the test's purpose.
  Maps maps;
  maps.Add(0x10000, 0x11000, 0x0, PROT_READ | PROT_WRITE, "[stack]", 0);
  maps.Add(0x100000, 0x101000, 0x0, PROT_READ, this->GetFilePath(), 0);  // headers
  maps.Add(0x101000, 0x102000, 0x0, PROT_READ | PROT_WRITE, "", 0);
  maps.Add(0x102000, 0x103000, 0x2000, PROT_READ, this->GetFilePath(), 0);
  maps.Add(0x103000, 0x104000, 0x0, PROT_READ | PROT_EXEC, "[special]", 0);
  maps.Add(0x104000, 0x105000, 0x4000, PROT_READ | PROT_EXEC, this->GetFilePath(), 0);
  maps.Add(0x105000, 0x106000, 0x0, PROT_READ | PROT_EXEC, "", 0);  // map_info1
  maps.Add(0x106000, 0x107000, 0x6000, PROT_READ | PROT_EXEC, this->GetFilePath(), 0);
  maps.Add(0x107000, 0x108000, 0x0, PROT_READ | PROT_WRITE, "[special]", 0);
  maps.Add(0x108000, 0x109000, 0x0, PROT_READ | PROT_EXEC, "", 0);  // map_info2
  maps.Add(0x200000, 0x201000, 0x0, PROT_READ | PROT_EXEC, "/path/to/something/else", 0);

  {
    std::shared_ptr<MapInfo> map_info1 = maps.Find(0x105000);

    Object* object = map_info1->GetObject(this->GetProcessMemory(), this->GetArch());
    ASSERT_NE(object, nullptr);
    EXPECT_NE(dynamic_cast<PeCoff*>(object), nullptr);
    EXPECT_TRUE(object->valid());

    EXPECT_EQ(map_info1->object_offset(), 0);
    EXPECT_EQ(map_info1->object_start_offset(), 0);
    EXPECT_EQ(map_info1->object_rva(), 0x5000);
    EXPECT_FALSE(map_info1->memory_backed_object());
  }
  {
    std::shared_ptr<MapInfo> map_info2 = maps.Find(0x108000);

    Object* object = map_info2->GetObject(this->GetProcessMemory(), this->GetArch());
    ASSERT_NE(object, nullptr);
    EXPECT_NE(dynamic_cast<PeCoff*>(object), nullptr);
    EXPECT_TRUE(object->valid());

    EXPECT_EQ(map_info2->object_offset(), 0);
    EXPECT_EQ(map_info2->object_start_offset(), 0);
    EXPECT_EQ(map_info2->object_rva(), 0x8000);
    EXPECT_FALSE(map_info2->memory_backed_object());
  }
}

TYPED_TEST(MapInfoGetObjectPeCoffTest, does_not_get_pe_from_anon_exec_map_if_named_map_has_offset) {
  Maps maps;
  maps.Add(0x101000, 0x102000, 0x1000, PROT_READ, this->GetFilePath(), 0);
  maps.Add(0x102000, 0x103000, 0x0, PROT_READ | PROT_EXEC, "", 0);

  std::shared_ptr<MapInfo> map_info = maps.Get(1);

  Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
  ASSERT_NE(object, nullptr);
  EXPECT_FALSE(object->valid());
}

TYPED_TEST(MapInfoGetObjectPeCoffTest, does_not_get_pe_from_exec_map_if_map_has_a_name) {
  Maps maps;
  maps.Add(0x100000, 0x101000, 0x0, PROT_READ, this->GetFilePath(), 0);
  maps.Add(0x101000, 0x103000, 0x0, PROT_READ | PROT_EXEC, "i_already_have_a_name", 0);

  std::shared_ptr<MapInfo> map_info = maps.Get(1);

  Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
  ASSERT_NE(object, nullptr);
  EXPECT_FALSE(object->valid());
}

TYPED_TEST(MapInfoGetObjectPeCoffTest, does_not_get_pe_from_anon_map_if_map_is_not_executable) {
  Maps maps;
  maps.Add(0x100000, 0x101000, 0x0, PROT_READ, this->GetFilePath(), 0);
  maps.Add(0x101000, 0x103000, 0x0, PROT_READ, "", 0);

  std::shared_ptr<MapInfo> map_info = maps.Get(1);

  Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
  ASSERT_NE(object, nullptr);
  EXPECT_FALSE(object->valid());
}

TYPED_TEST(MapInfoGetObjectPeCoffTest,
           does_not_get_pe_from_anon_exec_map_if_no_previous_named_map) {
  Maps maps;
  maps.Add(0x100000, 0x101000, 0x0, PROT_READ, "", 0);
  maps.Add(0x101000, 0x103000, 0x0, PROT_READ | PROT_EXEC, "", 0);

  std::shared_ptr<MapInfo> map_info = maps.Get(1);

  Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
  ASSERT_NE(object, nullptr);
  EXPECT_FALSE(object->valid());
}

TYPED_TEST(MapInfoGetObjectPeCoffTest, does_not_get_pe_from_anon_exec_map_if_wrong_arch) {
  Maps maps;
  maps.Add(0x100000, 0x101000, 0x0, PROT_READ, this->GetFilePath(), 0);
  maps.Add(0x101000, 0x103000, 0x0, PROT_READ | PROT_EXEC, "", 0);

  std::shared_ptr<MapInfo> map_info = maps.Get(1);

  Object* object = map_info->GetObject(this->GetProcessMemory(), ArchEnum::ARCH_ARM64);
  ASSERT_NE(object, nullptr);
  EXPECT_NE(dynamic_cast<PeCoff*>(object), nullptr);
  EXPECT_FALSE(object->valid());  // Invalidated by GetObject.
}

TYPED_TEST(MapInfoGetObjectPeCoffTest,
           does_not_get_pe_from_anon_exec_map_if_named_map_is_a_device_map) {
  Maps maps;
  maps.Add(0x100000, 0x101000, 0x0, PROT_READ | MAPS_FLAGS_DEVICE_MAP, this->GetFilePath(), 0);
  maps.Add(0x101000, 0x103000, 0x0, PROT_READ | PROT_EXEC, "", 0);

  std::shared_ptr<MapInfo> map_info = maps.Get(1);

  Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
  ASSERT_NE(object, nullptr);
  EXPECT_FALSE(object->valid());
}

TYPED_TEST(MapInfoGetObjectPeCoffTest,
           does_not_get_pe_from_anon_exec_map_if_named_map_is_not_a_pe) {
  const std::string not_a_pe_file_path =
      android::base::GetExecutableDirectory() + "/tests/files/elf64.xz";
  Maps maps;
  maps.Add(0x100000, 0x101000, 0x0, PROT_READ, not_a_pe_file_path, 0);
  maps.Add(0x101000, 0x103000, 0x0, PROT_READ | PROT_EXEC, "", 0);

  std::shared_ptr<MapInfo> map_info = maps.Get(1);

  Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
  ASSERT_NE(object, nullptr);
  EXPECT_FALSE(object->valid());
}

TYPED_TEST(MapInfoGetObjectPeCoffTest, does_not_get_pe_if_anon_exec_map_ends_beyond_size_of_image) {
  Maps maps;
  maps.Add(0x100000, 0x101000, 0x0, PROT_READ, this->GetFilePath(), 0);
  maps.Add(0x101000, 0x122000, 0x0, PROT_READ | PROT_EXEC, "", 0);

  std::shared_ptr<MapInfo> map_info = maps.Get(1);

  Object* object = map_info->GetObject(this->GetProcessMemory(), this->GetArch());
  ASSERT_NE(object, nullptr);
  EXPECT_FALSE(object->valid());
}

}  // namespace unwindstack
