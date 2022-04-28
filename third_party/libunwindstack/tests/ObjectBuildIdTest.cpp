/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <gtest/gtest.h>

#include <unwindstack/Object.h>
#include <string>

namespace unwindstack {

TEST(ObjectBuildIdTest, get_printable_build_id_empty) {
  std::string empty;
  ASSERT_EQ("", Object::GetPrintableBuildID(empty));
}

TEST(ObjectBuildIdTest, get_printable_build_id_check) {
  std::string empty = {'\xff', '\x45', '\x40', '\x0f'};
  ASSERT_EQ("ff45400f", Object::GetPrintableBuildID(empty));
}

}  // namespace unwindstack
