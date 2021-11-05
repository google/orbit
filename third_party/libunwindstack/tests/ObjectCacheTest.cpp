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
#include <unistd.h>

#include <android-base/file.h>

#include <gtest/gtest.h>

#include <unwindstack/Object.h>
#include <unwindstack/MapInfo.h>

#include "ElfTestUtils.h"
#include "utils/MemoryFake.h"

namespace unwindstack {

class ObjectCacheTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() { memory_.reset(new MemoryFake); }

  void SetUp() override { Object::SetCachingEnabled(true); }

  void TearDown() override { Object::SetCachingEnabled(false); }

  void WriteElfFile(uint64_t offset, TemporaryFile* tf, uint32_t type) {
    ASSERT_TRUE(type == EM_ARM || type == EM_386 || type == EM_X86_64);
    size_t ehdr_size;
    Elf32_Ehdr ehdr32;
    Elf64_Ehdr ehdr64;
    void* ptr;
    if (type == EM_ARM || type == EM_386) {
      ehdr_size = sizeof(ehdr32);
      ptr = &ehdr32;
      TestInitEhdr(&ehdr32, ELFCLASS32, type);
    } else {
      ehdr_size = sizeof(ehdr64);
      ptr = &ehdr64;
      TestInitEhdr(&ehdr64, ELFCLASS64, type);
    }

    ASSERT_EQ(offset, static_cast<uint64_t>(lseek(tf->fd, offset, SEEK_SET)));
    ASSERT_TRUE(android::base::WriteFully(tf->fd, ptr, ehdr_size));
  }

  void VerifyWithinSameMap(bool cache_enabled);
  void VerifySameMap(bool cache_enabled);
  void VerifyWithinSameMapNeverReadAtZero(bool cache_enabled);

  static std::shared_ptr<Memory> memory_;
};

std::shared_ptr<Memory> ObjectCacheTest::memory_;

void ObjectCacheTest::VerifySameMap(bool cache_enabled) {
  if (!cache_enabled) {
    Object::SetCachingEnabled(false);
  }

  TemporaryFile tf;
  ASSERT_TRUE(tf.fd != -1);
  WriteElfFile(0, &tf, EM_ARM);
  close(tf.fd);

  uint64_t start = 0x1000;
  uint64_t end = 0x20000;
  MapInfo info1(nullptr, nullptr, start, end, 0, 0x5, tf.path);
  MapInfo info2(nullptr, nullptr, start, end, 0, 0x5, tf.path);

  Object* object1 = info1.GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(object1->valid());
  Object* object2 = info2.GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(object2->valid());

  if (cache_enabled) {
    EXPECT_EQ(object1, object2);
  } else {
    EXPECT_NE(object1, object2);
  }
}

TEST_F(ObjectCacheTest, no_caching) {
  VerifySameMap(false);
}

TEST_F(ObjectCacheTest, caching_invalid_elf) {
  VerifySameMap(true);
}

void ObjectCacheTest::VerifyWithinSameMap(bool cache_enabled) {
  if (!cache_enabled) {
    Object::SetCachingEnabled(false);
  }

  TemporaryFile tf;
  ASSERT_TRUE(tf.fd != -1);
  WriteElfFile(0, &tf, EM_ARM);
  WriteElfFile(0x100, &tf, EM_386);
  WriteElfFile(0x200, &tf, EM_X86_64);
  lseek(tf.fd, 0x500, SEEK_SET);
  uint8_t value = 0;
  write(tf.fd, &value, 1);
  close(tf.fd);

  uint64_t start = 0x1000;
  uint64_t end = 0x20000;
  // Will have an elf at offset 0 in file.
  MapInfo info0_1(nullptr, nullptr, start, end, 0, 0x5, tf.path);
  MapInfo info0_2(nullptr, nullptr, start, end, 0, 0x5, tf.path);
  // Will have an elf at offset 0x100 in file.
  MapInfo info100_1(nullptr, nullptr, start, end, 0x100, 0x5, tf.path);
  MapInfo info100_2(nullptr, nullptr, start, end, 0x100, 0x5, tf.path);
  // Will have an elf at offset 0x200 in file.
  MapInfo info200_1(nullptr, nullptr, start, end, 0x200, 0x5, tf.path);
  MapInfo info200_2(nullptr, nullptr, start, end, 0x200, 0x5, tf.path);
  // Will have an elf at offset 0 in file.
  MapInfo info300_1(nullptr, nullptr, start, end, 0x300, 0x5, tf.path);
  MapInfo info300_2(nullptr, nullptr, start, end, 0x300, 0x5, tf.path);

  Object* object0_1 = info0_1.GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(object0_1->valid());
  EXPECT_EQ(ARCH_ARM, object0_1->arch());
  Object* object0_2 = info0_2.GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(object0_2->valid());
  EXPECT_EQ(ARCH_ARM, object0_2->arch());
  EXPECT_EQ(0U, info0_1.object_offset());
  EXPECT_EQ(0U, info0_2.object_offset());
  if (cache_enabled) {
    EXPECT_EQ(object0_1, object0_2);
  } else {
    EXPECT_NE(object0_1, object0_2);
  }

  Object* object100_1 = info100_1.GetObject(memory_, ARCH_X86);
  ASSERT_TRUE(object100_1->valid());
  EXPECT_EQ(ARCH_X86, object100_1->arch());
  Object* object100_2 = info100_2.GetObject(memory_, ARCH_X86);
  ASSERT_TRUE(object100_2->valid());
  EXPECT_EQ(ARCH_X86, object100_2->arch());
  EXPECT_EQ(0U, info100_1.object_offset());
  EXPECT_EQ(0U, info100_2.object_offset());
  if (cache_enabled) {
    EXPECT_EQ(object100_1, object100_2);
  } else {
    EXPECT_NE(object100_1, object100_2);
  }

  Object* object200_1 = info200_1.GetObject(memory_, ARCH_X86_64);
  ASSERT_TRUE(object200_1->valid());
  EXPECT_EQ(ARCH_X86_64, object200_1->arch());
  Object* object200_2 = info200_2.GetObject(memory_, ARCH_X86_64);
  ASSERT_TRUE(object200_2->valid());
  EXPECT_EQ(ARCH_X86_64, object200_2->arch());
  EXPECT_EQ(0U, info200_1.object_offset());
  EXPECT_EQ(0U, info200_2.object_offset());
  if (cache_enabled) {
    EXPECT_EQ(object200_1, object200_2);
  } else {
    EXPECT_NE(object200_1, object200_2);
  }

  Object* object300_1 = info300_1.GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(object300_1->valid());
  EXPECT_EQ(ARCH_ARM, object300_1->arch());
  Object* object300_2 = info300_2.GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(object300_2->valid());
  EXPECT_EQ(ARCH_ARM, object300_2->arch());
  EXPECT_EQ(0x300U, info300_1.object_offset());
  EXPECT_EQ(0x300U, info300_2.object_offset());
  if (cache_enabled) {
    EXPECT_EQ(object300_1, object300_2);
    EXPECT_EQ(object0_1, object300_1);
  } else {
    EXPECT_NE(object300_1, object300_2);
    EXPECT_NE(object0_1, object300_1);
  }
}

TEST_F(ObjectCacheTest, no_caching_valid_elf_offset_non_zero) {
  VerifyWithinSameMap(false);
}

TEST_F(ObjectCacheTest, caching_valid_elf_offset_non_zero) {
  VerifyWithinSameMap(true);
}

// Verify that when reading from multiple non-zero offsets in the same map
// that when cached, all of the elf objects are the same.
void ObjectCacheTest::VerifyWithinSameMapNeverReadAtZero(bool cache_enabled) {
  if (!cache_enabled) {
    Object::SetCachingEnabled(false);
  }

  TemporaryFile tf;
  ASSERT_TRUE(tf.fd != -1);
  WriteElfFile(0, &tf, EM_ARM);
  lseek(tf.fd, 0x500, SEEK_SET);
  uint8_t value = 0;
  write(tf.fd, &value, 1);
  close(tf.fd);

  uint64_t start = 0x1000;
  uint64_t end = 0x20000;
  // Multiple info sections at different offsets will have non-zero elf offsets.
  MapInfo info300_1(nullptr, nullptr, start, end, 0x300, 0x5, tf.path);
  MapInfo info300_2(nullptr, nullptr, start, end, 0x300, 0x5, tf.path);
  MapInfo info400_1(nullptr, nullptr, start, end, 0x400, 0x5, tf.path);
  MapInfo info400_2(nullptr, nullptr, start, end, 0x400, 0x5, tf.path);

  Object* object300_1 = info300_1.GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(object300_1->valid());
  EXPECT_EQ(ARCH_ARM, object300_1->arch());
  Object* object300_2 = info300_2.GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(object300_2->valid());
  EXPECT_EQ(ARCH_ARM, object300_2->arch());
  EXPECT_EQ(0x300U, info300_1.object_offset());
  EXPECT_EQ(0x300U, info300_2.object_offset());
  if (cache_enabled) {
    EXPECT_EQ(object300_1, object300_2);
  } else {
    EXPECT_NE(object300_1, object300_2);
  }

  Object* object400_1 = info400_1.GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(object400_1->valid());
  EXPECT_EQ(ARCH_ARM, object400_1->arch());
  Object* object400_2 = info400_2.GetObject(memory_, ARCH_ARM);
  ASSERT_TRUE(object400_2->valid());
  EXPECT_EQ(ARCH_ARM, object400_2->arch());
  EXPECT_EQ(0x400U, info400_1.object_offset());
  EXPECT_EQ(0x400U, info400_2.object_offset());
  if (cache_enabled) {
    EXPECT_EQ(object400_1, object400_2);
    EXPECT_EQ(object300_1, object400_1);
  } else {
    EXPECT_NE(object400_1, object400_2);
    EXPECT_NE(object300_1, object400_1);
  }
}

TEST_F(ObjectCacheTest, no_caching_valid_elf_offset_non_zero_never_read_at_zero) {
  VerifyWithinSameMapNeverReadAtZero(false);
}

TEST_F(ObjectCacheTest, caching_valid_elf_offset_non_zero_never_read_at_zero) {
  VerifyWithinSameMapNeverReadAtZero(true);
}

}  // namespace unwindstack
