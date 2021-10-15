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

#include <stdint.h>

#include <thread>

#include <gtest/gtest.h>

#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>

#include "ElfFake.h"

namespace unwindstack {

TEST(MapInfoTest, maps_constructor_const_char) {
  MapInfo prev_map(nullptr, nullptr, 0, 0, 0, 0, "");
  MapInfo map_info(&prev_map, &prev_map, 1, 2, 3, 4, "map");

  EXPECT_EQ(&prev_map, map_info.prev_map());
  EXPECT_EQ(1UL, map_info.start());
  EXPECT_EQ(2UL, map_info.end());
  EXPECT_EQ(3UL, map_info.offset());
  EXPECT_EQ(4UL, map_info.flags());
  EXPECT_EQ("map", map_info.name());
  EXPECT_EQ(INT64_MAX, map_info.load_bias());
  EXPECT_EQ(0UL, map_info.elf_offset());
  EXPECT_TRUE(map_info.elf().get() == nullptr);
}

TEST(MapInfoTest, maps_constructor_string) {
  std::string name("string_map");
  MapInfo prev_map(nullptr, nullptr, 0, 0, 0, 0, "");
  MapInfo map_info(&prev_map, &prev_map, 1, 2, 3, 4, name);

  EXPECT_EQ(&prev_map, map_info.prev_map());
  EXPECT_EQ(1UL, map_info.start());
  EXPECT_EQ(2UL, map_info.end());
  EXPECT_EQ(3UL, map_info.offset());
  EXPECT_EQ(4UL, map_info.flags());
  EXPECT_EQ("string_map", map_info.name());
  EXPECT_EQ(INT64_MAX, map_info.load_bias());
  EXPECT_EQ(0UL, map_info.elf_offset());
  EXPECT_TRUE(map_info.elf().get() == nullptr);
}

TEST(MapInfoTest, get_function_name) {
  ElfFake* elf = new ElfFake(nullptr);
  ElfInterfaceFake* interface = new ElfInterfaceFake(nullptr);
  elf->FakeSetInterface(interface);
  interface->FakePushFunctionData(FunctionData("function", 1000));

  MapInfo map_info(nullptr, nullptr, 1, 2, 3, 4, "");
  map_info.set_elf(elf);

  SharedString name;
  uint64_t offset;
  ASSERT_TRUE(map_info.GetFunctionName(1000, &name, &offset));
  EXPECT_EQ("function", name);
  EXPECT_EQ(1000UL, offset);
}

TEST(MapInfoTest, multiple_thread_get_elf_fields) {
  MapInfo map_info(nullptr, nullptr, 0, 0, 0, 0, "");

  static constexpr size_t kNumConcurrentThreads = 100;
  MapInfo::ElfFields* elf_fields[kNumConcurrentThreads];

  std::atomic_bool wait;
  wait = true;
  // Create all of the threads and have them do the call at the same time
  // to make it likely that a race will occur.
  std::vector<std::thread*> threads;
  for (size_t i = 0; i < kNumConcurrentThreads; i++) {
    std::thread* thread = new std::thread([i, &wait, &map_info, &elf_fields]() {
      while (wait)
        ;
      elf_fields[i] = &map_info.GetElfFields();
    });
    threads.push_back(thread);
  }

  // Set them all going and wait for the threads to finish.
  wait = false;
  for (auto thread : threads) {
    thread->join();
    delete thread;
  }

  // Now verify that all of elf fields are exactly the same and valid.
  MapInfo::ElfFields* expected_elf_fields = &map_info.GetElfFields();
  ASSERT_TRUE(expected_elf_fields != nullptr);
  for (size_t i = 0; i < kNumConcurrentThreads; i++) {
    EXPECT_EQ(expected_elf_fields, elf_fields[i]) << "Thread " << i << " mismatched.";
  }
}

}  // namespace unwindstack
