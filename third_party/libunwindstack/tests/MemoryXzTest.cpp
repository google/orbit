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

#include <android-base/file.h>

#include <gtest/gtest.h>

#include <MemoryBuffer.h>
#include <MemoryXz.h>

namespace unwindstack {

class MemoryXzTest : public ::testing::Test {
 protected:
  void SetUp() override { expected_content_ = ReadFile("boot_arm.oat.gnu_debugdata"); }

  static std::unique_ptr<MemoryBuffer> ReadFile(const char* filename) {
    std::string dir = android::base::GetExecutableDirectory() + "/tests/files/";
    std::string data;  // NB: It is actually binary data.
    EXPECT_TRUE(android::base::ReadFileToString(dir + filename, &data)) << filename;
    EXPECT_GT(data.size(), 0u);
    auto memory = std::make_unique<MemoryBuffer>();
    EXPECT_TRUE(memory->Resize(data.size()));
    memcpy(memory->GetPtr(0), data.data(), data.size());
    return memory;
  }

  void VerifyContent(MemoryXz& xz, uint64_t offset, uint64_t size) {
    EXPECT_EQ(xz.Size(), expected_content_->Size());
    EXPECT_LE(offset + size, expected_content_->Size());
    std::vector<uint8_t> seen_content(size);
    xz.ReadFully(offset, seen_content.data(), size);
    EXPECT_EQ(memcmp(seen_content.data(), expected_content_->GetPtr(offset), size), 0);
  }

  std::unique_ptr<MemoryBuffer> expected_content_;
};

// Test the expected random-accessible format.
TEST_F(MemoryXzTest, Decompress) {
  auto compressed = ReadFile("boot_arm.oat.gnu_debugdata.xz");
  MemoryXz xz(compressed.get(), 0, compressed->Size(), "boot_arm.oat");
  EXPECT_TRUE(xz.Init());
  EXPECT_GT(xz.BlockCount(), 1u);
  EXPECT_EQ(xz.BlockSize(), 16 * 1024u);
  EXPECT_EQ(xz.MemoryUsage(), 0u);
  VerifyContent(xz, 0, expected_content_->Size());
  EXPECT_EQ(xz.MemoryUsage(), xz.Size());
}

// Test one big monolithic compressed block.
TEST_F(MemoryXzTest, DecompressOneBlock) {
  auto compressed = ReadFile("boot_arm.oat.gnu_debugdata.xz.one-block");
  MemoryXz xz(compressed.get(), 0, compressed->Size(), "boot_arm.oat");
  EXPECT_TRUE(xz.Init());
  EXPECT_EQ(xz.BlockCount(), 1u);
  EXPECT_GT(xz.BlockSize(), xz.Size());
  EXPECT_EQ(xz.MemoryUsage(), 0u);
  VerifyContent(xz, 0, expected_content_->Size());
  EXPECT_EQ(xz.MemoryUsage(), xz.Size());
}

// Test fallback (non-consistent block sizes).
TEST_F(MemoryXzTest, DecompressOddSizes) {
  auto compressed = ReadFile("boot_arm.oat.gnu_debugdata.xz.odd-sizes");
  MemoryXz xz(compressed.get(), 0, compressed->Size(), "boot_arm.oat");
  EXPECT_TRUE(xz.Init());
  EXPECT_EQ(xz.BlockCount(), 1u);
  EXPECT_GT(xz.BlockSize(), xz.Size());
  EXPECT_EQ(xz.MemoryUsage(), xz.Size());
  VerifyContent(xz, 0, expected_content_->Size());
}

// Test fallback (non-power-of-2 block size).
TEST_F(MemoryXzTest, DecompressNonPower) {
  auto compressed = ReadFile("boot_arm.oat.gnu_debugdata.xz.non-power");
  MemoryXz xz(compressed.get(), 0, compressed->Size(), "boot_arm.oat");
  EXPECT_TRUE(xz.Init());
  EXPECT_EQ(xz.BlockCount(), 1u);
  EXPECT_GT(xz.BlockSize(), xz.Size());
  EXPECT_EQ(xz.MemoryUsage(), xz.Size());
  VerifyContent(xz, 0, expected_content_->Size());
}

// Read first byte of some blocks.
TEST_F(MemoryXzTest, ReadFirstByte) {
  auto compressed = ReadFile("boot_arm.oat.gnu_debugdata.xz");
  MemoryXz xz(compressed.get(), 0, compressed->Size(), "boot_arm.oat");
  EXPECT_TRUE(xz.Init());
  EXPECT_GT(xz.BlockCount(), 1u);
  EXPECT_EQ(xz.BlockSize(), 16 * 1024u);
  for (size_t i = 0; i < xz.BlockCount(); i += 3) {
    VerifyContent(xz, i * xz.BlockSize(), 1);
  }
  EXPECT_LT(xz.MemoryUsage(), xz.Size());  // We didn't decompress all blocks.
}

// Read last byte of some blocks.
TEST_F(MemoryXzTest, ReadLastByte) {
  auto compressed = ReadFile("boot_arm.oat.gnu_debugdata.xz");
  MemoryXz xz(compressed.get(), 0, compressed->Size(), "boot_arm.oat");
  EXPECT_TRUE(xz.Init());
  EXPECT_GT(xz.BlockCount(), 1u);
  EXPECT_EQ(xz.BlockSize(), 16 * 1024u);
  for (size_t i = 1; i < xz.BlockCount(); i += 3) {
    VerifyContent(xz, i * xz.BlockSize() - 1, 1);
  }
  EXPECT_LT(xz.MemoryUsage(), xz.Size());  // We didn't decompress all blocks.
}

// Read across boundary of blocks.
TEST_F(MemoryXzTest, ReadBoundary) {
  auto compressed = ReadFile("boot_arm.oat.gnu_debugdata.xz");
  MemoryXz xz(compressed.get(), 0, compressed->Size(), "boot_arm.oat");
  EXPECT_TRUE(xz.Init());
  EXPECT_GT(xz.BlockCount(), 1u);
  EXPECT_EQ(xz.BlockSize(), 16 * 1024u);
  for (size_t i = 1; i < xz.BlockCount(); i += 3) {
    VerifyContent(xz, i * xz.BlockSize() - 1, 2);
  }
  EXPECT_LT(xz.MemoryUsage(), xz.Size());  // We didn't decompress all blocks.
}

}  // namespace unwindstack
