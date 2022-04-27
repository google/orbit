/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <android-base/file.h>
#include <unwindstack/Maps.h>

namespace unwindstack {

class TestUpdatableMaps : public LocalUpdatableMaps {
 public:
  TestUpdatableMaps() : LocalUpdatableMaps() {}
  virtual ~TestUpdatableMaps() = default;

  const std::string GetMapsFile() const override { return maps_file_; }

  void TestSetMapsFile(const std::string& maps_file) { maps_file_ = maps_file; }

 private:
  std::string maps_file_;
};

class LocalUpdatableMapsTest : public ::testing::Test {
 protected:
  static const std::string GetDefaultMapString() {
    return "3000-4000 r-xp 00000 00:00 0\n8000-9000 r-xp 00000 00:00 0\n";
  }

  void SetUp() override {
    TemporaryFile tf;
    ASSERT_TRUE(android::base::WriteStringToFile(GetDefaultMapString(), tf.path));

    maps_.TestSetMapsFile(tf.path);
    ASSERT_TRUE(maps_.Parse());
    ASSERT_EQ(2U, maps_.Total());

    auto map_info = maps_.Get(0);
    ASSERT_TRUE(map_info != nullptr);
    EXPECT_EQ(0x3000U, map_info->start());
    EXPECT_EQ(0x4000U, map_info->end());
    EXPECT_EQ(0U, map_info->offset());
    EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
    EXPECT_TRUE(map_info->name().empty());

    map_info = maps_.Get(1);
    ASSERT_TRUE(map_info != nullptr);
    EXPECT_EQ(0x8000U, map_info->start());
    EXPECT_EQ(0x9000U, map_info->end());
    EXPECT_EQ(0U, map_info->offset());
    EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
    EXPECT_TRUE(map_info->name().empty());
  }

  TestUpdatableMaps maps_;
};

TEST_F(LocalUpdatableMapsTest, same_map) {
  TemporaryFile tf;
  ASSERT_TRUE(android::base::WriteStringToFile(GetDefaultMapString(), tf.path));

  maps_.TestSetMapsFile(tf.path);
  ASSERT_TRUE(maps_.Reparse());
  ASSERT_EQ(2U, maps_.Total());

  auto map_info = maps_.Get(0);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x3000U, map_info->start());
  EXPECT_EQ(0x4000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());
  EXPECT_EQ(nullptr, map_info->prev_map());
  EXPECT_EQ(maps_.Get(1), map_info->next_map());

  map_info = maps_.Get(1);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x8000U, map_info->start());
  EXPECT_EQ(0x9000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());
  EXPECT_EQ(maps_.Get(0), map_info->prev_map());
  EXPECT_EQ(nullptr, map_info->next_map());
}

TEST_F(LocalUpdatableMapsTest, same_map_new_perms) {
  TemporaryFile tf;
  ASSERT_TRUE(
      android::base::WriteStringToFile("3000-4000 rwxp 00000 00:00 0\n"
                                       "8000-9000 r-xp 00000 00:00 0\n",
                                       tf.path));

  maps_.TestSetMapsFile(tf.path);
  ASSERT_TRUE(maps_.Reparse());
  ASSERT_EQ(2U, maps_.Total());

  auto map_info = maps_.Get(0);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x3000U, map_info->start());
  EXPECT_EQ(0x4000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());
  EXPECT_EQ(nullptr, map_info->prev_map());
  EXPECT_EQ(maps_.Get(1), map_info->next_map());

  map_info = maps_.Get(1);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x8000U, map_info->start());
  EXPECT_EQ(0x9000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());
  EXPECT_EQ(maps_.Get(0), map_info->prev_map());
  EXPECT_EQ(nullptr, map_info->next_map());
}

TEST_F(LocalUpdatableMapsTest, same_map_new_name) {
  TemporaryFile tf;
  ASSERT_TRUE(
      android::base::WriteStringToFile("3000-4000 r-xp 00000 00:00 0 /fake/lib.so\n"
                                       "8000-9000 r-xp 00000 00:00 0\n",
                                       tf.path));

  maps_.TestSetMapsFile(tf.path);
  ASSERT_TRUE(maps_.Reparse());
  ASSERT_EQ(2U, maps_.Total());

  auto map_info = maps_.Get(0);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x3000U, map_info->start());
  EXPECT_EQ(0x4000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_EQ("/fake/lib.so", map_info->name());
  EXPECT_EQ(nullptr, map_info->prev_map());
  EXPECT_EQ(maps_.Get(1), map_info->next_map());

  map_info = maps_.Get(1);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x8000U, map_info->start());
  EXPECT_EQ(0x9000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());
  EXPECT_EQ(maps_.Get(0), map_info->prev_map());
  EXPECT_EQ(nullptr, map_info->next_map());
}

TEST_F(LocalUpdatableMapsTest, only_add_maps) {
  TemporaryFile tf;
  ASSERT_TRUE(
      android::base::WriteStringToFile("1000-2000 r-xp 00000 00:00 0\n"
                                       "3000-4000 r-xp 00000 00:00 0\n"
                                       "8000-9000 r-xp 00000 00:00 0\n"
                                       "a000-f000 r-xp 00000 00:00 0\n",
                                       tf.path));

  maps_.TestSetMapsFile(tf.path);
  ASSERT_TRUE(maps_.Reparse());
  ASSERT_EQ(4U, maps_.Total());

  auto map_info = maps_.Get(0);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x1000U, map_info->start());
  EXPECT_EQ(0x2000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());
  EXPECT_EQ(nullptr, map_info->prev_map());
  EXPECT_EQ(maps_.Get(1), map_info->next_map());

  map_info = maps_.Get(1);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x3000U, map_info->start());
  EXPECT_EQ(0x4000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());
  EXPECT_EQ(maps_.Get(0), map_info->prev_map());
  EXPECT_EQ(maps_.Get(2), map_info->next_map());

  map_info = maps_.Get(2);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x8000U, map_info->start());
  EXPECT_EQ(0x9000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());
  EXPECT_EQ(maps_.Get(1), map_info->prev_map());
  EXPECT_EQ(maps_.Get(3), map_info->next_map());

  map_info = maps_.Get(3);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0xa000U, map_info->start());
  EXPECT_EQ(0xf000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());
  EXPECT_EQ(maps_.Get(2), map_info->prev_map());
  EXPECT_EQ(nullptr, map_info->next_map());
}

TEST_F(LocalUpdatableMapsTest, all_new_maps) {
  TemporaryFile tf;
  ASSERT_TRUE(
      android::base::WriteStringToFile("1000-2000 r-xp 00000 00:00 0\n"
                                       "a000-f000 r-xp 00000 00:00 0\n",
                                       tf.path));

  maps_.TestSetMapsFile(tf.path);
  ASSERT_TRUE(maps_.Reparse());
  ASSERT_EQ(2U, maps_.Total());

  auto map_info = maps_.Get(0);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x1000U, map_info->start());
  EXPECT_EQ(0x2000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());
  EXPECT_EQ(nullptr, map_info->prev_map());
  EXPECT_EQ(maps_.Get(1), map_info->next_map());

  map_info = maps_.Get(1);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0xa000U, map_info->start());
  EXPECT_EQ(0xf000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());
  EXPECT_EQ(maps_.Get(0), map_info->prev_map());
  EXPECT_EQ(nullptr, map_info->next_map());
}

TEST_F(LocalUpdatableMapsTest, add_map_prev_name_updated) {
  TemporaryFile tf;
  ASSERT_TRUE(
      android::base::WriteStringToFile("3000-4000 rwxp 00000 00:00 0\n"
                                       "8000-9000 r-xp 00000 00:00 0\n"
                                       "9000-a000 r-xp 00000 00:00 0\n",
                                       tf.path));

  maps_.TestSetMapsFile(tf.path);
  ASSERT_TRUE(maps_.Reparse());
  ASSERT_EQ(3U, maps_.Total());

  auto map_info = maps_.Get(2);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x9000U, map_info->start());
  EXPECT_EQ(0xA000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_TRUE(map_info->name().empty());

  // Check all of the prev and next pointers.
  EXPECT_EQ(nullptr, maps_.Get(0)->prev_map());
  EXPECT_EQ(maps_.Get(1), maps_.Get(0)->next_map());
  EXPECT_EQ(maps_.Get(0), maps_.Get(1)->prev_map());
  EXPECT_EQ(maps_.Get(2), maps_.Get(1)->next_map());
  EXPECT_EQ(maps_.Get(1), maps_.Get(2)->prev_map());
  EXPECT_EQ(nullptr, maps_.Get(2)->next_map());
}

TEST_F(LocalUpdatableMapsTest, add_map_prev_real_name_updated) {
  TemporaryFile tf;
  ASSERT_TRUE(
      android::base::WriteStringToFile("3000-4000 r-xp 00000 00:00 0 /fake/lib.so\n"
                                       "4000-5000 ---p 00000 00:00 0\n"
                                       "7000-8000 r-xp 00000 00:00 0 /fake/lib.so\n"
                                       "8000-9000 ---p 00000 00:00 0\n",
                                       tf.path));

  maps_.TestSetMapsFile(tf.path);
  bool any_changed;
  ASSERT_TRUE(maps_.Reparse(&any_changed));
  ASSERT_EQ(4U, maps_.Total());
  ASSERT_TRUE(any_changed);

  // Try again and assert there are no changes this time.
  ASSERT_TRUE(maps_.Reparse(&any_changed));
  ASSERT_EQ(4U, maps_.Total());
  ASSERT_FALSE(any_changed);

  auto map_info = maps_.Get(2);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x7000U, map_info->start());
  EXPECT_EQ(0x8000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_EQ(maps_.Get(0), map_info->GetPrevRealMap());
  EXPECT_EQ(maps_.Get(1), map_info->prev_map());
  EXPECT_EQ(maps_.Get(3), map_info->next_map());
  EXPECT_EQ("/fake/lib.so", map_info->name());

  map_info = maps_.Get(3);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x8000U, map_info->start());
  EXPECT_EQ(0x9000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_TRUE(map_info->IsBlank());
  EXPECT_EQ(nullptr, map_info->GetPrevRealMap());
  EXPECT_EQ(maps_.Get(2), map_info->prev_map());
  EXPECT_EQ(maps_.Get(4), map_info->next_map());
  EXPECT_TRUE(map_info->name().empty());

  ASSERT_TRUE(
      android::base::WriteStringToFile("3000-4000 r-xp 00000 00:00 0 /fake/lib.so\n"
                                       "4000-5000 ---p 00000 00:00 0\n"
                                       "7000-8000 r-xp 00000 00:00 0 /fake/lib.so\n"
                                       "8000-9000 ---p 00000 00:00 0\n"
                                       "9000-a000 r-xp 00000 00:00 0 /fake/lib2.so\n"
                                       "a000-b000 r-xp 00000 00:00 0 /fake/lib3.so\n",
                                       tf.path));

  maps_.TestSetMapsFile(tf.path);
  ASSERT_TRUE(maps_.Reparse(&any_changed));
  ASSERT_EQ(6U, maps_.Total());
  ASSERT_TRUE(any_changed);

  // Try again and assert there are no changes this time.
  ASSERT_TRUE(maps_.Reparse(&any_changed));
  ASSERT_EQ(6U, maps_.Total());
  ASSERT_FALSE(any_changed);

  map_info = maps_.Get(2);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x7000U, map_info->start());
  EXPECT_EQ(0x8000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_EQ("/fake/lib.so", map_info->name());
  EXPECT_EQ(maps_.Get(1), map_info->prev_map());
  EXPECT_EQ(maps_.Get(0), map_info->GetPrevRealMap());
  EXPECT_EQ(maps_.Get(3), map_info->next_map());

  map_info = maps_.Get(4);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0x9000U, map_info->start());
  EXPECT_EQ(0xA000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_EQ("/fake/lib2.so", map_info->name());
  EXPECT_EQ(maps_.Get(3), map_info->prev_map());
  EXPECT_EQ(nullptr, map_info->GetPrevRealMap());
  EXPECT_EQ(maps_.Get(5), map_info->next_map());

  map_info = maps_.Get(5);
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_EQ(0xA000U, map_info->start());
  EXPECT_EQ(0xB000U, map_info->end());
  EXPECT_EQ(0U, map_info->offset());
  EXPECT_EQ(PROT_READ | PROT_EXEC, map_info->flags());
  EXPECT_EQ("/fake/lib3.so", map_info->name());
  EXPECT_EQ(maps_.Get(4), map_info->prev_map());
  EXPECT_EQ(nullptr, map_info->GetPrevRealMap());
  EXPECT_EQ(nullptr, map_info->next_map());
}

}  // namespace unwindstack
