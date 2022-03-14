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
#include <sys/mman.h>

#include <thread>

#include <gtest/gtest.h>

#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>

#include "ElfFake.h"

namespace unwindstack {

TEST(MapInfoTest, maps_constructor_const_char) {
  auto prev_map = MapInfo::Create(0, 0, 0, 0, "");
  auto map_info = MapInfo::Create(prev_map, 1, 2, 3, 4, "map");

  EXPECT_EQ(prev_map.get(), map_info->prev_map().get());
  EXPECT_EQ(1UL, map_info->start());
  EXPECT_EQ(2UL, map_info->end());
  EXPECT_EQ(3UL, map_info->offset());
  EXPECT_EQ(4UL, map_info->flags());
  EXPECT_EQ("map", map_info->name());
  EXPECT_EQ(UINT64_MAX, map_info->load_bias());
  EXPECT_EQ(0UL, map_info->object_offset());
  EXPECT_TRUE(map_info->object().get() == nullptr);
}

TEST(MapInfoTest, maps_constructor_string) {
  std::string name("string_map");
  auto prev_map = MapInfo::Create(0, 0, 0, 0, "");
  auto map_info = MapInfo::Create(prev_map, 1, 2, 3, 4, name);

  EXPECT_EQ(prev_map, map_info->prev_map());
  EXPECT_EQ(1UL, map_info->start());
  EXPECT_EQ(2UL, map_info->end());
  EXPECT_EQ(3UL, map_info->offset());
  EXPECT_EQ(4UL, map_info->flags());
  EXPECT_EQ(UINT64_MAX, map_info->load_bias());
  EXPECT_EQ(0UL, map_info->object_offset());
  EXPECT_TRUE(map_info->object().get() == nullptr);
}

TEST(MapInfoTest, real_map_check) {
  auto map1 = MapInfo::Create(0, 0x1000, 0, PROT_READ, "fake.so");
  auto map2 = MapInfo::Create(map1, 0, 0, 0, 0, "");
  auto map3 = MapInfo::Create(map2, 0x1000, 0x2000, 0x1000, PROT_READ | PROT_EXEC, "fake.so");

  EXPECT_EQ(nullptr, map1->prev_map());
  EXPECT_EQ(nullptr, map1->GetPrevRealMap());
  EXPECT_EQ(map2, map1->next_map());
  EXPECT_EQ(map3, map1->GetNextRealMap());

  EXPECT_EQ(map1, map2->prev_map());
  EXPECT_EQ(nullptr, map2->GetPrevRealMap());
  EXPECT_EQ(map3, map2->next_map());
  EXPECT_EQ(nullptr, map2->GetNextRealMap());

  EXPECT_EQ(map2, map3->prev_map());
  EXPECT_EQ(map1, map3->GetPrevRealMap());
  EXPECT_EQ(nullptr, map3->next_map());
  EXPECT_EQ(nullptr, map3->GetNextRealMap());

  // Verify that if the middle map is not blank, then the Get{Next,Prev}RealMap
  // functions return nullptrs.
  map2->set_offset(1);
  EXPECT_EQ(nullptr, map1->GetPrevRealMap());
  EXPECT_EQ(nullptr, map1->GetNextRealMap());
  EXPECT_EQ(nullptr, map3->GetPrevRealMap());
  EXPECT_EQ(nullptr, map3->GetNextRealMap());
  map2->set_offset(0);
  EXPECT_EQ(map3, map1->GetNextRealMap());

  map2->set_flags(1);
  EXPECT_EQ(nullptr, map1->GetPrevRealMap());
  EXPECT_EQ(nullptr, map1->GetNextRealMap());
  EXPECT_EQ(nullptr, map3->GetPrevRealMap());
  EXPECT_EQ(nullptr, map3->GetNextRealMap());
  map2->set_flags(0);
  EXPECT_EQ(map3, map1->GetNextRealMap());

  map2->set_name("something");
  EXPECT_EQ(nullptr, map1->GetPrevRealMap());
  EXPECT_EQ(nullptr, map1->GetNextRealMap());
  EXPECT_EQ(nullptr, map3->GetPrevRealMap());
  EXPECT_EQ(nullptr, map3->GetNextRealMap());
  map2->set_name("");
  EXPECT_EQ(map3, map1->GetNextRealMap());

  // Verify that if the Get{Next,Prev}RealMap names must match.
  map1->set_name("another");
  EXPECT_EQ(nullptr, map1->GetPrevRealMap());
  EXPECT_EQ(nullptr, map1->GetNextRealMap());
  EXPECT_EQ(nullptr, map3->GetPrevRealMap());
  EXPECT_EQ(nullptr, map3->GetNextRealMap());
  map1->set_name("fake.so");
  EXPECT_EQ(map3, map1->GetNextRealMap());

  map3->set_name("another");
  EXPECT_EQ(nullptr, map1->GetPrevRealMap());
  EXPECT_EQ(nullptr, map1->GetNextRealMap());
  EXPECT_EQ(nullptr, map3->GetPrevRealMap());
  EXPECT_EQ(nullptr, map3->GetNextRealMap());
  map3->set_name("fake.so");
  EXPECT_EQ(map3, map1->GetNextRealMap());
}

TEST(MapInfoTest, get_function_name) {
  ElfFake* elf = new ElfFake(nullptr);
  ElfInterfaceFake* interface = new ElfInterfaceFake(nullptr);
  elf->FakeSetInterface(interface);
  interface->FakePushFunctionData(FunctionData("function", 1000));

  auto map_info = MapInfo::Create(1, 2, 3, 4, "");
  map_info->set_object(elf);

  SharedString name;
  uint64_t offset;
  ASSERT_TRUE(map_info->GetFunctionName(1000, &name, &offset));
  EXPECT_EQ("function", name);
  EXPECT_EQ(1000UL, offset);
}

TEST(MapInfoTest, multiple_thread_get_object_fields) {
  auto map_info = MapInfo::Create(0, 0, 0, 0, "");

  static constexpr size_t kNumConcurrentThreads = 100;
  MapInfo::ObjectFields* object_fields[kNumConcurrentThreads];

  std::atomic_bool wait;
  wait = true;
  // Create all of the threads and have them do the call at the same time
  // to make it likely that a race will occur.
  std::vector<std::thread*> threads;
  for (size_t i = 0; i < kNumConcurrentThreads; i++) {
    std::thread* thread = new std::thread([i, &wait, &map_info, &object_fields]() {
      while (wait)
        ;
      object_fields[i] = &map_info->GetObjectFields();
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
  MapInfo::ObjectFields* expected_object_fields = &map_info->GetObjectFields();
  ASSERT_TRUE(expected_object_fields != nullptr);
  for (size_t i = 0; i < kNumConcurrentThreads; i++) {
    EXPECT_EQ(expected_object_fields, object_fields[i]) << "Thread " << i << " mismatched.";
  }
}

TEST(MapInfoTest, object_file_not_readable) {
  auto map_info_readable = MapInfo::Create(0, 0x1000, 0, PROT_READ, "fake.so");
  map_info_readable->set_memory_backed_object(true);
  ASSERT_TRUE(map_info_readable->ObjectFileNotReadable());

  auto map_info_no_name = MapInfo::Create(0, 0x1000, 0, PROT_READ, "");
  map_info_no_name->set_memory_backed_object(true);
  ASSERT_FALSE(map_info_no_name->ObjectFileNotReadable());

  auto map_info_bracket = MapInfo::Create(0, 0x2000, 0, PROT_READ, "[vdso]");
  map_info_bracket->set_memory_backed_object(true);
  ASSERT_FALSE(map_info_bracket->ObjectFileNotReadable());

  auto map_info_memfd = MapInfo::Create(0, 0x3000, 0, PROT_READ, "/memfd:jit-cache");
  map_info_memfd->set_memory_backed_object(true);
  ASSERT_FALSE(map_info_memfd->ObjectFileNotReadable());
}

}  // namespace unwindstack
