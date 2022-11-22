// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "LibunwindstackMaps.h"
#include "LibunwindstackUnwinder.h"
#include "Test/Path.h"

namespace orbit_linux_tracing {

static std::unique_ptr<LibunwindstackMaps> CreateFakeMapsEntry(std::string_view target) {
  std::string maps_file_entry = absl::StrFormat(
      "000000000000-000000001000 r--p 00000000 fe:00 123 %1$s\n"
      "000000001000-000000003000 r-xp 00001000 fe:00 123 %1$s\n"
      "000000003000-000000004000 r--p 00003000 fe:00 123 %1$s\n"
      "000000004000-000000005000 r--p 00003000 fe:00 123 %1$s\n"
      "000000005000-000000006000 rw-p 00004000 fe:00 123 %1$s",
      (orbit_test::GetTestdataDir() / target).string());
  return LibunwindstackMaps::ParseMaps(maps_file_entry);
}

constexpr pid_t kProcessId = 123;

// This is some disassembly of target.cc compiled with -fno-omit-frame-pointer and
// -momit-leaf-frame-pointer (target_fp):
//
// 0000000000001215 <_Z9every_1usv>:
//    1215:       48 c7 44 24 f8 00 00 00 00   movq   $0x0,-0x8(%rsp)
//    121e:       c7 44 24 f4 00 00 00 00      movl   $0x0,-0xc(%rsp)
//    1226:       81 7c 24 f4 55 01 00 00      cmpl   $0x155,-0xc(%rsp)
//    122e:       7f 12                        jg     1242 <_Z9every_1usv+0x2d>
//    1230:       8b 44 24 f4                  mov    -0xc(%rsp),%eax
//    1234:       48 98                        cltq
//    1236:       48 01 44 24 f8               add    %rax,-0x8(%rsp)
//    123b:       83 44 24 f4 01               addl   $0x1,-0xc(%rsp)
//    1240:       eb e4                        jmp    1226 <_Z9every_1usv+0x11>
//    1242:       48 8b 44 24 f8               mov    -0x8(%rsp),%rax
//    1247:       c3                           ret
//
// 0000000000001248 <_Z10every_10usv>:
//    1248:       55                       push   %rbp
//    1249:       48 89 e5                 mov    %rsp,%rbp
//    124c:       48 83 ec 10              sub    $0x10,%rsp
//    1250:       48 c7 45 f8 00 00 00 00  movq   $0x0,-0x8(%rbp)
//    1258:       c7 45 f4 00 00 00 00     movl   $0x0,-0xc(%rbp)
//    125f:       83 7d f4 09              cmpl   $0x9,-0xc(%rbp)
//    1263:       7f 0f                    jg     1274 <_Z10every_10usv+0x2c>
//    1265:       e8 ab ff ff ff           call   1215 <_Z9every_1usv>
//    126a:       48 01 45 f8              add    %rax,-0x8(%rbp)
//    126e:       83 45 f4 01              addl   $0x1,-0xc(%rbp)
//    1272:       eb eb                    jmp    125f <_Z10every_10usv+0x17>
//    1274:       48 8b 45 f8              mov    -0x8(%rbp),%rax
//    1278:       c9                       leave
//    1279:       c3                       ret

// This is some disassembly of target.cc compiled with -fomit-leaf-frame-pointer (target_no_fp):
//
// 000000000000128c <_Z10every_10usv>:
//     128c:       48 83 ec 10                 sub    $0x10,%rsp
//     1290:       48 c7 44 24 08 00 00 00 00  movq   $0x0,0x8(%rsp)
//     1299:       c7 44 24 04 00 00 00 00     movl   $0x0,0x4(%rsp)
//     12a1:       eb 0f                       jmp    12b2 <_Z10every_10usv+0x26>
//     12a3:       e8 b1 ff ff ff              call   1259 <_Z9every_1usv>
//     12a8:       48 01 44 24 08              add    %rax,0x8(%rsp)
//     12ad:       83 44 24 04 01              addl   $0x1,0x4(%rsp)
//     12b2:       83 7c 24 04 09              cmpl   $0x9,0x4(%rsp)
//     12b7:       7e ea                       jle    12a3 <_Z10every_10usv+0x17>
//     12b9:       48 8b 44 24 08              mov    0x8(%rsp),%rax
//     12be:       48 83 c4 10 c3              add    $0x10,%rsp

TEST(LibunwindstackUnwinder, DetectsFunctionWithFramePointerSet) {
  auto unwinder = LibunwindstackUnwinder::Create();

  auto maps = CreateFakeMapsEntry("target_fp");

  std::optional<bool> has_frame_pointer_set_or_error =
      unwinder->HasFramePointerSet(0x124c, kProcessId, maps->Get());

  ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
  EXPECT_TRUE(has_frame_pointer_set_or_error.value());
}

TEST(LibunwindstackUnwinder, DetectsLeafFunction) {
  auto unwinder = LibunwindstackUnwinder::Create();

  auto maps = CreateFakeMapsEntry("target_fp");

  std::optional<bool> has_frame_pointer_set_or_error =
      unwinder->HasFramePointerSet(0x122e, kProcessId, maps->Get());

  ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
  EXPECT_FALSE(has_frame_pointer_set_or_error.value());
}

TEST(LibunwindstackUnwinder, DetectsFunctionWithoutFramePointer) {
  auto unwinder = LibunwindstackUnwinder::Create();

  auto maps = CreateFakeMapsEntry("target_no_fp");

  std::optional<bool> has_frame_pointer_set_or_error =
      unwinder->HasFramePointerSet(0x12ad, kProcessId, maps->Get());

  ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
  EXPECT_FALSE(has_frame_pointer_set_or_error.value());
}

TEST(LibunwindstackUnwinder, DetectsFramePointerNotSetAtPushRbp) {
  auto unwinder = LibunwindstackUnwinder::Create();

  auto maps = CreateFakeMapsEntry("target_fp");

  std::optional<bool> has_frame_pointer_set_or_error =
      unwinder->HasFramePointerSet(0x1248, kProcessId, maps->Get());

  ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
  EXPECT_FALSE(has_frame_pointer_set_or_error.value());
}

TEST(LibunwindstackUnwinder, DetectsFramePointerNotSetAtMovRspToRbp) {
  auto unwinder = LibunwindstackUnwinder::Create();

  auto maps = CreateFakeMapsEntry("target_fp");

  std::optional<bool> has_frame_pointer_set_or_error =
      unwinder->HasFramePointerSet(0x1249, kProcessId, maps->Get());

  ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
  EXPECT_FALSE(has_frame_pointer_set_or_error.value());
}

TEST(LibunwindstackUnwinder, DetectsFramePointerSetAtLeave) {
  auto unwinder = LibunwindstackUnwinder::Create();

  auto maps = CreateFakeMapsEntry("target_fp");

  std::optional<bool> has_frame_pointer_set_or_error =
      unwinder->HasFramePointerSet(0x1278, kProcessId, maps->Get());

  ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
  EXPECT_TRUE(has_frame_pointer_set_or_error.value());
}

TEST(LibunwindstackUnwinder, DetectsFramePointerNotSetAtRet) {
  auto unwinder = LibunwindstackUnwinder::Create();

  auto maps = CreateFakeMapsEntry("target_fp");

  std::optional<bool> has_frame_pointer_set_or_error =
      unwinder->HasFramePointerSet(0x1279, kProcessId, maps->Get());

  ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
  EXPECT_FALSE(has_frame_pointer_set_or_error.value());
}

TEST(LibunwindstackUnwinder, FramePointerDetectionWorksWithCaching) {
  auto unwinder = LibunwindstackUnwinder::Create();

  auto maps = CreateFakeMapsEntry("target_fp");

  // Let's go through the following instructions a couple of times and verify the results remain
  // correct.
  //    1248:       55                      push   %rbp
  //    1249:       48 89 e5                mov    %rsp,%rbp
  //    124c:       48 83 ec 10             sub    $0x10,%rsp
  //    1250:       48 c7 45 f8 00 00 00    movq   $0x0,-0x8(%rbp)

  //    1278:       c9                      leave
  //    1279:       c3                      ret
  constexpr size_t kMaxRepetitions = 5;
  std::optional<bool> has_frame_pointer_set_or_error;

  for (size_t i = 0; i < kMaxRepetitions; i++) {
    //    1248:       55                      push   %rbp
    has_frame_pointer_set_or_error = unwinder->HasFramePointerSet(0x1248, kProcessId, maps->Get());
    ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
    EXPECT_FALSE(has_frame_pointer_set_or_error.value());

    //    1249:       48 89 e5                mov    %rsp,%rbp
    has_frame_pointer_set_or_error = unwinder->HasFramePointerSet(0x1249, kProcessId, maps->Get());
    ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
    EXPECT_FALSE(has_frame_pointer_set_or_error.value());

    //    124c:       48 83 ec 10             sub    $0x10,%rsp
    has_frame_pointer_set_or_error = unwinder->HasFramePointerSet(0x124c, kProcessId, maps->Get());
    ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
    EXPECT_TRUE(has_frame_pointer_set_or_error.value());

    //    1250:       48 c7 45 f8 00 00 00    movq   $0x0,-0x8(%rbp)
    has_frame_pointer_set_or_error = unwinder->HasFramePointerSet(0x1250, kProcessId, maps->Get());
    ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
    EXPECT_TRUE(has_frame_pointer_set_or_error.value());

    //    1278:       c9                      leave
    has_frame_pointer_set_or_error = unwinder->HasFramePointerSet(0x1278, kProcessId, maps->Get());
    ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
    EXPECT_TRUE(has_frame_pointer_set_or_error.value());

    //    1279:       c3                      ret
    has_frame_pointer_set_or_error = unwinder->HasFramePointerSet(0x1279, kProcessId, maps->Get());
    ASSERT_TRUE(has_frame_pointer_set_or_error.has_value());
    EXPECT_FALSE(has_frame_pointer_set_or_error.value());
  }
}

}  // namespace orbit_linux_tracing