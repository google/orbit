// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>
#include <sys/mman.h>

#include <memory>
#include <string>
#include <string_view>

#include "LibunwindstackMaps.h"
#include "unwindstack/MapInfo.h"
#include "unwindstack/Maps.h"
#include "unwindstack/SharedString.h"

namespace orbit_linux_tracing {

static constexpr const char* kMapsInitialContent =
    "101000-104000 r--p 00001000 01:02 42    /path/to/file\n"
    "104000-107000 r-xp 00000000 00:00 00\n"
    "200000-210000 rw-p 00000000 00:00 00    [stack]\n";

static testing::Matcher<unwindstack::MapInfo*> MapInfoEq(
    uint64_t start, uint64_t end, uint64_t offset, uint64_t flags, std::string_view name,
    const std::shared_ptr<unwindstack::MapInfo>& prev_map,
    const std::shared_ptr<unwindstack::MapInfo>& next_map) {
  return testing::AllOf(
      testing::Property("start", &unwindstack::MapInfo::start, start),
      testing::Property("end", &unwindstack::MapInfo::end, end),
      testing::Property("offset", &unwindstack::MapInfo::offset, offset),
      testing::Property("flags", &unwindstack::MapInfo::flags, flags),
      // Can't use Property because MapInfo::name() is not const.
      testing::ResultOf(
          [](unwindstack::MapInfo* map_info) { return std::string{map_info->name()}; }, name),
      testing::Property("prev_map", &unwindstack::MapInfo::prev_map, prev_map),
      testing::Property("next_map", &unwindstack::MapInfo::next_map, next_map));
}

TEST(LibunwindstackMaps, ParseMaps) {
  std::unique_ptr<LibunwindstackMaps> libunwindstack_maps =
      LibunwindstackMaps::ParseMaps(kMapsInitialContent);
  ASSERT_NE(libunwindstack_maps, nullptr);
  unwindstack::Maps* maps = libunwindstack_maps->Get();
  ASSERT_NE(maps, nullptr);
  ASSERT_EQ(maps->Total(), 3);

  EXPECT_THAT(maps->Get(0).get(), MapInfoEq(0x101000, 0x104000, 0x1000, PROT_READ, "/path/to/file",
                                            nullptr, maps->Get(1)));
  EXPECT_THAT(maps->Get(1).get(), MapInfoEq(0x104000, 0x107000, 0, PROT_READ | PROT_EXEC, "",
                                            maps->Get(0), maps->Get(2)));
  EXPECT_THAT(maps->Get(2).get(), MapInfoEq(0x200000, 0x210000, 0, PROT_READ | PROT_WRITE,
                                            "[stack]", maps->Get(1), nullptr));
}

TEST(LibunwindstackMaps, Find) {
  std::unique_ptr<LibunwindstackMaps> libunwindstack_maps =
      LibunwindstackMaps::ParseMaps(kMapsInitialContent);
  ASSERT_NE(libunwindstack_maps, nullptr);
  unwindstack::Maps* maps = libunwindstack_maps->Get();
  ASSERT_NE(maps, nullptr);

  EXPECT_EQ(libunwindstack_maps->Find(0x101000 - 1), nullptr);
  EXPECT_EQ(libunwindstack_maps->Find(0x101000), maps->Get(0));
  EXPECT_EQ(libunwindstack_maps->Find(0x104000), maps->Get(1));
  EXPECT_EQ(libunwindstack_maps->Find(0x107000), nullptr);
}

TEST(LibunwindstackMaps, AddAndSortNotOverlappingAnyExistingMap) {
  std::unique_ptr<LibunwindstackMaps> libunwindstack_maps =
      LibunwindstackMaps::ParseMaps(kMapsInitialContent);
  ASSERT_NE(libunwindstack_maps, nullptr);

  libunwindstack_maps->AddAndSort(0x107000, 0x200000, 0x7000, PROT_READ | PROT_WRITE,
                                  "/path/to/newfile");
  libunwindstack_maps->AddAndSort(0x210000, 0x211000, 0, PROT_READ, "");
  unwindstack::Maps* maps = libunwindstack_maps->Get();
  ASSERT_NE(maps, nullptr);
  ASSERT_EQ(maps->Total(), 5);

  EXPECT_THAT(maps->Get(0).get(), MapInfoEq(0x101000, 0x104000, 0x1000, PROT_READ, "/path/to/file",
                                            nullptr, maps->Get(1)));
  EXPECT_THAT(maps->Get(1).get(), MapInfoEq(0x104000, 0x107000, 0, PROT_READ | PROT_EXEC, "",
                                            maps->Get(0), maps->Get(2)));
  EXPECT_THAT(maps->Get(2).get(), MapInfoEq(0x107000, 0x200000, 0x7000, PROT_READ | PROT_WRITE,
                                            "/path/to/newfile", maps->Get(1), maps->Get(3)));
  EXPECT_THAT(maps->Get(3).get(), MapInfoEq(0x200000, 0x210000, 0, PROT_READ | PROT_WRITE,
                                            "[stack]", maps->Get(2), maps->Get(4)));
  EXPECT_THAT(maps->Get(4).get(),
              MapInfoEq(0x210000, 0x211000, 0, PROT_READ, "", maps->Get(3), nullptr));
}

TEST(LibunwindstackMaps, AddAndSortOverlappingEntireExistingMap) {
  std::unique_ptr<LibunwindstackMaps> libunwindstack_maps =
      LibunwindstackMaps::ParseMaps(kMapsInitialContent);
  ASSERT_NE(libunwindstack_maps, nullptr);

  libunwindstack_maps->AddAndSort(0x101000, 0x200000, 0x7000, PROT_READ | PROT_WRITE,
                                  "/path/to/newfile");
  libunwindstack_maps->AddAndSort(0x200000, 0x211000, 0, PROT_READ, "");
  unwindstack::Maps* maps = libunwindstack_maps->Get();
  ASSERT_NE(maps, nullptr);
  ASSERT_EQ(maps->Total(), 2);

  EXPECT_THAT(maps->Get(0).get(), MapInfoEq(0x101000, 0x200000, 0x7000, PROT_READ | PROT_WRITE,
                                            "/path/to/newfile", nullptr, maps->Get(1)));
  EXPECT_THAT(maps->Get(1).get(),
              MapInfoEq(0x200000, 0x211000, 0, PROT_READ, "", maps->Get(0), nullptr));
}

TEST(LibunwindstackMaps, AddAndSortOverlappingFirstPartOfExistingMap) {
  std::unique_ptr<LibunwindstackMaps> libunwindstack_maps =
      LibunwindstackMaps::ParseMaps(kMapsInitialContent);
  ASSERT_NE(libunwindstack_maps, nullptr);

  libunwindstack_maps->AddAndSort(0x100000, 0x102000, 0x7000, PROT_READ | PROT_WRITE,
                                  "/path/to/newfile");
  libunwindstack_maps->AddAndSort(0x1FF000, 0x201000, 0x0, PROT_READ, "");
  unwindstack::Maps* maps = libunwindstack_maps->Get();
  ASSERT_NE(maps, nullptr);
  ASSERT_EQ(maps->Total(), 5);

  EXPECT_THAT(maps->Get(0).get(), MapInfoEq(0x100000, 0x102000, 0x7000, PROT_READ | PROT_WRITE,
                                            "/path/to/newfile", nullptr, maps->Get(1)));
  EXPECT_THAT(maps->Get(1).get(), MapInfoEq(0x102000, 0x104000, 0x2000, PROT_READ, "/path/to/file",
                                            maps->Get(0), maps->Get(2)));
  EXPECT_THAT(maps->Get(2).get(), MapInfoEq(0x104000, 0x107000, 0, PROT_READ | PROT_EXEC, "",
                                            maps->Get(1), maps->Get(3)));
  EXPECT_THAT(maps->Get(3).get(),
              MapInfoEq(0x1FF000, 0x201000, 0, PROT_READ, "", maps->Get(2), maps->Get(4)));
  EXPECT_THAT(maps->Get(4).get(), MapInfoEq(0x201000, 0x210000, 0, PROT_READ | PROT_WRITE,
                                            "[stack]", maps->Get(3), nullptr));
}

TEST(LibunwindstackMaps, AddAndSortOverlappingLastPartOfExistingMap) {
  std::unique_ptr<LibunwindstackMaps> libunwindstack_maps =
      LibunwindstackMaps::ParseMaps(kMapsInitialContent);
  ASSERT_NE(libunwindstack_maps, nullptr);

  libunwindstack_maps->AddAndSort(0x103000, 0x104000, 0x7000, PROT_READ | PROT_WRITE,
                                  "/path/to/newfile");
  libunwindstack_maps->AddAndSort(0x201000, 0x211000, 0, PROT_READ, "");
  unwindstack::Maps* maps = libunwindstack_maps->Get();
  ASSERT_NE(maps, nullptr);
  ASSERT_EQ(maps->Total(), 5);

  EXPECT_THAT(maps->Get(0).get(), MapInfoEq(0x101000, 0x103000, 0x1000, PROT_READ, "/path/to/file",
                                            nullptr, maps->Get(1)));
  EXPECT_THAT(maps->Get(1).get(), MapInfoEq(0x103000, 0x104000, 0x7000, PROT_READ | PROT_WRITE,
                                            "/path/to/newfile", maps->Get(0), maps->Get(2)));
  EXPECT_THAT(maps->Get(2).get(), MapInfoEq(0x104000, 0x107000, 0, PROT_READ | PROT_EXEC, "",
                                            maps->Get(1), maps->Get(3)));
  EXPECT_THAT(maps->Get(3).get(), MapInfoEq(0x200000, 0x201000, 0, PROT_READ | PROT_WRITE,
                                            "[stack]", maps->Get(2), maps->Get(4)));
  EXPECT_THAT(maps->Get(4).get(),
              MapInfoEq(0x201000, 0x211000, 0, PROT_READ, "", maps->Get(3), nullptr));
}

TEST(LibunwindstackMaps, AddAndSortOverlappingMultipleExistingMaps) {
  std::unique_ptr<LibunwindstackMaps> libunwindstack_maps =
      LibunwindstackMaps::ParseMaps(kMapsInitialContent);
  ASSERT_NE(libunwindstack_maps, nullptr);

  libunwindstack_maps->AddAndSort(0x103000, 0x202000, 0x7000, PROT_READ | PROT_WRITE,
                                  "/path/to/newfile");
  unwindstack::Maps* maps = libunwindstack_maps->Get();
  ASSERT_NE(maps, nullptr);
  ASSERT_EQ(maps->Total(), 3);

  EXPECT_THAT(maps->Get(0).get(), MapInfoEq(0x101000, 0x103000, 0x1000, PROT_READ, "/path/to/file",
                                            nullptr, maps->Get(1)));
  EXPECT_THAT(maps->Get(1).get(), MapInfoEq(0x103000, 0x202000, 0x7000, PROT_READ | PROT_WRITE,
                                            "/path/to/newfile", maps->Get(0), maps->Get(2)));
  EXPECT_THAT(maps->Get(2).get(), MapInfoEq(0x202000, 0x210000, 0, PROT_READ | PROT_WRITE,
                                            "[stack]", maps->Get(1), nullptr));

  libunwindstack_maps->AddAndSort(0x106000, 0x212000, 0, PROT_READ, "");
  maps = libunwindstack_maps->Get();
  ASSERT_NE(maps, nullptr);
  ASSERT_EQ(maps->Total(), 3);

  EXPECT_THAT(maps->Get(0).get(), MapInfoEq(0x101000, 0x103000, 0x1000, PROT_READ, "/path/to/file",
                                            nullptr, maps->Get(1)));
  EXPECT_THAT(maps->Get(1).get(), MapInfoEq(0x103000, 0x106000, 0x7000, PROT_READ | PROT_WRITE,
                                            "/path/to/newfile", maps->Get(0), maps->Get(2)));
  EXPECT_THAT(maps->Get(2).get(),
              MapInfoEq(0x106000, 0x212000, 0, PROT_READ, "", maps->Get(1), nullptr));
}

TEST(LibunwindstackMaps, AddAndSortOverlappingMiddlePartOfExistingMap) {
  std::unique_ptr<LibunwindstackMaps> libunwindstack_maps =
      LibunwindstackMaps::ParseMaps(kMapsInitialContent);
  ASSERT_NE(libunwindstack_maps, nullptr);

  libunwindstack_maps->AddAndSort(0x102000, 0x103000, 0x7000, PROT_READ | PROT_WRITE,
                                  "/path/to/newfile");
  libunwindstack_maps->AddAndSort(0x201000, 0x202000, 0x0, PROT_READ, "");
  unwindstack::Maps* maps = libunwindstack_maps->Get();
  ASSERT_NE(maps, nullptr);
  ASSERT_EQ(maps->Total(), 7);

  EXPECT_THAT(maps->Get(0).get(), MapInfoEq(0x101000, 0x102000, 0x1000, PROT_READ, "/path/to/file",
                                            nullptr, maps->Get(1)));
  EXPECT_THAT(maps->Get(1).get(), MapInfoEq(0x102000, 0x103000, 0x7000, PROT_READ | PROT_WRITE,
                                            "/path/to/newfile", maps->Get(0), maps->Get(2)));
  EXPECT_THAT(maps->Get(2).get(), MapInfoEq(0x103000, 0x104000, 0x3000, PROT_READ, "/path/to/file",
                                            maps->Get(1), maps->Get(3)));
  EXPECT_THAT(maps->Get(3).get(), MapInfoEq(0x104000, 0x107000, 0, PROT_READ | PROT_EXEC, "",
                                            maps->Get(2), maps->Get(4)));
  EXPECT_THAT(maps->Get(4).get(), MapInfoEq(0x200000, 0x201000, 0, PROT_READ | PROT_WRITE,
                                            "[stack]", maps->Get(3), maps->Get(5)));
  EXPECT_THAT(maps->Get(5).get(),
              MapInfoEq(0x201000, 0x202000, 0, PROT_READ, "", maps->Get(4), maps->Get(6)));
  EXPECT_THAT(maps->Get(6).get(), MapInfoEq(0x202000, 0x210000, 0, PROT_READ | PROT_WRITE,
                                            "[stack]", maps->Get(5), nullptr));
}

TEST(LibunwindstackMaps, AddAndSortIntoEmpty) {
  std::unique_ptr<LibunwindstackMaps> libunwindstack_maps = LibunwindstackMaps::ParseMaps("");
  ASSERT_NE(libunwindstack_maps, nullptr);

  libunwindstack_maps->AddAndSort(0x107000, 0x200000, 0x7000, PROT_READ | PROT_WRITE,
                                  "/path/to/newfile");
  unwindstack::Maps* maps = libunwindstack_maps->Get();
  ASSERT_NE(maps, nullptr);
  ASSERT_EQ(maps->Total(), 1);

  EXPECT_THAT(maps->Get(0).get(), MapInfoEq(0x107000, 0x200000, 0x7000, PROT_READ | PROT_WRITE,
                                            "/path/to/newfile", nullptr, nullptr));
}

}  // namespace orbit_linux_tracing
