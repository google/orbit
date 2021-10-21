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

#include <elf.h>
#include <stdint.h>

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include <unwindstack/Arch.h>
#include <unwindstack/Elf.h>
#include <unwindstack/Memory.h>

#include "GlobalDebugImpl.h"

namespace unwindstack {

TEST(GlobalDebugImplTest, strip_address_tag_non_arm64) {
  std::shared_ptr<Memory> memory;
  std::vector<std::string> libs;
  GlobalDebugImpl<Elf, uint64_t, Uint64_P> debug(ARCH_X86_64, memory, libs, nullptr);

  EXPECT_EQ(0UL, debug.StripAddressTag(0));
  EXPECT_EQ(0x1234567812345678UL, debug.StripAddressTag(0x1234567812345678UL));
}

TEST(GlobalDebugImplTest, strip_address_tag_arm64) {
  std::shared_ptr<Memory> memory;
  std::vector<std::string> libs;
  GlobalDebugImpl<Elf, uint64_t, Uint64_P> debug(ARCH_ARM64, memory, libs, nullptr);

  EXPECT_EQ(0UL, debug.StripAddressTag(0));
  EXPECT_EQ(0x34567812345678UL, debug.StripAddressTag(0x1234567812345678UL));

  // Verify value is sign-extended.
  EXPECT_EQ(0xfff4567812345678UL, debug.StripAddressTag(0x00f4567812345678UL));
}

}  // namespace unwindstack
