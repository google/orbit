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

#include <stdint.h>

#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "MemoryCache.h"
#include "utils/MemoryFake.h"

namespace unwindstack {

class MemoryThreadCacheTest : public ::testing::Test {
 protected:
  void SetUp() override {
    memory_ = new MemoryFake;
    memory_cache_.reset(new MemoryThreadCache(memory_));

    memory_->SetMemoryBlock(0x8000, 4096, 0xab);
    memory_->SetMemoryBlock(0x9000, 4096, 0xde);
    memory_->SetMemoryBlock(0xa000, 3000, 0x50);
  }

  MemoryFake* memory_;
  std::unique_ptr<MemoryThreadCache> memory_cache_;

  constexpr static size_t kMaxCachedSize = 64;
};

TEST_F(MemoryThreadCacheTest, cached_read) {
  for (size_t i = 1; i <= kMaxCachedSize; i++) {
    std::vector<uint8_t> buffer(i);
    ASSERT_TRUE(memory_cache_->ReadFully(0x8000 + i, buffer.data(), i))
        << "Read failed at size " << i;
    ASSERT_EQ(std::vector<uint8_t>(i, 0xab), buffer) << "Failed at size " << i;
  }

  // Verify the cached data is used.
  memory_->SetMemoryBlock(0x8000, 4096, 0xff);
  for (size_t i = 1; i <= kMaxCachedSize; i++) {
    std::vector<uint8_t> buffer(i);
    ASSERT_TRUE(memory_cache_->ReadFully(0x8000 + i, buffer.data(), i))
        << "Read failed at size " << i;
    ASSERT_EQ(std::vector<uint8_t>(i, 0xab), buffer) << "Failed at size " << i;
  }
}

TEST_F(MemoryThreadCacheTest, no_cached_read_after_clear) {
  for (size_t i = 1; i <= kMaxCachedSize; i++) {
    std::vector<uint8_t> buffer(i);
    ASSERT_TRUE(memory_cache_->ReadFully(0x8000 + i, buffer.data(), i))
        << "Read failed at size " << i;
    ASSERT_EQ(std::vector<uint8_t>(i, 0xab), buffer) << "Failed at size " << i;
  }

  // Verify the cached data is not used after a reset.
  memory_cache_->Clear();
  memory_->SetMemoryBlock(0x8000, 4096, 0xff);
  for (size_t i = 1; i <= kMaxCachedSize; i++) {
    std::vector<uint8_t> buffer(i);
    ASSERT_TRUE(memory_cache_->ReadFully(0x8000 + i, buffer.data(), i))
        << "Read failed at size " << i;
    ASSERT_EQ(std::vector<uint8_t>(i, 0xff), buffer) << "Failed at size " << i;
  }
}

TEST_F(MemoryThreadCacheTest, cached_read_across_caches) {
  std::vector<uint8_t> expect(16, 0xab);
  expect.resize(32, 0xde);

  std::vector<uint8_t> buffer(32);
  ASSERT_TRUE(memory_cache_->ReadFully(0x8ff0, buffer.data(), 32));
  ASSERT_EQ(expect, buffer);

  // Verify the cached data is used.
  memory_->SetMemoryBlock(0x8000, 4096, 0xff);
  memory_->SetMemoryBlock(0x9000, 4096, 0xff);
  ASSERT_TRUE(memory_cache_->ReadFully(0x8ff0, buffer.data(), 32));
  ASSERT_EQ(expect, buffer);
}

TEST_F(MemoryThreadCacheTest, no_cache_read) {
  for (size_t i = kMaxCachedSize + 1; i < 2 * kMaxCachedSize; i++) {
    std::vector<uint8_t> buffer(i);
    ASSERT_TRUE(memory_cache_->ReadFully(0x8000 + i, buffer.data(), i))
        << "Read failed at size " << i;
    ASSERT_EQ(std::vector<uint8_t>(i, 0xab), buffer) << "Failed at size " << i;
  }

  // Verify the cached data is not used.
  memory_->SetMemoryBlock(0x8000, 4096, 0xff);
  for (size_t i = kMaxCachedSize + 1; i < 2 * kMaxCachedSize; i++) {
    std::vector<uint8_t> buffer(i);
    ASSERT_TRUE(memory_cache_->ReadFully(0x8000 + i, buffer.data(), i))
        << "Read failed at size " << i;
    ASSERT_EQ(std::vector<uint8_t>(i, 0xff), buffer) << "Failed at size " << i;
  }
}

TEST_F(MemoryThreadCacheTest, read_for_cache_fail) {
  std::vector<uint8_t> buffer(kMaxCachedSize);
  ASSERT_TRUE(memory_cache_->ReadFully(0xa010, buffer.data(), kMaxCachedSize));
  ASSERT_EQ(std::vector<uint8_t>(kMaxCachedSize, 0x50), buffer);

  // Verify the cached data is not used.
  memory_->SetMemoryBlock(0xa000, 3000, 0xff);
  ASSERT_TRUE(memory_cache_->ReadFully(0xa010, buffer.data(), kMaxCachedSize));
  ASSERT_EQ(std::vector<uint8_t>(kMaxCachedSize, 0xff), buffer);
}

TEST_F(MemoryThreadCacheTest, read_for_cache_fail_cross) {
  std::vector<uint8_t> expect(16, 0xde);
  expect.resize(32, 0x50);

  std::vector<uint8_t> buffer(32);
  ASSERT_TRUE(memory_cache_->ReadFully(0x9ff0, buffer.data(), 32));
  ASSERT_EQ(expect, buffer);

  // Verify the cached data is not used for the second half but for the first.
  memory_->SetMemoryBlock(0xa000, 3000, 0xff);
  ASSERT_TRUE(memory_cache_->ReadFully(0x9ff0, buffer.data(), 32));
  expect.resize(16);
  expect.resize(32, 0xff);
  ASSERT_EQ(expect, buffer);
}

TEST_F(MemoryThreadCacheTest, read_cached_in_thread) {
  // Read from a different thread than this one.
  std::thread thread([this] {
    for (size_t i = 1; i <= kMaxCachedSize; i++) {
      std::vector<uint8_t> buffer(i);
      ASSERT_TRUE(this->memory_cache_->ReadFully(0x8000 + i, buffer.data(), i))
          << "Read failed at size " << i;
      ASSERT_EQ(std::vector<uint8_t>(i, 0xab), buffer) << "Failed at size " << i;
    }
  });

  thread.join();

  // Now modify the backing data, and read from the main thread verifying
  // it is not using cached data.
  memory_->SetMemoryBlock(0x8000, 4096, 0xff);
  for (size_t i = 1; i <= kMaxCachedSize; i++) {
    std::vector<uint8_t> buffer(i);
    ASSERT_TRUE(memory_cache_->ReadFully(0x8000 + i, buffer.data(), i))
        << "Read failed at size " << i;
    ASSERT_EQ(std::vector<uint8_t>(i, 0xff), buffer) << "Failed at size " << i;
  }
}

static void ExhaustPthreadKeys(std::vector<pthread_key_t>* keys) {
  // Use up all of the keys to force the next attempt to create one to fail.
  static constexpr size_t kMaxKeysToCreate = 10000;
  keys->resize(kMaxKeysToCreate);
  for (size_t i = 0; i < kMaxKeysToCreate; i++) {
    if (pthread_key_create(&(*keys)[i], nullptr) != 0) {
      keys->resize(i);
      break;
    }
  }
  ASSERT_NE(0U, keys->size()) << "No keys created.";
  ASSERT_LT(keys->size(), kMaxKeysToCreate) << "Cannot use up pthread keys.";
}

TEST_F(MemoryThreadCacheTest, read_uncached_due_to_error) {
  std::vector<pthread_key_t> keys;
  ASSERT_NO_FATAL_FAILURE(ExhaustPthreadKeys(&keys));

  MemoryFake* fake = new MemoryFake;
  MemoryThreadCache memory(fake);
  fake->SetMemoryBlock(0x8000, 4096, 0xad);

  // Read the data, which should be uncached.
  uint8_t value;
  ASSERT_TRUE(memory.ReadFully(0x8000, &value, 1));
  ASSERT_EQ(0xad, value);
  ASSERT_TRUE(memory.ReadFully(0x8001, &value, 1));
  ASSERT_EQ(0xad, value);

  // Verify the previous read did not cache anything.
  fake->SetMemoryBlock(0x8000, 4096, 0x12);
  ASSERT_TRUE(memory.ReadFully(0x8000, &value, 1));
  ASSERT_EQ(0x12, value);
  ASSERT_TRUE(memory.ReadFully(0x8001, &value, 1));
  ASSERT_EQ(0x12, value);

  for (pthread_key_t& key : keys) {
    pthread_key_delete(key);
  }
}

TEST_F(MemoryThreadCacheTest, clear_cache_when_no_cache) {
  std::vector<pthread_key_t> keys;
  ASSERT_NO_FATAL_FAILURE(ExhaustPthreadKeys(&keys));

  MemoryFake* fake = new MemoryFake;
  MemoryThreadCache memory(fake);
  memory.Clear();

  for (pthread_key_t& key : keys) {
    pthread_key_delete(key);
  }
}

}  // namespace unwindstack
