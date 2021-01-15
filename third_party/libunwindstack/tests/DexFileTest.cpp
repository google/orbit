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

#include <malloc.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <unordered_map>

#include <MemoryLocal.h>
#include <android-base/file.h>
#include <gtest/gtest.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Memory.h>

#include "DexFile.h"
#include "DexFileData.h"
#include "utils/MemoryFake.h"

namespace unwindstack {

static constexpr size_t kNumLeakLoops = 5000;
static constexpr size_t kMaxAllowedLeakBytes = 4 * 1024;

static void CheckForLeak(size_t loop, size_t* first_allocated_bytes, size_t* last_allocated_bytes) {
  size_t allocated_bytes = mallinfo().uordblks;
  if (*first_allocated_bytes == 0) {
    *first_allocated_bytes = allocated_bytes;
  } else if (*last_allocated_bytes > *first_allocated_bytes) {
    // Check that the total memory did not increase too much over the first loop.
    ASSERT_LE(*last_allocated_bytes - *first_allocated_bytes, kMaxAllowedLeakBytes)
        << "Failed in loop " << loop << " first_allocated_bytes " << *first_allocated_bytes
        << " last_allocated_bytes " << *last_allocated_bytes;
  }
  *last_allocated_bytes = allocated_bytes;
}

TEST(DexFileTest, from_file_no_leak) {
  TemporaryFile tf;
  ASSERT_TRUE(tf.fd != -1);

  ASSERT_EQ(sizeof(kDexData),
            static_cast<size_t>(TEMP_FAILURE_RETRY(write(tf.fd, kDexData, sizeof(kDexData)))));

  size_t first_allocated_bytes = 0;
  size_t last_allocated_bytes = 0;
  for (size_t i = 0; i < kNumLeakLoops; i++) {
    MemoryFake memory;
    auto info = MapInfo::Create(0, 0x10000, 0, 0x5, tf.path);
    EXPECT_TRUE(DexFile::Create(0, sizeof(kDexData), &memory, info.get()) != nullptr);
    ASSERT_NO_FATAL_FAILURE(CheckForLeak(i, &first_allocated_bytes, &last_allocated_bytes));
  }
}

TEST(DexFileTest, from_memory_no_leak) {
  MemoryFake memory;

  memory.SetMemory(0x1000, kDexData, sizeof(kDexData));

  size_t first_allocated_bytes = 0;
  size_t last_allocated_bytes = 0;
  for (size_t i = 0; i < kNumLeakLoops; i++) {
    EXPECT_TRUE(DexFile::Create(0x1000, sizeof(kDexData), &memory, nullptr) != nullptr);
    ASSERT_NO_FATAL_FAILURE(CheckForLeak(i, &first_allocated_bytes, &last_allocated_bytes));
  }
}

TEST(DexFileTest, create_using_file) {
  TemporaryFile tf;
  ASSERT_TRUE(tf.fd != -1);

  ASSERT_EQ(0x500, lseek(tf.fd, 0x500, SEEK_SET));
  ASSERT_EQ(sizeof(kDexData),
            static_cast<size_t>(TEMP_FAILURE_RETRY(write(tf.fd, kDexData, sizeof(kDexData)))));

  MemoryFake memory;
  auto info = MapInfo::Create(0, 0x10000, 0, 0x5, tf.path);
  EXPECT_TRUE(DexFile::Create(0x500, sizeof(kDexData), &memory, info.get()) != nullptr);
}

TEST(DexFileTest, create_using_file_non_zero_start) {
  TemporaryFile tf;
  ASSERT_TRUE(tf.fd != -1);

  ASSERT_EQ(0x500, lseek(tf.fd, 0x500, SEEK_SET));
  ASSERT_EQ(sizeof(kDexData),
            static_cast<size_t>(TEMP_FAILURE_RETRY(write(tf.fd, kDexData, sizeof(kDexData)))));

  MemoryFake memory;
  auto info = MapInfo::Create(0x100, 0x10000, 0, 0x5, tf.path);
  EXPECT_TRUE(DexFile::Create(0x600, sizeof(kDexData), &memory, info.get()) != nullptr);
}

TEST(DexFileTest, create_using_file_non_zero_offset) {
  TemporaryFile tf;
  ASSERT_TRUE(tf.fd != -1);

  ASSERT_EQ(0x500, lseek(tf.fd, 0x500, SEEK_SET));
  ASSERT_EQ(sizeof(kDexData),
            static_cast<size_t>(TEMP_FAILURE_RETRY(write(tf.fd, kDexData, sizeof(kDexData)))));

  MemoryFake memory;
  auto info = MapInfo::Create(0x100, 0x10000, 0x200, 0x5, tf.path);
  EXPECT_TRUE(DexFile::Create(0x400, sizeof(kDexData), &memory, info.get()) != nullptr);
}

TEST(DexFileTest, create_using_memory_empty_file) {
  MemoryFake memory;
  memory.SetMemory(0x4000, kDexData, sizeof(kDexData));
  auto info = MapInfo::Create(0x100, 0x10000, 0x200, 0x5, "");
  EXPECT_TRUE(DexFile::Create(0x4000, sizeof(kDexData), &memory, info.get()) != nullptr);
}

TEST(DexFileTest, create_using_memory_file_does_not_exist) {
  MemoryFake memory;
  memory.SetMemory(0x4000, kDexData, sizeof(kDexData));
  auto info = MapInfo::Create(0x100, 0x10000, 0x200, 0x5, "/does/not/exist");
  EXPECT_TRUE(DexFile::Create(0x4000, sizeof(kDexData), &memory, info.get()) != nullptr);
}

TEST(DexFileTest, create_using_memory_file_is_malformed) {
  TemporaryFile tf;
  ASSERT_TRUE(tf.fd != -1);

  ASSERT_EQ(sizeof(kDexData) - 10,
            static_cast<size_t>(TEMP_FAILURE_RETRY(write(tf.fd, kDexData, sizeof(kDexData) - 10))));

  MemoryFake memory;
  memory.SetMemory(0x4000, kDexData, sizeof(kDexData));
  auto info = MapInfo::Create(0x4000, 0x10000, 0x200, 0x5, "/does/not/exist");
  std::shared_ptr<DexFile> dex_file =
      DexFile::Create(0x4000, sizeof(kDexData), &memory, info.get());
  ASSERT_TRUE(dex_file != nullptr);

  // Check it came from memory by clearing memory and verifying it fails.
  memory.Clear();
  dex_file = DexFile::Create(0x4000, sizeof(kDexData), &memory, info.get());
  EXPECT_TRUE(dex_file == nullptr);
}

TEST(DexFileTest, create_using_memory_header_too_small) {
  MemoryFake memory;
  size_t size = 10;
  memory.SetMemory(0x4000, kDexData, size);
  EXPECT_TRUE(DexFile::Create(0x4000, size, &memory, nullptr) == nullptr);
}

TEST(DexFileTest, create_using_memory_size_too_small) {
  MemoryFake memory;
  size_t size = sizeof(kDexData) - 1;
  memory.SetMemory(0x4000, kDexData, size);
  EXPECT_TRUE(DexFile::Create(0x4000, size, &memory, nullptr) == nullptr);
}

TEST(DexFileTest, get_method) {
  MemoryFake memory;
  memory.SetMemory(0x4000, kDexData, sizeof(kDexData));
  auto info = MapInfo::Create(0x100, 0x10000, 0x200, 0x5, "");
  std::shared_ptr<DexFile> dex_file(DexFile::Create(0x4000, sizeof(kDexData), &memory, info.get()));
  ASSERT_TRUE(dex_file != nullptr);

  SharedString method;
  uint64_t method_offset;
  ASSERT_TRUE(dex_file->GetFunctionName(0x4102, &method, &method_offset));
  EXPECT_EQ("Main.<init>", method);
  EXPECT_EQ(2U, method_offset);

  ASSERT_TRUE(dex_file->GetFunctionName(0x4118, &method, &method_offset));
  EXPECT_EQ("Main.main", method);
  EXPECT_EQ(0U, method_offset);
}

TEST(DexFileTest, get_method_empty) {
  MemoryFake memory;
  memory.SetMemory(0x4000, kDexData, sizeof(kDexData));
  auto info = MapInfo::Create(0x100, 0x10000, 0x200, 0x5, "");
  std::shared_ptr<DexFile> dex_file(DexFile::Create(0x4000, sizeof(kDexData), &memory, info.get()));
  ASSERT_TRUE(dex_file != nullptr);

  SharedString method;
  uint64_t method_offset;
  EXPECT_FALSE(dex_file->GetFunctionName(0x100000, &method, &method_offset));

  EXPECT_FALSE(dex_file->GetFunctionName(0x98, &method, &method_offset));
}

TEST(DexFileTest, get_method_from_cache) {
  TemporaryFile tf;
  ASSERT_TRUE(tf.fd != -1);
  ASSERT_EQ(0, lseek(tf.fd, 0, SEEK_SET));
  ASSERT_EQ(sizeof(kDexData),
            static_cast<size_t>(TEMP_FAILURE_RETRY(write(tf.fd, kDexData, sizeof(kDexData)))));

  MemoryFake memory;
  auto info = MapInfo::Create(0x4000, 0x10000, 0, 0x5, tf.path);
  std::shared_ptr<DexFile> dex_file =
      DexFile::Create(0x4000, sizeof(kDexData), &memory, info.get());
  EXPECT_TRUE(dex_file != nullptr);

  SharedString method;
  uint64_t method_offset;
  ASSERT_TRUE(dex_file->GetFunctionName(0x4118, &method, &method_offset));
  EXPECT_EQ("Main.main", method);
  EXPECT_EQ(0U, method_offset);

  // Corrupt the dex file: change the name of the class.
  int main = std::string(reinterpret_cast<const char*>(kDexData), sizeof(kDexData)).find("Main");
  ASSERT_EQ(main, lseek(tf.fd, main, SEEK_SET));
  ASSERT_EQ(4u, static_cast<size_t>(TEMP_FAILURE_RETRY(write(tf.fd, "MAIN", 4))));

  // Check that we see the *old* cached value.
  ASSERT_TRUE(dex_file->GetFunctionName(0x4118, &method, &method_offset));
  EXPECT_EQ("Main.main", method);
  EXPECT_EQ(0U, method_offset);

  // Check that for other methods we see the *new* updated value.
  ASSERT_TRUE(dex_file->GetFunctionName(0x4102, &method, &method_offset));
  EXPECT_EQ("MAIN.<init>", method);
  EXPECT_EQ(2U, method_offset);
}

}  // namespace unwindstack
