// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/mman.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "ModuleUtils/ReadLinuxMaps.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TestUtils.h"

using orbit_test_utils::HasNoError;

namespace orbit_module_utils {

TEST(ReadLinuxMaps, ReadMaps) {
  const ErrorMessageOr<std::string> proc_pid_maps_content = ReadMaps(getpid());
  EXPECT_THAT(proc_pid_maps_content, HasNoError());
  EXPECT_GT(proc_pid_maps_content.value().size(), 0);
}

TEST(ReadLinuxMaps, ParseMaps) {
  constexpr const char* kProcPidMapsContent{
      "00400000-00452000 r-xp 00000000 08:02 173521      /usr/bin/dbus-daemon\n"
      "00e03000-00e24000 rw-p 00000000 00:00 0           [heap]\n"
      "35b1800000-35b1820000 r-xp 00000000 08:02 135522  /path with spaces\n"
      "35b1a21000-35b1a22000 rw-p 00000000 00:00 0       \n"};
  std::vector<LinuxMemoryMapping> maps = ParseMaps(kProcPidMapsContent);
  ASSERT_EQ(maps.size(), 4);

  EXPECT_EQ(maps[0].start_address(), 0x400000);
  EXPECT_EQ(maps[0].end_address(), 0x452000);
  EXPECT_EQ(maps[0].perms(), PROT_READ | PROT_EXEC);
  EXPECT_EQ(maps[0].inode(), 173521);
  EXPECT_EQ(maps[0].pathname(), "/usr/bin/dbus-daemon");

  EXPECT_EQ(maps[1].start_address(), 0xe03000);
  EXPECT_EQ(maps[1].end_address(), 0xe24000);
  EXPECT_EQ(maps[1].perms(), PROT_READ | PROT_WRITE);
  EXPECT_EQ(maps[1].inode(), 0);
  EXPECT_EQ(maps[1].pathname(), "[heap]");

  EXPECT_EQ(maps[2].start_address(), 0x35b1800000);
  EXPECT_EQ(maps[2].end_address(), 0x35b1820000);
  EXPECT_EQ(maps[2].perms(), PROT_READ | PROT_EXEC);
  EXPECT_EQ(maps[2].inode(), 135522);
  EXPECT_EQ(maps[2].pathname(), "/path with spaces");

  EXPECT_EQ(maps[3].start_address(), 0x35b1a21000);
  EXPECT_EQ(maps[3].end_address(), 0x35b1a22000);
  EXPECT_EQ(maps[3].perms(), PROT_READ | PROT_WRITE);
  EXPECT_EQ(maps[3].inode(), 0);
  EXPECT_EQ(maps[3].pathname(), "");
}

TEST(ReadLinuxMaps, ParseMapsFromInvalidProcPidMapsContent) {
  std::vector<LinuxMemoryMapping> maps;

  maps = ParseMaps("");
  EXPECT_EQ(maps.size(), 0);

  maps = ParseMaps("\n\n");
  EXPECT_EQ(maps.size(), 0);

  // Missing inode.
  maps = ParseMaps("00400000-00452000 r-xp 00000000 08:02");
  EXPECT_EQ(maps.size(), 0);

  // Unexpected protection format.
  maps = ParseMaps("00400000-00452000 r-x 00000000 08:02 173521      /usr/bin/dbus-daemon");
  EXPECT_EQ(maps.size(), 0);

  // Non-numeric inode.
  maps = ParseMaps("00400000-00452000 r-xp 00000000 08:02 173521a      /usr/bin/dbus-daemon\n");
  EXPECT_EQ(maps.size(), 0);
}

TEST(ReadLinuxMaps, ReadAndParseMaps) {
  const ErrorMessageOr<std::vector<LinuxMemoryMapping>> maps = ReadAndParseMaps(getpid());
  EXPECT_THAT(maps, HasNoError());
  EXPECT_GT(maps.value().size(), 0);
}

}  // namespace orbit_module_utils
