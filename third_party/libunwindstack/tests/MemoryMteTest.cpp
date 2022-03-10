/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <sys/mman.h>
#include <sys/types.h>

#include <gtest/gtest.h>

#ifdef __ANDROID__
#include <bionic/mte.h>
#endif

#include "MemoryLocal.h"
#include "MemoryRemote.h"
#include "PidUtils.h"
#include "TestUtils.h"

namespace unwindstack {

static uintptr_t CreateTagMapping() {
#if defined(__aarch64__)
  uintptr_t mapping =
      reinterpret_cast<uintptr_t>(mmap(nullptr, getpagesize(), PROT_READ | PROT_WRITE | PROT_MTE,
                                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  if (reinterpret_cast<void*>(mapping) == MAP_FAILED) {
    return 0;
  }
  __asm__ __volatile__(".arch_extension mte; stg %0, [%0]"
                       :
                       : "r"(mapping + (1ULL << 56))
                       : "memory");
  return mapping;
#else
  return 0;
#endif
}

TEST(MemoryMteTest, remote_read_tag) {
#if !defined(__aarch64__)
  GTEST_SKIP() << "Requires aarch64";
#else
  if (!mte_supported()) {
    GTEST_SKIP() << "Requires MTE";
  }
#endif

  uintptr_t mapping = CreateTagMapping();
  ASSERT_NE(0U, mapping);

  pid_t pid;
  if ((pid = fork()) == 0) {
    while (true)
      ;
    exit(1);
  }
  ASSERT_LT(0, pid);
  TestScopedPidReaper reap(pid);

  ASSERT_TRUE(Attach(pid));

  MemoryRemote remote(pid);

  EXPECT_EQ(1, remote.ReadTag(mapping));
  EXPECT_EQ(0, remote.ReadTag(mapping + 16));

  ASSERT_TRUE(Detach(pid));
}

TEST(MemoryMteTest, local_read_tag) {
#if !defined(__aarch64__)
  GTEST_SKIP() << "Requires aarch64";
#else
  if (!mte_supported()) {
    GTEST_SKIP() << "Requires MTE";
  }
#endif

  uintptr_t mapping = CreateTagMapping();
  ASSERT_NE(0U, mapping);

  MemoryLocal local;

  EXPECT_EQ(1, local.ReadTag(mapping));
  EXPECT_EQ(0, local.ReadTag(mapping + 16));
}

}  // namespace unwindstack
