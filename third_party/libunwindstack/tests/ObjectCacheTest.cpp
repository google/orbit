/*
 * Copyright (C) 2018 The Android Open Source Project
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
#include <sys/mman.h>
#include <unistd.h>

#include <memory>

#include <android-base/file.h>

#include <gtest/gtest.h>

#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Object.h>

#include "ElfFake.h"
#include "ElfTestUtils.h"
#include "utils/MemoryFake.h"

#include <inttypes.h>

namespace unwindstack {

class ObjectCacheTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() { memory_.reset(new MemoryFake); }

  void SetUp() override {
    Object::SetCachingEnabled(true);

    // Create maps for testing.
    maps_.reset(
        new BufferMaps("1000-2000 r-xs 00000000 00:00 0 elf_one.so\n"
                       "2000-3000 r-xs 00000000 00:00 0 elf_two.so\n"
                       "3000-4000 ---s 00000000 00:00 0\n"
                       "4000-5000 r--s 00000000 00:00 0 elf_three.so\n"
                       "5000-6000 r-xs 00001000 00:00 0 elf_three.so\n"
                       "6000-7000 ---s 00000000 00:00 0\n"
                       "7000-8000 r--s 00001000 00:00 0 app_one.apk\n"
                       "8000-9000 r-xs 00005000 00:00 0 app_one.apk\n"
                       "9000-a000 r--s 00004000 00:00 0 app_two.apk\n"
                       "a000-b000 r-xs 00005000 00:00 0 app_two.apk\n"
                       "b000-c000 r--s 00008000 00:00 0 app_two.apk\n"
                       "c000-d000 r-xs 00009000 00:00 0 app_two.apk\n"
                       "d000-e000 ---s 00000000 00:00 0\n"
                       "e000-f000 r-xs 00000000 00:00 0 invalid\n"
                       "f000-10000 r-xs 00000000 00:00 0 invalid\n"
                       "10000-11000 r-xs 00000000 00:00 0 elf_two.so\n"
                       "11000-12000 r-xs 00000000 00:00 0 elf_one.so\n"
                       "12000-13000 r--s 00000000 00:00 0 elf_three.so\n"
                       "13000-14000 r-xs 00001000 00:00 0 elf_three.so\n"
                       "14000-15000 ---s 00000000 00:00 0\n"
                       "15000-16000 r--s 00001000 00:00 0 app_one.apk\n"
                       "16000-17000 r-xs 00005000 00:00 0 app_one.apk\n"
                       "17000-18000 r--s 00004000 00:00 0 app_two.apk\n"
                       "18000-19000 r-xs 00005000 00:00 0 app_two.apk\n"
                       "19000-1a000 r--s 00008000 00:00 0 app_two.apk\n"
                       "1a000-1b000 r-xs 00009000 00:00 0 app_two.apk\n"));
    ASSERT_TRUE(maps_->Parse());

    std::unordered_map<std::string, std::string> renames;

    temps_.emplace_back(new TemporaryFile);
    renames["elf_one.so"] = temps_.back()->path;
    WriteElfFile(0, temps_.back().get());

    temps_.emplace_back(new TemporaryFile);
    renames["elf_two.so"] = temps_.back()->path;
    WriteElfFile(0, temps_.back().get());

    temps_.emplace_back(new TemporaryFile);
    renames["elf_three.so"] = temps_.back()->path;
    WriteElfFile(0, temps_.back().get());

    temps_.emplace_back(new TemporaryFile);
    renames["app_one.apk"] = temps_.back()->path;
    WriteElfFile(0x1000, temps_.back().get());
    WriteElfFile(0x5000, temps_.back().get());

    temps_.emplace_back(new TemporaryFile);
    renames["app_two.apk"] = temps_.back()->path;
    WriteElfFile(0x4000, temps_.back().get());
    WriteElfFile(0x8000, temps_.back().get());

    for (auto& map_info : *maps_) {
      if (!map_info->name().empty()) {
        if (renames.count(map_info->name()) != 0) {
          // Replace the name with the temporary file name.
          map_info->name() = renames.at(map_info->name());
        }
      }
    }
  }

  // Make sure the cache is cleared between runs.
  void TearDown() override { Object::SetCachingEnabled(false); }

  void WriteElfFile(uint64_t offset, TemporaryFile* tf) {
    Elf32_Ehdr ehdr;
    TestInitEhdr(&ehdr, ELFCLASS32, EM_ARM);
    Elf32_Shdr shdr = {};
    shdr.sh_type = SHT_NULL;

    ehdr.e_shnum = 1;
    ehdr.e_shoff = 0x2000;
    ehdr.e_shentsize = sizeof(shdr);

    ASSERT_EQ(offset, static_cast<uint64_t>(lseek(tf->fd, offset, SEEK_SET)));
    ASSERT_TRUE(android::base::WriteFully(tf->fd, &ehdr, sizeof(ehdr)));
    ASSERT_EQ(offset + 0x2000, static_cast<uint64_t>(lseek(tf->fd, offset + 0x2000, SEEK_SET)));
    ASSERT_TRUE(android::base::WriteFully(tf->fd, &shdr, sizeof(shdr)));
  }

  std::vector<std::unique_ptr<TemporaryFile>> temps_;
  std::unique_ptr<Maps> maps_;
  static std::shared_ptr<Memory> memory_;
};

std::shared_ptr<Memory> ObjectCacheTest::memory_;

TEST_F(ObjectCacheTest, verify_elf_caching) {
  Object* elf_one = maps_->Find(0x1000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(elf_one->valid());
  Object* elf_two = maps_->Find(0x2000)->GetObject(memory_, ARCH_ARM);
  EXPECT_TRUE(elf_two->valid());
  Object* elf_three = maps_->Find(0x4000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(elf_three->valid());

  // Check that the caching is working for elf files.
  EXPECT_EQ(maps_->Find(0x5000)->GetObject(memory_, ARCH_ARM), elf_three);
  EXPECT_EQ(0U, maps_->Find(0x5000)->object_start_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x5000)->object_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x5000)->offset());

  EXPECT_EQ(maps_->Find(0x10000)->GetObject(memory_, ARCH_ARM), elf_two);
  EXPECT_EQ(0U, maps_->Find(0x10000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x10000)->object_offset());
  EXPECT_EQ(0U, maps_->Find(0x10000)->offset());

  EXPECT_EQ(maps_->Find(0x11000)->GetObject(memory_, ARCH_ARM), elf_one);
  EXPECT_EQ(0U, maps_->Find(0x11000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x11000)->object_offset());
  EXPECT_EQ(0U, maps_->Find(0x11000)->offset());

  EXPECT_EQ(maps_->Find(0x12000)->GetObject(memory_, ARCH_ARM), elf_three);
  EXPECT_EQ(0U, maps_->Find(0x12000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x12000)->object_offset());
  EXPECT_EQ(0U, maps_->Find(0x12000)->offset());

  EXPECT_EQ(maps_->Find(0x13000)->GetObject(memory_, ARCH_ARM), elf_three);
  EXPECT_EQ(0U, maps_->Find(0x13000)->object_start_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x13000)->object_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x13000)->offset());
}

TEST_F(ObjectCacheTest, verify_elf_caching_ro_first_ro_second) {
  Object* elf_three = maps_->Find(0x4000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(elf_three->valid());

  EXPECT_EQ(maps_->Find(0x12000)->GetObject(memory_, ARCH_ARM), elf_three);
  EXPECT_EQ(0U, maps_->Find(0x12000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x12000)->object_offset());
  EXPECT_EQ(0U, maps_->Find(0x12000)->offset());
}

TEST_F(ObjectCacheTest, verify_elf_caching_ro_first_rx_second) {
  Object* elf_three = maps_->Find(0x4000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(elf_three->valid());

  EXPECT_EQ(maps_->Find(0x13000)->GetObject(memory_, ARCH_ARM), elf_three);
  EXPECT_EQ(0U, maps_->Find(0x13000)->object_start_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x13000)->object_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x13000)->offset());
}

TEST_F(ObjectCacheTest, verify_elf_caching_rx_first_ro_second) {
  Object* elf_three = maps_->Find(0x5000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(elf_three->valid());

  EXPECT_EQ(maps_->Find(0x12000)->GetObject(memory_, ARCH_ARM), elf_three);
  EXPECT_EQ(0U, maps_->Find(0x12000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x12000)->object_offset());
  EXPECT_EQ(0U, maps_->Find(0x12000)->offset());
}

TEST_F(ObjectCacheTest, verify_elf_caching_rx_first_rx_second) {
  Object* elf_three = maps_->Find(0x5000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(elf_three->valid());

  EXPECT_EQ(maps_->Find(0x13000)->GetObject(memory_, ARCH_ARM), elf_three);
  EXPECT_EQ(0U, maps_->Find(0x13000)->object_start_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x13000)->object_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x13000)->offset());
}

TEST_F(ObjectCacheTest, verify_elf_apk_caching) {
  Object* app_one_elf1 = maps_->Find(0x7000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_one_elf1->valid());
  Object* app_one_elf2 = maps_->Find(0x8000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_one_elf2->valid());
  Object* app_two_elf1 = maps_->Find(0x9000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf1->valid());
  Object* app_two_elf2 = maps_->Find(0xb000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf2->valid());

  // Check that the caching is working for elf files in apks.
  EXPECT_EQ(maps_->Find(0xa000)->GetObject(memory_, ARCH_ARM), app_two_elf1);
  EXPECT_EQ(0x4000U, maps_->Find(0xa000)->object_start_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0xa000)->object_offset());
  EXPECT_EQ(0x5000U, maps_->Find(0xa000)->offset());

  EXPECT_EQ(maps_->Find(0xc000)->GetObject(memory_, ARCH_ARM), app_two_elf2);
  EXPECT_EQ(0x8000U, maps_->Find(0xc000)->object_start_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0xc000)->object_offset());
  EXPECT_EQ(0x9000U, maps_->Find(0xc000)->offset());

  EXPECT_EQ(maps_->Find(0x15000)->GetObject(memory_, ARCH_ARM), app_one_elf1);
  EXPECT_EQ(0x1000U, maps_->Find(0x15000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x15000)->object_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x15000)->offset());

  EXPECT_EQ(maps_->Find(0x16000)->GetObject(memory_, ARCH_ARM), app_one_elf2);
  EXPECT_EQ(0x1000U, maps_->Find(0x16000)->object_start_offset());
  EXPECT_EQ(0x4000U, maps_->Find(0x16000)->object_offset());
  EXPECT_EQ(0x5000U, maps_->Find(0x16000)->offset());

  EXPECT_EQ(maps_->Find(0x17000)->GetObject(memory_, ARCH_ARM), app_two_elf1);
  EXPECT_EQ(0x4000U, maps_->Find(0x17000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x17000)->object_offset());
  EXPECT_EQ(0x4000U, maps_->Find(0x17000)->offset());

  EXPECT_EQ(maps_->Find(0x18000)->GetObject(memory_, ARCH_ARM), app_two_elf1);
  EXPECT_EQ(0x4000U, maps_->Find(0x18000)->object_start_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x18000)->object_offset());
  EXPECT_EQ(0x5000U, maps_->Find(0x18000)->offset());

  EXPECT_EQ(maps_->Find(0x19000)->GetObject(memory_, ARCH_ARM), app_two_elf2);
  EXPECT_EQ(0x8000U, maps_->Find(0x19000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x19000)->object_offset());
  EXPECT_EQ(0x8000U, maps_->Find(0x19000)->offset());

  EXPECT_EQ(maps_->Find(0x1a000)->GetObject(memory_, ARCH_ARM), app_two_elf2);
  EXPECT_EQ(0x8000U, maps_->Find(0x1a000)->object_start_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x1a000)->object_offset());
  EXPECT_EQ(0x9000U, maps_->Find(0x1a000)->offset());
}

TEST_F(ObjectCacheTest, verify_elf_apk_caching_ro_first_ro_second) {
  Object* app_two_elf1 = maps_->Find(0x9000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf1->valid());
  Object* app_two_elf2 = maps_->Find(0xb000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf2->valid());

  EXPECT_EQ(maps_->Find(0x17000)->GetObject(memory_, ARCH_ARM), app_two_elf1);
  EXPECT_EQ(0x4000U, maps_->Find(0x17000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x17000)->object_offset());
  EXPECT_EQ(0x4000U, maps_->Find(0x17000)->offset());

  EXPECT_EQ(maps_->Find(0x19000)->GetObject(memory_, ARCH_ARM), app_two_elf2);
  EXPECT_EQ(0x8000U, maps_->Find(0x19000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x19000)->object_offset());
  EXPECT_EQ(0x8000U, maps_->Find(0x19000)->offset());
}

TEST_F(ObjectCacheTest, verify_elf_apk_caching_ro_first_rx_second) {
  Object* app_two_elf1 = maps_->Find(0x9000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf1->valid());
  Object* app_two_elf2 = maps_->Find(0xb000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf2->valid());

  EXPECT_EQ(maps_->Find(0x18000)->GetObject(memory_, ARCH_ARM), app_two_elf1);
  EXPECT_EQ(0x4000U, maps_->Find(0x18000)->object_start_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x18000)->object_offset());
  EXPECT_EQ(0x5000U, maps_->Find(0x18000)->offset());

  EXPECT_EQ(maps_->Find(0x1a000)->GetObject(memory_, ARCH_ARM), app_two_elf2);
  EXPECT_EQ(0x8000U, maps_->Find(0x1a000)->object_start_offset());
  EXPECT_EQ(0x1000U, maps_->Find(0x1a000)->object_offset());
  EXPECT_EQ(0x9000U, maps_->Find(0x1a000)->offset());
}

TEST_F(ObjectCacheTest, verify_elf_apk_caching_rx_first_ro_second) {
  Object* app_two_elf1 = maps_->Find(0xa000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf1->valid());
  Object* app_two_elf2 = maps_->Find(0xc000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf2->valid());

  EXPECT_EQ(maps_->Find(0x17000)->GetObject(memory_, ARCH_ARM), app_two_elf1);
  EXPECT_EQ(0x4000U, maps_->Find(0x17000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x17000)->object_offset());
  EXPECT_EQ(0x4000U, maps_->Find(0x17000)->offset());

  EXPECT_EQ(maps_->Find(0x19000)->GetObject(memory_, ARCH_ARM), app_two_elf2);
  EXPECT_EQ(0x8000U, maps_->Find(0x19000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x19000)->object_offset());
  EXPECT_EQ(0x8000U, maps_->Find(0x19000)->offset());
}

TEST_F(ObjectCacheTest, verify_elf_apk_caching_rx_first_rx_second) {
  Object* app_two_elf1 = maps_->Find(0x9000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf1->valid());
  Object* app_two_elf2 = maps_->Find(0xb000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf2->valid());

  EXPECT_EQ(maps_->Find(0x17000)->GetObject(memory_, ARCH_ARM), app_two_elf1);
  EXPECT_EQ(0x4000U, maps_->Find(0x17000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x17000)->object_offset());
  EXPECT_EQ(0x4000U, maps_->Find(0x17000)->offset());

  EXPECT_EQ(maps_->Find(0x19000)->GetObject(memory_, ARCH_ARM), app_two_elf2);
  EXPECT_EQ(0x8000U, maps_->Find(0x19000)->object_start_offset());
  EXPECT_EQ(0U, maps_->Find(0x19000)->object_offset());
  EXPECT_EQ(0x8000U, maps_->Find(0x19000)->offset());
}

// Verify that with elf caching disabled, we aren't caching improperly.
TEST_F(ObjectCacheTest, verify_disable_elf_caching) {
  Object::SetCachingEnabled(false);

  Object* elf_one = maps_->Find(0x1000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(elf_one->valid());
  Object* elf_two = maps_->Find(0x2000)->GetObject(memory_, ARCH_ARM);
  EXPECT_TRUE(elf_two->valid());
  Object* elf_three = maps_->Find(0x4000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(elf_three->valid());
  EXPECT_EQ(maps_->Find(0x5000)->GetObject(memory_, ARCH_ARM), elf_three);

  EXPECT_NE(maps_->Find(0x10000)->GetObject(memory_, ARCH_ARM), elf_two);
  EXPECT_NE(maps_->Find(0x11000)->GetObject(memory_, ARCH_ARM), elf_one);
  EXPECT_NE(maps_->Find(0x12000)->GetObject(memory_, ARCH_ARM), elf_three);
  EXPECT_NE(maps_->Find(0x13000)->GetObject(memory_, ARCH_ARM), elf_three);

  Object* app_one_elf1 = maps_->Find(0x7000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_one_elf1->valid());
  Object* app_one_elf2 = maps_->Find(0x8000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_one_elf2->valid());
  Object* app_two_elf1 = maps_->Find(0x9000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf1->valid());
  EXPECT_EQ(maps_->Find(0xa000)->GetObject(memory_, ARCH_ARM), app_two_elf1);
  Object* app_two_elf2 = maps_->Find(0xb000)->GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(app_two_elf2->valid());
  EXPECT_EQ(maps_->Find(0xc000)->GetObject(memory_, ARCH_ARM), app_two_elf2);

  EXPECT_NE(maps_->Find(0x15000)->GetObject(memory_, ARCH_ARM), app_one_elf1);
  EXPECT_NE(maps_->Find(0x16000)->GetObject(memory_, ARCH_ARM), app_one_elf2);
  EXPECT_NE(maps_->Find(0x17000)->GetObject(memory_, ARCH_ARM), app_two_elf1);
  EXPECT_NE(maps_->Find(0x18000)->GetObject(memory_, ARCH_ARM), app_two_elf1);
  EXPECT_NE(maps_->Find(0x19000)->GetObject(memory_, ARCH_ARM), app_two_elf2);
  EXPECT_NE(maps_->Find(0x1a000)->GetObject(memory_, ARCH_ARM), app_two_elf2);
}

// Verify that invalid elf objects are not cached.
TEST_F(ObjectCacheTest, verify_invalid_not_cached) {
  Object* invalid_elf1 = maps_->Find(0xe000)->GetObject(memory_, ARCH_ARM);
  ASSERT_FALSE(invalid_elf1->valid());
  Object* invalid_elf2 = maps_->Find(0xf000)->GetObject(memory_, ARCH_ARM);
  ASSERT_FALSE(invalid_elf2->valid());
  ASSERT_NE(invalid_elf1, invalid_elf2);
}

}  // namespace unwindstack
