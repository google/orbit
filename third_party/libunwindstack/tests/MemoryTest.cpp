/*
 * Copyright (C) 2017 The Android Open Source Project
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
#include <string.h>

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <unwindstack/Memory.h>

#include "utils/MemoryFake.h"

namespace unwindstack {

TEST(MemoryTest, read32) {
  MemoryFakeAlwaysReadZero memory;

  uint32_t data = 0xffffffff;
  ASSERT_TRUE(memory.Read32(0, &data));
  ASSERT_EQ(0U, data);
}

TEST(MemoryTest, read64) {
  MemoryFakeAlwaysReadZero memory;

  uint64_t data = 0xffffffffffffffffULL;
  ASSERT_TRUE(memory.Read64(0, &data));
  ASSERT_EQ(0U, data);
}

struct FakeStruct {
  int one;
  bool two;
  uint32_t three;
  uint64_t four;
};

TEST(MemoryTest, read_string) {
  std::string name("string_in_memory");

  MemoryFake memory;

  memory.SetMemory(100, name.c_str(), name.size() + 1);

  std::string dst_name;
  ASSERT_TRUE(memory.ReadString(100, &dst_name, 100));
  ASSERT_EQ("string_in_memory", dst_name);

  ASSERT_TRUE(memory.ReadString(107, &dst_name, 100));
  ASSERT_EQ("in_memory", dst_name);

  // Set size greater than string.
  ASSERT_TRUE(memory.ReadString(107, &dst_name, 10));
  ASSERT_EQ("in_memory", dst_name);

  ASSERT_FALSE(memory.ReadString(107, &dst_name, 9));
}

TEST(MemoryTest, read_string_error) {
  std::string name("short");

  MemoryFake memory;

  // Save everything except the terminating '\0'.
  memory.SetMemory(0, name.c_str(), name.size());

  std::string dst_name;
  // Read from a non-existant address.
  ASSERT_FALSE(memory.ReadString(100, &dst_name, 100));

  // This should fail because there is no terminating '\0'.
  ASSERT_FALSE(memory.ReadString(0, &dst_name, 100));

  // This should pass because there is a terminating '\0'.
  memory.SetData8(name.size(), '\0');
  ASSERT_TRUE(memory.ReadString(0, &dst_name, 100));
  ASSERT_EQ("short", dst_name);
}

TEST(MemoryTest, read_string_long) {
  // This string should be greater than 768 characters long (greater than 3 times
  // the buffer in the ReadString function) to read multiple blocks.
  static constexpr char kLongString[] =
      "one two three four five six seven eight nine ten eleven twelve thirteen fourteen fifteen "
      "sixteen seventeen eightteen nineteen twenty twenty-one twenty-two twenty-three twenty-four "
      "twenty-five twenty-six twenty-seven twenty-eight twenty-nine thirty thirty-one thirty-two "
      "thirty-three thirty-four thirty-five thirty-six thirty-seven thirty-eight thirty-nine forty "
      "forty-one forty-two forty-three forty-four forty-five forty-size forty-seven forty-eight "
      "forty-nine fifty fifty-one fifty-two fifty-three fifty-four fifty-five fifty-six "
      "fifty-seven fifty-eight fifty-nine sixty sixty-one sixty-two sixty-three sixty-four "
      "sixty-five sixty-six sixty-seven sixty-eight sixty-nine seventy seventy-one seventy-two "
      "seventy-three seventy-four seventy-five seventy-six seventy-seven seventy-eight "
      "seventy-nine eighty";

  MemoryFake memory;

  memory.SetMemory(100, kLongString, sizeof(kLongString));

  std::string dst_name;
  ASSERT_TRUE(memory.ReadString(100, &dst_name, sizeof(kLongString)));
  ASSERT_EQ(kLongString, dst_name);

  std::string expected_str(kLongString, 255);
  memory.SetMemory(100, expected_str.data(), expected_str.length() + 1);
  ASSERT_TRUE(memory.ReadString(100, &dst_name, 256));
  ASSERT_EQ(expected_str, dst_name);
  ASSERT_FALSE(memory.ReadString(100, &dst_name, 255));

  expected_str = std::string(kLongString, 256);
  memory.SetMemory(100, expected_str.data(), expected_str.length() + 1);
  ASSERT_TRUE(memory.ReadString(100, &dst_name, 257));
  ASSERT_EQ(expected_str, dst_name);
  ASSERT_FALSE(memory.ReadString(100, &dst_name, 256));

  expected_str = std::string(kLongString, 299);
  memory.SetMemory(100, expected_str.data(), expected_str.length() + 1);
  ASSERT_TRUE(memory.ReadString(100, &dst_name, 300));
  ASSERT_EQ(expected_str, dst_name);
}

}  // namespace unwindstack
