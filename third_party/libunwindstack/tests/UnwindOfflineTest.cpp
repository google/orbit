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

#include <sys/mman.h>

#include <gtest/gtest.h>

#include <cstddef>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include <unwindstack/Arch.h>
#include <unwindstack/Memory.h>
#include <unwindstack/RegsArm64.h>
#include <unwindstack/Unwinder.h>

#include "TestUtils.h"
#include "utils/MemoryFake.h"
#include "utils/OfflineUnwindUtils.h"

// This collection of tests exercises Unwinder::Unwind for offline unwinds.
//
// See `libunwindstack/utils/OfflineUnwindUtils.h` for more info on offline unwinds
// and b/192012600 for additional information regarding offline unwind benchmarks.
namespace unwindstack {
namespace {

class UnwindOfflineTest : public ::testing::Test {
 public:
  bool GetExpectedSamplesFrameInfo(
      std::string* expected_frame_info, std::string* error_msg,
      const std::string& sample_name = OfflineUnwindUtils::kSingleSample) {
    const std::string* a_frame_info_path = offline_utils_.GetFrameInfoFilepath(sample_name);
    if (a_frame_info_path == nullptr) {
      std::stringstream err_stream;
      err_stream << "Unable to get frame info filepath for invalid sample name " << sample_name
                 << ".\n";
      *error_msg = err_stream.str();
      return false;
    }

    std::ifstream in(*a_frame_info_path);
    std::stringstream buffer;
    buffer << in.rdbuf();
    *expected_frame_info = buffer.str();
    return true;
  }

  void ConsecutiveUnwindTest(const std::vector<UnwindSampleInfo>& sample_infos) {
    std::string error_msg;
    if (!offline_utils_.Init(sample_infos, &error_msg)) FAIL() << error_msg;

    for (const auto& sample_info : sample_infos) {
      const std::string& sample_name = sample_info.offline_files_dir;
      // Need to change to sample directory for Unwinder to properly init ELF objects.
      // See more info at OfflineUnwindUtils::ChangeToSampleDirectory.
      if (!offline_utils_.ChangeToSampleDirectory(&error_msg, sample_name)) FAIL() << error_msg;

      Unwinder unwinder =
          Unwinder(128, offline_utils_.GetMaps(sample_name), offline_utils_.GetRegs(sample_name),
                   offline_utils_.GetProcessMemory(sample_name));
      if (sample_info.memory_flag == ProcessMemoryFlag::kIncludeJitMemory) {
        unwinder.SetJitDebug(offline_utils_.GetJitDebug(sample_name));
      }
      unwinder.Unwind();

      size_t expected_num_frames;
      if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg, sample_name))
        FAIL() << error_msg;
      std::string expected_frame_info;
      if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg, sample_name))
        FAIL() << error_msg;

      std::string actual_frame_info = DumpFrames(unwinder);
      ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << actual_frame_info;
      EXPECT_EQ(expected_frame_info, actual_frame_info);
    }
  }

 protected:
  void TearDown() override { offline_utils_.ReturnToCurrentWorkingDirectory(); }

  OfflineUnwindUtils offline_utils_;
};

TEST_F(UnwindOfflineTest, pc_straddle_arm) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "straddle_arm/", .arch = ARCH_ARM}, &error_msg))
    FAIL() << error_msg;

  Regs* regs = offline_utils_.GetRegs();
  std::unique_ptr<Regs> regs_copy(regs->Clone());
  Unwinder unwinder(128, offline_utils_.GetMaps(), regs, offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0xf31ea9f8U, unwinder.frames()[0].pc);
  EXPECT_EQ(0xe9c866f8U, unwinder.frames()[0].sp);
  EXPECT_EQ(0xf2da0a1bU, unwinder.frames()[1].pc);
  EXPECT_EQ(0xe9c86728U, unwinder.frames()[1].sp);
  EXPECT_EQ(0xf2da1441U, unwinder.frames()[2].pc);
  EXPECT_EQ(0xe9c86730U, unwinder.frames()[2].sp);
  EXPECT_EQ(0xf3367147U, unwinder.frames()[3].pc);
  EXPECT_EQ(0xe9c86778U, unwinder.frames()[3].sp);

  // Display build ids now.
  unwinder.SetRegs(regs_copy.get());
  unwinder.SetDisplayBuildID(true);
  unwinder.Unwind();

  frame_info = DumpFrames(unwinder);
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(
      "  #00 pc 0001a9f8  libc.so (abort+64) (BuildId: 2dd0d4ba881322a0edabeed94808048c)\n"
      "  #01 pc 00006a1b  libbase.so (android::base::DefaultAborter(char const*)+6) (BuildId: "
      "ed43842c239cac1a618e600ea91c4cbd)\n"
      "  #02 pc 00007441  libbase.so (android::base::LogMessage::~LogMessage()+748) (BuildId: "
      "ed43842c239cac1a618e600ea91c4cbd)\n"
      "  #03 pc 00015147  /does/not/exist/libhidlbase.so\n",
      frame_info);
}

TEST_F(UnwindOfflineTest, pc_in_gnu_debugdata_arm) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "gnu_debugdata_arm/", .arch = ARCH_ARM},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0xf1f6dc49U, unwinder.frames()[0].pc);
  EXPECT_EQ(0xd8fe6930U, unwinder.frames()[0].sp);
  EXPECT_EQ(0xf1f6dce5U, unwinder.frames()[1].pc);
  EXPECT_EQ(0xd8fe6958U, unwinder.frames()[1].sp);
}

TEST_F(UnwindOfflineTest, pc_straddle_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "straddle_arm64/", .arch = ARCH_ARM64},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x64d09d4fd8U, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7fe0d84040U, unwinder.frames()[0].sp);
  EXPECT_EQ(0x64d09d5078U, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7fe0d84070U, unwinder.frames()[1].sp);
  EXPECT_EQ(0x64d09d508cU, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7fe0d84080U, unwinder.frames()[2].sp);
  EXPECT_EQ(0x64d09d88fcU, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7fe0d84090U, unwinder.frames()[3].sp);
  EXPECT_EQ(0x64d09d88d8U, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7fe0d840f0U, unwinder.frames()[4].sp);
  EXPECT_EQ(0x64d0a00d70U, unwinder.frames()[5].pc);
  EXPECT_EQ(0x7fe0d84110U, unwinder.frames()[5].sp);
}

TEST_F(UnwindOfflineTest, jit_debug_x86) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "jit_debug_x86/",
                            .arch = ARCH_X86,
                            .memory_flag = ProcessMemoryFlag::kIncludeJitMemory},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.SetJitDebug(offline_utils_.GetJitDebug());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0xeb89bfb8U, unwinder.frames()[0].pc);
  EXPECT_EQ(0xffeb5280U, unwinder.frames()[0].sp);
  EXPECT_EQ(0xeb89af00U, unwinder.frames()[1].pc);
  EXPECT_EQ(0xffeb52a0U, unwinder.frames()[1].sp);
  EXPECT_EQ(0xec6061a8U, unwinder.frames()[2].pc);
  EXPECT_EQ(0xffeb5ce0U, unwinder.frames()[2].sp);
  EXPECT_EQ(0xee75be80U, unwinder.frames()[3].pc);
  EXPECT_EQ(0xffeb5d30U, unwinder.frames()[3].sp);
  EXPECT_EQ(0xf728e4d2U, unwinder.frames()[4].pc);
  EXPECT_EQ(0xffeb5d60U, unwinder.frames()[4].sp);
  EXPECT_EQ(0xf6d27ab5U, unwinder.frames()[5].pc);
  EXPECT_EQ(0xffeb5d80U, unwinder.frames()[5].sp);
  EXPECT_EQ(0xf6f7df0dU, unwinder.frames()[6].pc);
  EXPECT_EQ(0xffeb5e20U, unwinder.frames()[6].sp);
  EXPECT_EQ(0xf6f73552U, unwinder.frames()[7].pc);
  EXPECT_EQ(0xffeb5ec0U, unwinder.frames()[7].sp);
  EXPECT_EQ(0xf6f7499aU, unwinder.frames()[8].pc);
  EXPECT_EQ(0xffeb5f40U, unwinder.frames()[8].sp);
  EXPECT_EQ(0xf7265362U, unwinder.frames()[9].pc);
  EXPECT_EQ(0xffeb5fb0U, unwinder.frames()[9].sp);
  EXPECT_EQ(0xf72945bdU, unwinder.frames()[10].pc);
  EXPECT_EQ(0xffeb6110U, unwinder.frames()[10].sp);
  EXPECT_EQ(0xee75be03U, unwinder.frames()[11].pc);
  EXPECT_EQ(0xffeb6160U, unwinder.frames()[11].sp);
  EXPECT_EQ(0xf728e4d2U, unwinder.frames()[12].pc);
  EXPECT_EQ(0xffeb6180U, unwinder.frames()[12].sp);
  EXPECT_EQ(0xf6d27ab5U, unwinder.frames()[13].pc);
  EXPECT_EQ(0xffeb61b0U, unwinder.frames()[13].sp);
  EXPECT_EQ(0xf6f7df0dU, unwinder.frames()[14].pc);
  EXPECT_EQ(0xffeb6250U, unwinder.frames()[14].sp);
  EXPECT_EQ(0xf6f73552U, unwinder.frames()[15].pc);
  EXPECT_EQ(0xffeb62f0U, unwinder.frames()[15].sp);
  EXPECT_EQ(0xf6f7499aU, unwinder.frames()[16].pc);
  EXPECT_EQ(0xffeb6370U, unwinder.frames()[16].sp);
  EXPECT_EQ(0xf7265362U, unwinder.frames()[17].pc);
  EXPECT_EQ(0xffeb63e0U, unwinder.frames()[17].sp);
  EXPECT_EQ(0xf72945bdU, unwinder.frames()[18].pc);
  EXPECT_EQ(0xffeb6530U, unwinder.frames()[18].sp);
  EXPECT_EQ(0xee75bd3bU, unwinder.frames()[19].pc);
  EXPECT_EQ(0xffeb6580U, unwinder.frames()[19].sp);
  EXPECT_EQ(0xf728e4d2U, unwinder.frames()[20].pc);
  EXPECT_EQ(0xffeb65b0U, unwinder.frames()[20].sp);
  EXPECT_EQ(0xf6d27ab5U, unwinder.frames()[21].pc);
  EXPECT_EQ(0xffeb65e0U, unwinder.frames()[21].sp);
  EXPECT_EQ(0xf6f7df0dU, unwinder.frames()[22].pc);
  EXPECT_EQ(0xffeb6680U, unwinder.frames()[22].sp);
  EXPECT_EQ(0xf6f73552U, unwinder.frames()[23].pc);
  EXPECT_EQ(0xffeb6720U, unwinder.frames()[23].sp);
  EXPECT_EQ(0xf6f7499aU, unwinder.frames()[24].pc);
  EXPECT_EQ(0xffeb67a0U, unwinder.frames()[24].sp);
  EXPECT_EQ(0xf7265362U, unwinder.frames()[25].pc);
  EXPECT_EQ(0xffeb6810U, unwinder.frames()[25].sp);
  EXPECT_EQ(0xf72945bdU, unwinder.frames()[26].pc);
  EXPECT_EQ(0xffeb6960U, unwinder.frames()[26].sp);
  EXPECT_EQ(0xee75bbdbU, unwinder.frames()[27].pc);
  EXPECT_EQ(0xffeb69b0U, unwinder.frames()[27].sp);
  EXPECT_EQ(0xf728e6a2U, unwinder.frames()[28].pc);
  EXPECT_EQ(0xffeb69f0U, unwinder.frames()[28].sp);
  EXPECT_EQ(0xf6d27acbU, unwinder.frames()[29].pc);
  EXPECT_EQ(0xffeb6a20U, unwinder.frames()[29].sp);
  EXPECT_EQ(0xf6f7df0dU, unwinder.frames()[30].pc);
  EXPECT_EQ(0xffeb6ac0U, unwinder.frames()[30].sp);
  EXPECT_EQ(0xf6f73552U, unwinder.frames()[31].pc);
  EXPECT_EQ(0xffeb6b60U, unwinder.frames()[31].sp);
  EXPECT_EQ(0xf6f7499aU, unwinder.frames()[32].pc);
  EXPECT_EQ(0xffeb6be0U, unwinder.frames()[32].sp);
  EXPECT_EQ(0xf7265362U, unwinder.frames()[33].pc);
  EXPECT_EQ(0xffeb6c50U, unwinder.frames()[33].sp);
  EXPECT_EQ(0xf72945bdU, unwinder.frames()[34].pc);
  EXPECT_EQ(0xffeb6dd0U, unwinder.frames()[34].sp);
  EXPECT_EQ(0xee75b624U, unwinder.frames()[35].pc);
  EXPECT_EQ(0xffeb6e20U, unwinder.frames()[35].sp);
  EXPECT_EQ(0xf728e4d2U, unwinder.frames()[36].pc);
  EXPECT_EQ(0xffeb6e50U, unwinder.frames()[36].sp);
  EXPECT_EQ(0xf6d27ab5U, unwinder.frames()[37].pc);
  EXPECT_EQ(0xffeb6e70U, unwinder.frames()[37].sp);
  EXPECT_EQ(0xf6f7df0dU, unwinder.frames()[38].pc);
  EXPECT_EQ(0xffeb6f10U, unwinder.frames()[38].sp);
  EXPECT_EQ(0xf6f73552U, unwinder.frames()[39].pc);
  EXPECT_EQ(0xffeb6fb0U, unwinder.frames()[39].sp);
  EXPECT_EQ(0xf6f7499aU, unwinder.frames()[40].pc);
  EXPECT_EQ(0xffeb7030U, unwinder.frames()[40].sp);
  EXPECT_EQ(0xf7265362U, unwinder.frames()[41].pc);
  EXPECT_EQ(0xffeb70a0U, unwinder.frames()[41].sp);
  EXPECT_EQ(0xf72945bdU, unwinder.frames()[42].pc);
  EXPECT_EQ(0xffeb71f0U, unwinder.frames()[42].sp);
  EXPECT_EQ(0xee75aedbU, unwinder.frames()[43].pc);
  EXPECT_EQ(0xffeb7240U, unwinder.frames()[43].sp);
  EXPECT_EQ(0xf728e4d2U, unwinder.frames()[44].pc);
  EXPECT_EQ(0xffeb72a0U, unwinder.frames()[44].sp);
  EXPECT_EQ(0xf6d27ab5U, unwinder.frames()[45].pc);
  EXPECT_EQ(0xffeb72c0U, unwinder.frames()[45].sp);
  EXPECT_EQ(0xf6f7df0dU, unwinder.frames()[46].pc);
  EXPECT_EQ(0xffeb7360U, unwinder.frames()[46].sp);
  EXPECT_EQ(0xf6f73552U, unwinder.frames()[47].pc);
  EXPECT_EQ(0xffeb7400U, unwinder.frames()[47].sp);
  EXPECT_EQ(0xf6f7499aU, unwinder.frames()[48].pc);
  EXPECT_EQ(0xffeb7480U, unwinder.frames()[48].sp);
  EXPECT_EQ(0xf7265362U, unwinder.frames()[49].pc);
  EXPECT_EQ(0xffeb74f0U, unwinder.frames()[49].sp);
  EXPECT_EQ(0xf72945bdU, unwinder.frames()[50].pc);
  EXPECT_EQ(0xffeb7680U, unwinder.frames()[50].sp);
  EXPECT_EQ(0xee756c21U, unwinder.frames()[51].pc);
  EXPECT_EQ(0xffeb76d0U, unwinder.frames()[51].sp);
  EXPECT_EQ(0xf728e6a2U, unwinder.frames()[52].pc);
  EXPECT_EQ(0xffeb76f0U, unwinder.frames()[52].sp);
  EXPECT_EQ(0xf6d27acbU, unwinder.frames()[53].pc);
  EXPECT_EQ(0xffeb7710U, unwinder.frames()[53].sp);
  EXPECT_EQ(0xf6f7df0dU, unwinder.frames()[54].pc);
  EXPECT_EQ(0xffeb77b0U, unwinder.frames()[54].sp);
  EXPECT_EQ(0xf6f73552U, unwinder.frames()[55].pc);
  EXPECT_EQ(0xffeb7850U, unwinder.frames()[55].sp);
  EXPECT_EQ(0xf6f7499aU, unwinder.frames()[56].pc);
  EXPECT_EQ(0xffeb78d0U, unwinder.frames()[56].sp);
  EXPECT_EQ(0xf7265362U, unwinder.frames()[57].pc);
  EXPECT_EQ(0xffeb7940U, unwinder.frames()[57].sp);
  EXPECT_EQ(0xf72945bdU, unwinder.frames()[58].pc);
  EXPECT_EQ(0xffeb7a80U, unwinder.frames()[58].sp);
  EXPECT_EQ(0xf728e6a2U, unwinder.frames()[59].pc);
  EXPECT_EQ(0xffeb7ad0U, unwinder.frames()[59].sp);
  EXPECT_EQ(0xf6d27acbU, unwinder.frames()[60].pc);
  EXPECT_EQ(0xffeb7af0U, unwinder.frames()[60].sp);
  EXPECT_EQ(0xf718bc95U, unwinder.frames()[61].pc);
  EXPECT_EQ(0xffeb7b90U, unwinder.frames()[61].sp);
  EXPECT_EQ(0xf718bb5aU, unwinder.frames()[62].pc);
  EXPECT_EQ(0xffeb7c50U, unwinder.frames()[62].sp);
  EXPECT_EQ(0xf706b3ddU, unwinder.frames()[63].pc);
  EXPECT_EQ(0xffeb7d10U, unwinder.frames()[63].sp);
  EXPECT_EQ(0xf6d6548cU, unwinder.frames()[64].pc);
  EXPECT_EQ(0xffeb7d70U, unwinder.frames()[64].sp);
  EXPECT_EQ(0xf6d5df06U, unwinder.frames()[65].pc);
  EXPECT_EQ(0xffeb7df0U, unwinder.frames()[65].sp);
  EXPECT_EQ(0x56574d8cU, unwinder.frames()[66].pc);
  EXPECT_EQ(0xffeb7e40U, unwinder.frames()[66].sp);
  EXPECT_EQ(0x56574a80U, unwinder.frames()[67].pc);
  EXPECT_EQ(0xffeb7e70U, unwinder.frames()[67].sp);
  EXPECT_EQ(0xf7363275U, unwinder.frames()[68].pc);
  EXPECT_EQ(0xffeb7ef0U, unwinder.frames()[68].sp);
}

TEST_F(UnwindOfflineTest, jit_debug_arm) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "jit_debug_arm/",
                            .arch = ARCH_ARM,
                            .memory_flag = ProcessMemoryFlag::kIncludeJitMemory},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.SetJitDebug(offline_utils_.GetJitDebug());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0xdfe66a5eU, unwinder.frames()[0].pc);
  EXPECT_EQ(0xff85d180U, unwinder.frames()[0].sp);
  EXPECT_EQ(0xe044712dU, unwinder.frames()[1].pc);
  EXPECT_EQ(0xff85d200U, unwinder.frames()[1].sp);
  EXPECT_EQ(0xe27a7cb1U, unwinder.frames()[2].pc);
  EXPECT_EQ(0xff85d290U, unwinder.frames()[2].sp);
  EXPECT_EQ(0xed75c175U, unwinder.frames()[3].pc);
  EXPECT_EQ(0xff85d2b0U, unwinder.frames()[3].sp);
  EXPECT_EQ(0xed761129U, unwinder.frames()[4].pc);
  EXPECT_EQ(0xff85d2e8U, unwinder.frames()[4].sp);
  EXPECT_EQ(0xed3b97a9U, unwinder.frames()[5].pc);
  EXPECT_EQ(0xff85d370U, unwinder.frames()[5].sp);
  EXPECT_EQ(0xed541833U, unwinder.frames()[6].pc);
  EXPECT_EQ(0xff85d3d8U, unwinder.frames()[6].sp);
  EXPECT_EQ(0xed528935U, unwinder.frames()[7].pc);
  EXPECT_EQ(0xff85d428U, unwinder.frames()[7].sp);
  EXPECT_EQ(0xed52971dU, unwinder.frames()[8].pc);
  EXPECT_EQ(0xff85d470U, unwinder.frames()[8].sp);
  EXPECT_EQ(0xed73c865U, unwinder.frames()[9].pc);
  EXPECT_EQ(0xff85d4b0U, unwinder.frames()[9].sp);
  EXPECT_EQ(0xed7606ffU, unwinder.frames()[10].pc);
  EXPECT_EQ(0xff85d5d0U, unwinder.frames()[10].sp);
  EXPECT_EQ(0xe27a7c31U, unwinder.frames()[11].pc);
  EXPECT_EQ(0xff85d640U, unwinder.frames()[11].sp);
  EXPECT_EQ(0xed75c175U, unwinder.frames()[12].pc);
  EXPECT_EQ(0xff85d660U, unwinder.frames()[12].sp);
  EXPECT_EQ(0xed761129U, unwinder.frames()[13].pc);
  EXPECT_EQ(0xff85d698U, unwinder.frames()[13].sp);
  EXPECT_EQ(0xed3b97a9U, unwinder.frames()[14].pc);
  EXPECT_EQ(0xff85d720U, unwinder.frames()[14].sp);
  EXPECT_EQ(0xed541833U, unwinder.frames()[15].pc);
  EXPECT_EQ(0xff85d788U, unwinder.frames()[15].sp);
  EXPECT_EQ(0xed528935U, unwinder.frames()[16].pc);
  EXPECT_EQ(0xff85d7d8U, unwinder.frames()[16].sp);
  EXPECT_EQ(0xed52971dU, unwinder.frames()[17].pc);
  EXPECT_EQ(0xff85d820U, unwinder.frames()[17].sp);
  EXPECT_EQ(0xed73c865U, unwinder.frames()[18].pc);
  EXPECT_EQ(0xff85d860U, unwinder.frames()[18].sp);
  EXPECT_EQ(0xed7606ffU, unwinder.frames()[19].pc);
  EXPECT_EQ(0xff85d970U, unwinder.frames()[19].sp);
  EXPECT_EQ(0xe27a7b77U, unwinder.frames()[20].pc);
  EXPECT_EQ(0xff85d9e0U, unwinder.frames()[20].sp);
  EXPECT_EQ(0xed75c175U, unwinder.frames()[21].pc);
  EXPECT_EQ(0xff85da10U, unwinder.frames()[21].sp);
  EXPECT_EQ(0xed761129U, unwinder.frames()[22].pc);
  EXPECT_EQ(0xff85da48U, unwinder.frames()[22].sp);
  EXPECT_EQ(0xed3b97a9U, unwinder.frames()[23].pc);
  EXPECT_EQ(0xff85dad0U, unwinder.frames()[23].sp);
  EXPECT_EQ(0xed541833U, unwinder.frames()[24].pc);
  EXPECT_EQ(0xff85db38U, unwinder.frames()[24].sp);
  EXPECT_EQ(0xed528935U, unwinder.frames()[25].pc);
  EXPECT_EQ(0xff85db88U, unwinder.frames()[25].sp);
  EXPECT_EQ(0xed52971dU, unwinder.frames()[26].pc);
  EXPECT_EQ(0xff85dbd0U, unwinder.frames()[26].sp);
  EXPECT_EQ(0xed73c865U, unwinder.frames()[27].pc);
  EXPECT_EQ(0xff85dc10U, unwinder.frames()[27].sp);
  EXPECT_EQ(0xed7606ffU, unwinder.frames()[28].pc);
  EXPECT_EQ(0xff85dd20U, unwinder.frames()[28].sp);
  EXPECT_EQ(0xe27a7a29U, unwinder.frames()[29].pc);
  EXPECT_EQ(0xff85dd90U, unwinder.frames()[29].sp);
  EXPECT_EQ(0xed75c175U, unwinder.frames()[30].pc);
  EXPECT_EQ(0xff85ddc0U, unwinder.frames()[30].sp);
  EXPECT_EQ(0xed76122fU, unwinder.frames()[31].pc);
  EXPECT_EQ(0xff85de08U, unwinder.frames()[31].sp);
  EXPECT_EQ(0xed3b97bbU, unwinder.frames()[32].pc);
  EXPECT_EQ(0xff85de90U, unwinder.frames()[32].sp);
  EXPECT_EQ(0xed541833U, unwinder.frames()[33].pc);
  EXPECT_EQ(0xff85def8U, unwinder.frames()[33].sp);
  EXPECT_EQ(0xed528935U, unwinder.frames()[34].pc);
  EXPECT_EQ(0xff85df48U, unwinder.frames()[34].sp);
  EXPECT_EQ(0xed52971dU, unwinder.frames()[35].pc);
  EXPECT_EQ(0xff85df90U, unwinder.frames()[35].sp);
  EXPECT_EQ(0xed73c865U, unwinder.frames()[36].pc);
  EXPECT_EQ(0xff85dfd0U, unwinder.frames()[36].sp);
  EXPECT_EQ(0xed7606ffU, unwinder.frames()[37].pc);
  EXPECT_EQ(0xff85e110U, unwinder.frames()[37].sp);
  EXPECT_EQ(0xe27a739bU, unwinder.frames()[38].pc);
  EXPECT_EQ(0xff85e180U, unwinder.frames()[38].sp);
  EXPECT_EQ(0xed75c175U, unwinder.frames()[39].pc);
  EXPECT_EQ(0xff85e1b0U, unwinder.frames()[39].sp);
  EXPECT_EQ(0xed761129U, unwinder.frames()[40].pc);
  EXPECT_EQ(0xff85e1e0U, unwinder.frames()[40].sp);
  EXPECT_EQ(0xed3b97a9U, unwinder.frames()[41].pc);
  EXPECT_EQ(0xff85e268U, unwinder.frames()[41].sp);
  EXPECT_EQ(0xed541833U, unwinder.frames()[42].pc);
  EXPECT_EQ(0xff85e2d0U, unwinder.frames()[42].sp);
  EXPECT_EQ(0xed528935U, unwinder.frames()[43].pc);
  EXPECT_EQ(0xff85e320U, unwinder.frames()[43].sp);
  EXPECT_EQ(0xed52971dU, unwinder.frames()[44].pc);
  EXPECT_EQ(0xff85e368U, unwinder.frames()[44].sp);
  EXPECT_EQ(0xed73c865U, unwinder.frames()[45].pc);
  EXPECT_EQ(0xff85e3a8U, unwinder.frames()[45].sp);
  EXPECT_EQ(0xed7606ffU, unwinder.frames()[46].pc);
  EXPECT_EQ(0xff85e4c0U, unwinder.frames()[46].sp);
  EXPECT_EQ(0xe27a6aa7U, unwinder.frames()[47].pc);
  EXPECT_EQ(0xff85e530U, unwinder.frames()[47].sp);
  EXPECT_EQ(0xed75c175U, unwinder.frames()[48].pc);
  EXPECT_EQ(0xff85e5a0U, unwinder.frames()[48].sp);
  EXPECT_EQ(0xed761129U, unwinder.frames()[49].pc);
  EXPECT_EQ(0xff85e5d8U, unwinder.frames()[49].sp);
  EXPECT_EQ(0xed3b97a9U, unwinder.frames()[50].pc);
  EXPECT_EQ(0xff85e660U, unwinder.frames()[50].sp);
  EXPECT_EQ(0xed541833U, unwinder.frames()[51].pc);
  EXPECT_EQ(0xff85e6c8U, unwinder.frames()[51].sp);
  EXPECT_EQ(0xed528935U, unwinder.frames()[52].pc);
  EXPECT_EQ(0xff85e718U, unwinder.frames()[52].sp);
  EXPECT_EQ(0xed52971dU, unwinder.frames()[53].pc);
  EXPECT_EQ(0xff85e760U, unwinder.frames()[53].sp);
  EXPECT_EQ(0xed73c865U, unwinder.frames()[54].pc);
  EXPECT_EQ(0xff85e7a0U, unwinder.frames()[54].sp);
  EXPECT_EQ(0xed7606ffU, unwinder.frames()[55].pc);
  EXPECT_EQ(0xff85e8f0U, unwinder.frames()[55].sp);
  EXPECT_EQ(0xe27a1a99U, unwinder.frames()[56].pc);
  EXPECT_EQ(0xff85e960U, unwinder.frames()[56].sp);
  EXPECT_EQ(0xed75c175U, unwinder.frames()[57].pc);
  EXPECT_EQ(0xff85e990U, unwinder.frames()[57].sp);
  EXPECT_EQ(0xed76122fU, unwinder.frames()[58].pc);
  EXPECT_EQ(0xff85e9c8U, unwinder.frames()[58].sp);
  EXPECT_EQ(0xed3b97bbU, unwinder.frames()[59].pc);
  EXPECT_EQ(0xff85ea50U, unwinder.frames()[59].sp);
  EXPECT_EQ(0xed541833U, unwinder.frames()[60].pc);
  EXPECT_EQ(0xff85eab8U, unwinder.frames()[60].sp);
  EXPECT_EQ(0xed528935U, unwinder.frames()[61].pc);
  EXPECT_EQ(0xff85eb08U, unwinder.frames()[61].sp);
  EXPECT_EQ(0xed52971dU, unwinder.frames()[62].pc);
  EXPECT_EQ(0xff85eb50U, unwinder.frames()[62].sp);
  EXPECT_EQ(0xed73c865U, unwinder.frames()[63].pc);
  EXPECT_EQ(0xff85eb90U, unwinder.frames()[63].sp);
  EXPECT_EQ(0xed7606ffU, unwinder.frames()[64].pc);
  EXPECT_EQ(0xff85ec90U, unwinder.frames()[64].sp);
  EXPECT_EQ(0xed75c175U, unwinder.frames()[65].pc);
  EXPECT_EQ(0xff85ed00U, unwinder.frames()[65].sp);
  EXPECT_EQ(0xed76122fU, unwinder.frames()[66].pc);
  EXPECT_EQ(0xff85ed38U, unwinder.frames()[66].sp);
  EXPECT_EQ(0xed3b97bbU, unwinder.frames()[67].pc);
  EXPECT_EQ(0xff85edc0U, unwinder.frames()[67].sp);
  EXPECT_EQ(0xed6ac92dU, unwinder.frames()[68].pc);
  EXPECT_EQ(0xff85ee28U, unwinder.frames()[68].sp);
  EXPECT_EQ(0xed6ac6c3U, unwinder.frames()[69].pc);
  EXPECT_EQ(0xff85eeb8U, unwinder.frames()[69].sp);
  EXPECT_EQ(0xed602411U, unwinder.frames()[70].pc);
  EXPECT_EQ(0xff85ef48U, unwinder.frames()[70].sp);
  EXPECT_EQ(0xed3e0a9fU, unwinder.frames()[71].pc);
  EXPECT_EQ(0xff85ef90U, unwinder.frames()[71].sp);
  EXPECT_EQ(0xed3db9b9U, unwinder.frames()[72].pc);
  EXPECT_EQ(0xff85f008U, unwinder.frames()[72].sp);
  EXPECT_EQ(0xab0d459fU, unwinder.frames()[73].pc);
  EXPECT_EQ(0xff85f038U, unwinder.frames()[73].sp);
  EXPECT_EQ(0xab0d4349U, unwinder.frames()[74].pc);
  EXPECT_EQ(0xff85f050U, unwinder.frames()[74].sp);
  EXPECT_EQ(0xedb0d0c9U, unwinder.frames()[75].pc);
  EXPECT_EQ(0xff85f0c0U, unwinder.frames()[75].sp);
}

struct LeakType {
  LeakType(Maps* maps, Regs* regs, std::shared_ptr<Memory>& process_memory,
           size_t expected_num_frames)
      : maps(maps),
        regs(regs),
        process_memory(process_memory),
        expected_num_frames(expected_num_frames) {}

  Maps* maps;
  Regs* regs;
  std::shared_ptr<Memory>& process_memory;
  size_t expected_num_frames;
};

static void OfflineUnwind(void* data) {
  LeakType* leak_data = reinterpret_cast<LeakType*>(data);

  std::unique_ptr<Regs> regs_copy(leak_data->regs->Clone());
  std::unique_ptr<JitDebug> jit_debug =
      CreateJitDebug(leak_data->regs->Arch(), leak_data->process_memory);
  Unwinder unwinder(128, leak_data->maps, regs_copy.get(), leak_data->process_memory);
  unwinder.SetJitDebug(jit_debug.get());
  unwinder.Unwind();
  ASSERT_EQ(leak_data->expected_num_frames, unwinder.NumFrames());
}

TEST_F(UnwindOfflineTest, unwind_offline_check_for_leaks) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "jit_debug_arm/",
                            .arch = ARCH_ARM,
                            .memory_flag = ProcessMemoryFlag::kIncludeJitMemory},
                           &error_msg))
    FAIL() << error_msg;

  std::shared_ptr<Memory> process_memory = offline_utils_.GetProcessMemory();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  LeakType data(offline_utils_.GetMaps(), offline_utils_.GetRegs(), process_memory,
                expected_num_frames);
  TestCheckForLeaks(OfflineUnwind, &data);
}

// The eh_frame_hdr data is present but set to zero fdes. This should
// fallback to iterating over the cies/fdes and ignore the eh_frame_hdr.
// No .gnu_debugdata section in the elf file, so no symbols.
TEST_F(UnwindOfflineTest, bad_eh_frame_hdr_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "bad_eh_frame_hdr_arm64/", .arch = ARCH_ARM64},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x60a9fdf550U, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7fdd141990U, unwinder.frames()[0].sp);
  EXPECT_EQ(0x60a9fdf568U, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7fdd1419a0U, unwinder.frames()[1].sp);
  EXPECT_EQ(0x60a9fdf57cU, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7fdd1419b0U, unwinder.frames()[2].sp);
  EXPECT_EQ(0x60a9fdf590U, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7fdd1419c0U, unwinder.frames()[3].sp);
  EXPECT_EQ(0x7542d68e98U, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7fdd1419d0U, unwinder.frames()[4].sp);
}

// The elf has bad eh_frame unwind information for the pcs. If eh_frame
// is used first, the unwind will not match the expected output.
TEST_F(UnwindOfflineTest, debug_frame_first_x86) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "debug_frame_first_x86/", .arch = ARCH_X86},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x56598685U, unwinder.frames()[0].pc);
  EXPECT_EQ(0xffcf9e38U, unwinder.frames()[0].sp);
  EXPECT_EQ(0x565986b7U, unwinder.frames()[1].pc);
  EXPECT_EQ(0xffcf9e50U, unwinder.frames()[1].sp);
  EXPECT_EQ(0x565986d7U, unwinder.frames()[2].pc);
  EXPECT_EQ(0xffcf9e60U, unwinder.frames()[2].sp);
  EXPECT_EQ(0x565986f7U, unwinder.frames()[3].pc);
  EXPECT_EQ(0xffcf9e70U, unwinder.frames()[3].sp);
  EXPECT_EQ(0xf744a275U, unwinder.frames()[4].pc);
  EXPECT_EQ(0xffcf9e80U, unwinder.frames()[4].sp);
}

// Make sure that a pc that is at the beginning of an fde unwinds correctly.
TEST_F(UnwindOfflineTest, eh_frame_hdr_begin_x86_64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "eh_frame_hdr_begin_x86_64/", .arch = ARCH_X86_64},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x561550b17a80U, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7ffcc8596ce8U, unwinder.frames()[0].sp);
  EXPECT_EQ(0x561550b17dd9U, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7ffcc8596cf0U, unwinder.frames()[1].sp);
  EXPECT_EQ(0x561550b1821eU, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7ffcc8596f40U, unwinder.frames()[2].sp);
  EXPECT_EQ(0x561550b183edU, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7ffcc8597190U, unwinder.frames()[3].sp);
  EXPECT_EQ(0x7f4de62162b0U, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7ffcc85971a0U, unwinder.frames()[4].sp);
}

TEST_F(UnwindOfflineTest, art_quick_osr_stub_arm) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "art_quick_osr_stub_arm/",
                            .arch = ARCH_ARM,
                            .memory_flag = ProcessMemoryFlag::kIncludeJitMemory},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.SetJitDebug(offline_utils_.GetJitDebug());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0xd025c788U, unwinder.frames()[0].pc);
  EXPECT_EQ(0xcd4ff140U, unwinder.frames()[0].sp);
  EXPECT_EQ(0xd025cdd5U, unwinder.frames()[1].pc);
  EXPECT_EQ(0xcd4ff140U, unwinder.frames()[1].sp);
  EXPECT_EQ(0xe4a755bbU, unwinder.frames()[2].pc);
  EXPECT_EQ(0xcd4ff160U, unwinder.frames()[2].sp);
  EXPECT_EQ(0xe48c77a5U, unwinder.frames()[3].pc);
  EXPECT_EQ(0xcd4ff190U, unwinder.frames()[3].sp);
  EXPECT_EQ(0xe4a641a7U, unwinder.frames()[4].pc);
  EXPECT_EQ(0xcd4ff298U, unwinder.frames()[4].sp);
  EXPECT_EQ(0xe4a74474U, unwinder.frames()[5].pc);
  EXPECT_EQ(0xcd4ff2b8U, unwinder.frames()[5].sp);
  EXPECT_EQ(0xcd8365b0U, unwinder.frames()[6].pc);
  EXPECT_EQ(0xcd4ff2e0U, unwinder.frames()[6].sp);
  EXPECT_EQ(0xe4839f1bU, unwinder.frames()[7].pc);
  EXPECT_EQ(0xcd4ff2e0U, unwinder.frames()[7].sp);
  EXPECT_EQ(0xe483e593U, unwinder.frames()[8].pc);
  EXPECT_EQ(0xcd4ff330U, unwinder.frames()[8].sp);
  EXPECT_EQ(0xe4856d01U, unwinder.frames()[9].pc);
  EXPECT_EQ(0xcd4ff380U, unwinder.frames()[9].sp);
  EXPECT_EQ(0xe4a60427U, unwinder.frames()[10].pc);
  EXPECT_EQ(0xcd4ff430U, unwinder.frames()[10].sp);
  EXPECT_EQ(0xe4a67b94U, unwinder.frames()[11].pc);
  EXPECT_EQ(0xcd4ff498U, unwinder.frames()[11].sp);
  EXPECT_EQ(0x7004873eU, unwinder.frames()[12].pc);
  EXPECT_EQ(0xcd4ff4c0U, unwinder.frames()[12].sp);
  EXPECT_EQ(0xe4839f1bU, unwinder.frames()[13].pc);
  EXPECT_EQ(0xcd4ff4c0U, unwinder.frames()[13].sp);
  EXPECT_EQ(0xe483e4d5U, unwinder.frames()[14].pc);
  EXPECT_EQ(0xcd4ff510U, unwinder.frames()[14].sp);
  EXPECT_EQ(0xe4a545abU, unwinder.frames()[15].pc);
  EXPECT_EQ(0xcd4ff538U, unwinder.frames()[15].sp);
  EXPECT_EQ(0xe4a79affU, unwinder.frames()[16].pc);
  EXPECT_EQ(0xcd4ff640U, unwinder.frames()[16].sp);
  EXPECT_EQ(0xe4a75575U, unwinder.frames()[17].pc);
  EXPECT_EQ(0xcd4ff6b0U, unwinder.frames()[17].sp);
  EXPECT_EQ(0xe4a7a531U, unwinder.frames()[18].pc);
  EXPECT_EQ(0xcd4ff6e8U, unwinder.frames()[18].sp);
  EXPECT_EQ(0xe471668dU, unwinder.frames()[19].pc);
  EXPECT_EQ(0xcd4ff770U, unwinder.frames()[19].sp);
  EXPECT_EQ(0xe49c4f49U, unwinder.frames()[20].pc);
  EXPECT_EQ(0xcd4ff7c8U, unwinder.frames()[20].sp);
  EXPECT_EQ(0xe49c5cd9U, unwinder.frames()[21].pc);
  EXPECT_EQ(0xcd4ff850U, unwinder.frames()[21].sp);
  EXPECT_EQ(0xe49e71ddU, unwinder.frames()[22].pc);
  EXPECT_EQ(0xcd4ff8e8U, unwinder.frames()[22].sp);
  EXPECT_EQ(0xe7df3925U, unwinder.frames()[23].pc);
  EXPECT_EQ(0xcd4ff958U, unwinder.frames()[23].sp);
  EXPECT_EQ(0xe7daee39U, unwinder.frames()[24].pc);
  EXPECT_EQ(0xcd4ff960U, unwinder.frames()[24].sp);
}

TEST_F(UnwindOfflineTest, jit_map_arm) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "jit_map_arm/", .arch = ARCH_ARM}, &error_msg))
    FAIL() << error_msg;

  Maps* maps = offline_utils_.GetMaps();
  maps->Add(0xd025c788, 0xd025c9f0, 0, PROT_READ | PROT_EXEC | MAPS_FLAGS_JIT_SYMFILE_MAP,
            "jit_map0.so", 0);
  maps->Add(0xd025cd98, 0xd025cff4, 0, PROT_READ | PROT_EXEC | MAPS_FLAGS_JIT_SYMFILE_MAP,
            "jit_map1.so", 0);
  maps->Sort();

  Unwinder unwinder(128, maps, offline_utils_.GetRegs(), offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0xd025c788U, unwinder.frames()[0].pc);
  EXPECT_EQ(0xcd4ff140U, unwinder.frames()[0].sp);
  EXPECT_EQ(0xd025cdd5U, unwinder.frames()[1].pc);
  EXPECT_EQ(0xcd4ff140U, unwinder.frames()[1].sp);
  EXPECT_EQ(0xe4a755bbU, unwinder.frames()[2].pc);
  EXPECT_EQ(0xcd4ff160U, unwinder.frames()[2].sp);
  EXPECT_EQ(0xe49e71ddU, unwinder.frames()[3].pc);
  EXPECT_EQ(0xcd4ff8e8U, unwinder.frames()[3].sp);
  EXPECT_EQ(0xe7df3925U, unwinder.frames()[4].pc);
  EXPECT_EQ(0xcd4ff958U, unwinder.frames()[4].sp);
  EXPECT_EQ(0xe7daee39U, unwinder.frames()[5].pc);
  EXPECT_EQ(0xcd4ff960U, unwinder.frames()[5].sp);
}

TEST_F(UnwindOfflineTest, offset_arm) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "offset_arm/", .arch = ARCH_ARM}, &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x2e55fa0U, unwinder.frames()[0].pc);
  EXPECT_EQ(0xf43d2cccU, unwinder.frames()[0].sp);
  EXPECT_EQ(0x2e55febU, unwinder.frames()[1].pc);
  EXPECT_EQ(0xf43d2ce0U, unwinder.frames()[1].sp);
  EXPECT_EQ(0x2e55ff3U, unwinder.frames()[2].pc);
  EXPECT_EQ(0xf43d2ce8U, unwinder.frames()[2].sp);
  EXPECT_EQ(0x2e59ed3U, unwinder.frames()[3].pc);
  EXPECT_EQ(0xf43d2cf0U, unwinder.frames()[3].sp);
  EXPECT_EQ(0xf413652cU, unwinder.frames()[4].pc);
  EXPECT_EQ(0xf43d2d10U, unwinder.frames()[4].sp);
  EXPECT_EQ(0U, unwinder.frames()[5].pc);
  EXPECT_EQ(0xffcc0ee0U, unwinder.frames()[5].sp);
  EXPECT_EQ(0x2e562d9U, unwinder.frames()[6].pc);
  EXPECT_EQ(0xffcc0ee0U, unwinder.frames()[6].sp);
  EXPECT_EQ(0x2e56c4fU, unwinder.frames()[7].pc);
  EXPECT_EQ(0xffcc1060U, unwinder.frames()[7].sp);
  EXPECT_EQ(0x2e56c81U, unwinder.frames()[8].pc);
  EXPECT_EQ(0xffcc1078U, unwinder.frames()[8].sp);
  EXPECT_EQ(0x2e58547U, unwinder.frames()[9].pc);
  EXPECT_EQ(0xffcc1090U, unwinder.frames()[9].sp);
  EXPECT_EQ(0x2e58d99U, unwinder.frames()[10].pc);
  EXPECT_EQ(0xffcc1438U, unwinder.frames()[10].sp);
  EXPECT_EQ(0x2e7e453U, unwinder.frames()[11].pc);
  EXPECT_EQ(0xffcc1448U, unwinder.frames()[11].sp);
  EXPECT_EQ(0x2e7ede7U, unwinder.frames()[12].pc);
  EXPECT_EQ(0xffcc1458U, unwinder.frames()[12].sp);
  EXPECT_EQ(0x2e7f105U, unwinder.frames()[13].pc);
  EXPECT_EQ(0xffcc1490U, unwinder.frames()[13].sp);
  EXPECT_EQ(0x2e84215U, unwinder.frames()[14].pc);
  EXPECT_EQ(0xffcc14c0U, unwinder.frames()[14].sp);
  EXPECT_EQ(0x2e83f4fU, unwinder.frames()[15].pc);
  EXPECT_EQ(0xffcc1510U, unwinder.frames()[15].sp);
  EXPECT_EQ(0x2e773dbU, unwinder.frames()[16].pc);
  EXPECT_EQ(0xffcc1528U, unwinder.frames()[16].sp);
  EXPECT_EQ(0xf41a2c0dU, unwinder.frames()[17].pc);
  EXPECT_EQ(0xffcc1540U, unwinder.frames()[17].sp);
  EXPECT_EQ(0x2b6c02fU, unwinder.frames()[18].pc);
  EXPECT_EQ(0xffcc1558U, unwinder.frames()[18].sp);
}

// Test using a non-zero load bias library that has the fde entries
// encoded as 0xb, which is not set as pc relative.
TEST_F(UnwindOfflineTest, debug_frame_load_bias_arm) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "debug_frame_load_bias_arm/", .arch = ARCH_ARM},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0xf0be238cU, unwinder.frames()[0].pc);
  EXPECT_EQ(0xffd4a638U, unwinder.frames()[0].sp);
  EXPECT_EQ(0xf0bb240fU, unwinder.frames()[1].pc);
  EXPECT_EQ(0xffd4a638U, unwinder.frames()[1].sp);
  EXPECT_EQ(0xf1a75535U, unwinder.frames()[2].pc);
  EXPECT_EQ(0xffd4a650U, unwinder.frames()[2].sp);
  EXPECT_EQ(0xf1a75633U, unwinder.frames()[3].pc);
  EXPECT_EQ(0xffd4a6b0U, unwinder.frames()[3].sp);
  EXPECT_EQ(0xf1a75b57U, unwinder.frames()[4].pc);
  EXPECT_EQ(0xffd4a6d0U, unwinder.frames()[4].sp);
  EXPECT_EQ(0x8d1cc21U, unwinder.frames()[5].pc);
  EXPECT_EQ(0xffd4a6e8U, unwinder.frames()[5].sp);
  EXPECT_EQ(0xf0c15b89U, unwinder.frames()[6].pc);
  EXPECT_EQ(0xffd4a700U, unwinder.frames()[6].sp);
  EXPECT_EQ(0x8d1cb77U, unwinder.frames()[7].pc);
  EXPECT_EQ(0xffd4a718U, unwinder.frames()[7].sp);
}

TEST_F(UnwindOfflineTest, shared_lib_in_apk_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "shared_lib_in_apk_arm64/", .arch = ARCH_ARM64},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x7e82c4fcbcULL, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7df8ca3bf0ULL, unwinder.frames()[0].sp);
  EXPECT_EQ(0x7e82b5726cULL, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7df8ca3bf0ULL, unwinder.frames()[1].sp);
  EXPECT_EQ(0x7e82b018c0ULL, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7df8ca3da0ULL, unwinder.frames()[2].sp);
  EXPECT_EQ(0x7e7eecc6f4ULL, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7dabf3db60ULL, unwinder.frames()[3].sp);
  EXPECT_EQ(0x7e7eeccad4ULL, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7dabf3dc40ULL, unwinder.frames()[4].sp);
  EXPECT_EQ(0x7dabc405b4ULL, unwinder.frames()[5].pc);
  EXPECT_EQ(0x7dabf3dc50ULL, unwinder.frames()[5].sp);
  EXPECT_EQ(0x7e7eec7e68ULL, unwinder.frames()[6].pc);
  EXPECT_EQ(0x7dabf3dc70ULL, unwinder.frames()[6].sp);
  // Ignore top frame since the test code was modified to end in __libc_init.
}

TEST_F(UnwindOfflineTest, shared_lib_in_apk_memory_only_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init(
          {.offline_files_dir = "shared_lib_in_apk_memory_only_arm64/", .arch = ARCH_ARM64},
          &error_msg))
    FAIL() << error_msg;
  // Add the memory that represents the shared library.

  std::shared_ptr<Memory> process_memory = offline_utils_.GetProcessMemory();
  MemoryOfflineParts* memory = reinterpret_cast<MemoryOfflineParts*>(process_memory.get());
  const std::string* offline_files_path = offline_utils_.GetOfflineFilesPath();
  if (offline_files_path == nullptr) FAIL() << "GetOfflineFilesPath() failed.";

  if (!AddMemory(*offline_files_path + "lib_mem.data", memory, &error_msg)) FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(), process_memory);
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x7e82c4fcbcULL, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7df8ca3bf0ULL, unwinder.frames()[0].sp);
  EXPECT_EQ(0x7e82b5726cULL, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7df8ca3bf0ULL, unwinder.frames()[1].sp);
  EXPECT_EQ(0x7e82b018c0ULL, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7df8ca3da0ULL, unwinder.frames()[2].sp);
  EXPECT_EQ(0x7e7eecc6f4ULL, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7dabf3db60ULL, unwinder.frames()[3].sp);
  EXPECT_EQ(0x7e7eeccad4ULL, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7dabf3dc40ULL, unwinder.frames()[4].sp);
  EXPECT_EQ(0x7dabc405b4ULL, unwinder.frames()[5].pc);
  EXPECT_EQ(0x7dabf3dc50ULL, unwinder.frames()[5].sp);
  EXPECT_EQ(0x7e7eec7e68ULL, unwinder.frames()[6].pc);
  EXPECT_EQ(0x7dabf3dc70ULL, unwinder.frames()[6].sp);
  // Ignore top frame since the test code was modified to end in __libc_init.
}

TEST_F(UnwindOfflineTest, shared_lib_in_apk_single_map_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init(
          {.offline_files_dir = "shared_lib_in_apk_single_map_arm64/", .arch = ARCH_ARM64},
          &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x7cbe0b14bcULL, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7be4f077d0ULL, unwinder.frames()[0].sp);
  EXPECT_EQ(0x7be6715f5cULL, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7be4f077d0ULL, unwinder.frames()[1].sp);
  EXPECT_EQ(0x7be6715e9cULL, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7be4f07800ULL, unwinder.frames()[2].sp);
  EXPECT_EQ(0x7be6715d70ULL, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7be4f07840ULL, unwinder.frames()[3].sp);
  EXPECT_EQ(0x7be6716408ULL, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7be4f07860ULL, unwinder.frames()[4].sp);
  EXPECT_EQ(0x7be67168d8ULL, unwinder.frames()[5].pc);
  EXPECT_EQ(0x7be4f07880ULL, unwinder.frames()[5].sp);
  EXPECT_EQ(0x7be6716814ULL, unwinder.frames()[6].pc);
  EXPECT_EQ(0x7be4f078f0ULL, unwinder.frames()[6].sp);
  EXPECT_EQ(0x7be6704f60ULL, unwinder.frames()[7].pc);
  EXPECT_EQ(0x7be4f07910ULL, unwinder.frames()[7].sp);
  EXPECT_EQ(0x7be5f7b024ULL, unwinder.frames()[8].pc);
  EXPECT_EQ(0x7be4f07950ULL, unwinder.frames()[8].sp);
  EXPECT_EQ(0x7be5f7cad0ULL, unwinder.frames()[9].pc);
  EXPECT_EQ(0x7be4f07aa0ULL, unwinder.frames()[9].sp);
  EXPECT_EQ(0x7be5f7cb64ULL, unwinder.frames()[10].pc);
  EXPECT_EQ(0x7be4f07ce0ULL, unwinder.frames()[10].sp);
  EXPECT_EQ(0x7cbe11406cULL, unwinder.frames()[11].pc);
  EXPECT_EQ(0x7be4f07d00ULL, unwinder.frames()[11].sp);
  EXPECT_EQ(0x7cbe0b5e18ULL, unwinder.frames()[12].pc);
  EXPECT_EQ(0x7be4f07d20ULL, unwinder.frames()[12].sp);
}

TEST_F(UnwindOfflineTest, invalid_elf_offset_arm) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "invalid_elf_offset_arm/",
                            .arch = ARCH_ARM,
                            .memory_flag = ProcessMemoryFlag::kNoMemory},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ("  #00 pc 00aa7508  invalid.apk (offset 0x12e4000)\n", frame_info);
  EXPECT_EQ(0xc898f508, unwinder.frames()[0].pc);
  EXPECT_EQ(0xc2044218, unwinder.frames()[0].sp);
}

TEST_F(UnwindOfflineTest, load_bias_ro_rx_x86_64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "load_bias_ro_rx_x86_64/", .arch = ARCH_X86_64},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(
      "  #00 pc 00000000000e9dd4  libc.so (__write+20)\n"
      "  #01 pc 000000000007ab9c  libc.so (_IO_file_write+44)\n"
      "  #02 pc 0000000000079f3e  libc.so\n"
      "  #03 pc 000000000007bce8  libc.so (_IO_do_write+24)\n"
      "  #04 pc 000000000007b26e  libc.so (_IO_file_xsputn+270)\n"
      "  #05 pc 000000000004f7f9  libc.so (_IO_vfprintf+1945)\n"
      "  #06 pc 0000000000057cb5  libc.so (_IO_printf+165)\n"
      "  #07 pc 0000000000ed1796  perfetto_unittests "
      "(testing::internal::PrettyUnitTestResultPrinter::OnTestIterationStart(testing::UnitTest "
      "const&, int)+374)\n"
      "  #08 pc 0000000000ed30fd  perfetto_unittests "
      "(testing::internal::TestEventRepeater::OnTestIterationStart(testing::UnitTest const&, "
      "int)+125)\n"
      "  #09 pc 0000000000ed5e25  perfetto_unittests "
      "(testing::internal::UnitTestImpl::RunAllTests()+581)\n"
      "  #10 pc 0000000000ef63f3  perfetto_unittests "
      "(bool "
      "testing::internal::HandleSehExceptionsInMethodIfSupported<testing::internal::UnitTestImpl, "
      "bool>(testing::internal::UnitTestImpl*, bool (testing::internal::UnitTestImpl::*)(), char "
      "const*)+131)\n"
      "  #11 pc 0000000000ee2a21  perfetto_unittests "
      "(bool "
      "testing::internal::HandleExceptionsInMethodIfSupported<testing::internal::UnitTestImpl, "
      "bool>(testing::internal::UnitTestImpl*, bool (testing::internal::UnitTestImpl::*)(), char "
      "const*)+113)\n"
      "  #12 pc 0000000000ed5bb9  perfetto_unittests (testing::UnitTest::Run()+185)\n"
      "  #13 pc 0000000000e900f0  perfetto_unittests (RUN_ALL_TESTS()+16)\n"
      "  #14 pc 0000000000e900d8  perfetto_unittests (main+56)\n"
      "  #15 pc 000000000002352a  libc.so (__libc_start_main+234)\n"
      "  #16 pc 0000000000919029  perfetto_unittests (_start+41)\n",
      frame_info);

  EXPECT_EQ(0x7f9326a57dd4ULL, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7ffd224153c8ULL, unwinder.frames()[0].sp);
  EXPECT_EQ(0x7f93269e8b9cULL, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7ffd224153d0ULL, unwinder.frames()[1].sp);
  EXPECT_EQ(0x7f93269e7f3eULL, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7ffd22415400ULL, unwinder.frames()[2].sp);
  EXPECT_EQ(0x7f93269e9ce8ULL, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7ffd22415440ULL, unwinder.frames()[3].sp);
  EXPECT_EQ(0x7f93269e926eULL, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7ffd22415450ULL, unwinder.frames()[4].sp);
  EXPECT_EQ(0x7f93269bd7f9ULL, unwinder.frames()[5].pc);
  EXPECT_EQ(0x7ffd22415490ULL, unwinder.frames()[5].sp);
  EXPECT_EQ(0x7f93269c5cb5ULL, unwinder.frames()[6].pc);
  EXPECT_EQ(0x7ffd22415a10ULL, unwinder.frames()[6].sp);
  EXPECT_EQ(0xed1796ULL, unwinder.frames()[7].pc);
  EXPECT_EQ(0x7ffd22415af0ULL, unwinder.frames()[7].sp);
  EXPECT_EQ(0xed30fdULL, unwinder.frames()[8].pc);
  EXPECT_EQ(0x7ffd22415b70ULL, unwinder.frames()[8].sp);
  EXPECT_EQ(0xed5e25ULL, unwinder.frames()[9].pc);
  EXPECT_EQ(0x7ffd22415bb0ULL, unwinder.frames()[9].sp);
  EXPECT_EQ(0xef63f3ULL, unwinder.frames()[10].pc);
  EXPECT_EQ(0x7ffd22415c60ULL, unwinder.frames()[10].sp);
  EXPECT_EQ(0xee2a21ULL, unwinder.frames()[11].pc);
  EXPECT_EQ(0x7ffd22415cc0ULL, unwinder.frames()[11].sp);
  EXPECT_EQ(0xed5bb9ULL, unwinder.frames()[12].pc);
  EXPECT_EQ(0x7ffd22415d40ULL, unwinder.frames()[12].sp);
  EXPECT_EQ(0xe900f0ULL, unwinder.frames()[13].pc);
  EXPECT_EQ(0x7ffd22415d90ULL, unwinder.frames()[13].sp);
  EXPECT_EQ(0xe900d8ULL, unwinder.frames()[14].pc);
  EXPECT_EQ(0x7ffd22415da0ULL, unwinder.frames()[14].sp);
  EXPECT_EQ(0x7f932699152aULL, unwinder.frames()[15].pc);
  EXPECT_EQ(0x7ffd22415dd0ULL, unwinder.frames()[15].sp);
  EXPECT_EQ(0x919029ULL, unwinder.frames()[16].pc);
  EXPECT_EQ(0x7ffd22415e90ULL, unwinder.frames()[16].sp);
}

TEST_F(UnwindOfflineTest, load_bias_different_section_bias_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init(
          {.offline_files_dir = "load_bias_different_section_bias_arm64/", .arch = ARCH_ARM64},
          &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x7112cb99bcULL, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7112bdbbf0ULL, unwinder.frames()[0].sp);
  EXPECT_EQ(0x7112c394e8ULL, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7112bdbbf0ULL, unwinder.frames()[1].sp);
  EXPECT_EQ(0x7112be28c0ULL, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7112bdbda0ULL, unwinder.frames()[2].sp);
  EXPECT_EQ(0x71115ab3e8ULL, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7fdd4a3f00ULL, unwinder.frames()[3].sp);
  EXPECT_EQ(0x5f739dc9fcULL, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7fdd4a3fe0ULL, unwinder.frames()[4].sp);
  EXPECT_EQ(0x5f739edd80ULL, unwinder.frames()[5].pc);
  EXPECT_EQ(0x7fdd4a3ff0ULL, unwinder.frames()[5].sp);
  EXPECT_EQ(0x5f739ee24cULL, unwinder.frames()[6].pc);
  EXPECT_EQ(0x7fdd4a4010ULL, unwinder.frames()[6].sp);
  EXPECT_EQ(0x5f739ee558ULL, unwinder.frames()[7].pc);
  EXPECT_EQ(0x7fdd4a4040ULL, unwinder.frames()[7].sp);
  EXPECT_EQ(0x5f739f2ffcULL, unwinder.frames()[8].pc);
  EXPECT_EQ(0x7fdd4a4070ULL, unwinder.frames()[8].sp);
  EXPECT_EQ(0x5f739f2d9cULL, unwinder.frames()[9].pc);
  EXPECT_EQ(0x7fdd4a4100ULL, unwinder.frames()[9].sp);
  EXPECT_EQ(0x5f739dd4e4ULL, unwinder.frames()[10].pc);
  EXPECT_EQ(0x7fdd4a4130ULL, unwinder.frames()[10].sp);
  EXPECT_EQ(0x71115a6a34ULL, unwinder.frames()[11].pc);
  EXPECT_EQ(0x7fdd4a4170ULL, unwinder.frames()[11].sp);
}

TEST_F(UnwindOfflineTest, eh_frame_bias_x86) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "eh_frame_bias_x86/", .arch = ARCH_X86},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0xffffe430ULL, unwinder.frames()[0].pc);
  EXPECT_EQ(0xfffe1a30ULL, unwinder.frames()[0].sp);
  EXPECT_EQ(0xeb585a4bULL, unwinder.frames()[1].pc);
  EXPECT_EQ(0xfffe1a40ULL, unwinder.frames()[1].sp);
  EXPECT_EQ(0xeb5333a3ULL, unwinder.frames()[2].pc);
  EXPECT_EQ(0xfffe1a60ULL, unwinder.frames()[2].sp);
  EXPECT_EQ(0xeb5333edULL, unwinder.frames()[3].pc);
  EXPECT_EQ(0xfffe1ab0ULL, unwinder.frames()[3].sp);
  EXPECT_EQ(0xeb841ea2ULL, unwinder.frames()[4].pc);
  EXPECT_EQ(0xfffe1ae0ULL, unwinder.frames()[4].sp);
  EXPECT_EQ(0xeb83d5e7ULL, unwinder.frames()[5].pc);
  EXPECT_EQ(0xfffe1b30ULL, unwinder.frames()[5].sp);
  EXPECT_EQ(0xeb83d193ULL, unwinder.frames()[6].pc);
  EXPECT_EQ(0xfffe1bd0ULL, unwinder.frames()[6].sp);
  EXPECT_EQ(0xeb836c77ULL, unwinder.frames()[7].pc);
  EXPECT_EQ(0xfffe1c00ULL, unwinder.frames()[7].sp);
  EXPECT_EQ(0xeb518f66ULL, unwinder.frames()[8].pc);
  EXPECT_EQ(0xfffe1d00ULL, unwinder.frames()[8].sp);
  EXPECT_EQ(0xeb83460eULL, unwinder.frames()[9].pc);
  EXPECT_EQ(0xfffe1d40ULL, unwinder.frames()[9].sp);
  EXPECT_EQ(0x00000001ULL, unwinder.frames()[10].pc);
  EXPECT_EQ(0xfffe1d74ULL, unwinder.frames()[10].sp);
}

TEST_F(UnwindOfflineTest, signal_load_bias_arm) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "signal_load_bias_arm/", .arch = ARCH_ARM},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0xb6955f9eULL, unwinder.frames()[0].pc);
  EXPECT_EQ(0xf2790ce8ULL, unwinder.frames()[0].sp);
  EXPECT_EQ(0xb6955fa7ULL, unwinder.frames()[1].pc);
  EXPECT_EQ(0xf2790ce8ULL, unwinder.frames()[1].sp);
  EXPECT_EQ(0xb6955fafULL, unwinder.frames()[2].pc);
  EXPECT_EQ(0xf2790cf0ULL, unwinder.frames()[2].sp);
  EXPECT_EQ(0xb695980bULL, unwinder.frames()[3].pc);
  EXPECT_EQ(0xf2790cf8ULL, unwinder.frames()[3].sp);
  EXPECT_EQ(0xf23febd4ULL, unwinder.frames()[4].pc);
  EXPECT_EQ(0xf2790d10ULL, unwinder.frames()[4].sp);
  EXPECT_EQ(0xb695601eULL, unwinder.frames()[5].pc);
  EXPECT_EQ(0xffe67798ULL, unwinder.frames()[5].sp);
  EXPECT_EQ(0xb6956633ULL, unwinder.frames()[6].pc);
  EXPECT_EQ(0xffe67890ULL, unwinder.frames()[6].sp);
  EXPECT_EQ(0xb695664bULL, unwinder.frames()[7].pc);
  EXPECT_EQ(0xffe678a0ULL, unwinder.frames()[7].sp);
  EXPECT_EQ(0xb6958711ULL, unwinder.frames()[8].pc);
  EXPECT_EQ(0xffe678b0ULL, unwinder.frames()[8].sp);
  EXPECT_EQ(0xb6958603ULL, unwinder.frames()[9].pc);
  EXPECT_EQ(0xffe67ac8ULL, unwinder.frames()[9].sp);
  EXPECT_EQ(0xb697ffe3ULL, unwinder.frames()[10].pc);
  EXPECT_EQ(0xffe67ad8ULL, unwinder.frames()[10].sp);
  EXPECT_EQ(0xb6980b25ULL, unwinder.frames()[11].pc);
  EXPECT_EQ(0xffe67ae8ULL, unwinder.frames()[11].sp);
  EXPECT_EQ(0xb6980e27ULL, unwinder.frames()[12].pc);
  EXPECT_EQ(0xffe67b18ULL, unwinder.frames()[12].sp);
  EXPECT_EQ(0xb698893dULL, unwinder.frames()[13].pc);
  EXPECT_EQ(0xffe67b48ULL, unwinder.frames()[13].sp);
  EXPECT_EQ(0xb698860bULL, unwinder.frames()[14].pc);
  EXPECT_EQ(0xffe67bb0ULL, unwinder.frames()[14].sp);
  EXPECT_EQ(0xb6995035ULL, unwinder.frames()[15].pc);
  EXPECT_EQ(0xffe67bd0ULL, unwinder.frames()[15].sp);
  EXPECT_EQ(0xf23fe155ULL, unwinder.frames()[16].pc);
  EXPECT_EQ(0xffe67d10ULL, unwinder.frames()[16].sp);
}

TEST_F(UnwindOfflineTest, empty_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "empty_arm64/", .arch = ARCH_ARM64}, &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x72a02203a4U, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7ffb6c0b50U, unwinder.frames()[0].sp);
  EXPECT_EQ(0x72a01dd44cU, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7ffb6c0b50U, unwinder.frames()[1].sp);
  EXPECT_EQ(0x729f759ce4U, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7ffb6c0c50U, unwinder.frames()[2].sp);
  EXPECT_EQ(0x729f759e98U, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7ffb6c0ce0U, unwinder.frames()[3].sp);
  EXPECT_EQ(0x729f75a6acU, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7ffb6c0d10U, unwinder.frames()[4].sp);
  EXPECT_EQ(0x5d478af3b0U, unwinder.frames()[5].pc);
  EXPECT_EQ(0x7ffb6c0d40U, unwinder.frames()[5].sp);
  EXPECT_EQ(0x72a01cf594U, unwinder.frames()[6].pc);
  EXPECT_EQ(0x7ffb6c0f30U, unwinder.frames()[6].sp);
}

// This test has a libc.so where the __restore has been changed so
// that the signal handler match does not occur and it uses the
// fde to do the unwind.
TEST_F(UnwindOfflineTest, signal_fde_x86) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "signal_fde_x86/", .arch = ARCH_X86}, &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x5ae0d4d9U, unwinder.frames()[0].pc);
  EXPECT_EQ(0xecb37188U, unwinder.frames()[0].sp);
  EXPECT_EQ(0x5ae0d4fcU, unwinder.frames()[1].pc);
  EXPECT_EQ(0xecb37190U, unwinder.frames()[1].sp);
  EXPECT_EQ(0x5ae0d52cU, unwinder.frames()[2].pc);
  EXPECT_EQ(0xecb371b0U, unwinder.frames()[2].sp);
  EXPECT_EQ(0x5ae16f62U, unwinder.frames()[3].pc);
  EXPECT_EQ(0xecb371d0U, unwinder.frames()[3].sp);
  EXPECT_EQ(0xec169fb0U, unwinder.frames()[4].pc);
  EXPECT_EQ(0xecb371f0U, unwinder.frames()[4].sp);
  EXPECT_EQ(0x0U, unwinder.frames()[5].pc);
  EXPECT_EQ(0xffcfac6cU, unwinder.frames()[5].sp);
  EXPECT_EQ(0x5ae0d61aU, unwinder.frames()[6].pc);
  EXPECT_EQ(0xffcfac6cU, unwinder.frames()[6].sp);
  EXPECT_EQ(0x5ae0e3aaU, unwinder.frames()[7].pc);
  EXPECT_EQ(0xffcfad60U, unwinder.frames()[7].sp);
  EXPECT_EQ(0x5ae0e3eaU, unwinder.frames()[8].pc);
  EXPECT_EQ(0xffcfad90U, unwinder.frames()[8].sp);
  EXPECT_EQ(0x5ae13444U, unwinder.frames()[9].pc);
  EXPECT_EQ(0xffcfadc0U, unwinder.frames()[9].sp);
  EXPECT_EQ(0x5ae145b8U, unwinder.frames()[10].pc);
  EXPECT_EQ(0xffcfb020U, unwinder.frames()[10].sp);
  EXPECT_EQ(0x5ae93a19U, unwinder.frames()[11].pc);
  EXPECT_EQ(0xffcfb050U, unwinder.frames()[11].sp);
  EXPECT_EQ(0x5ae938c5U, unwinder.frames()[12].pc);
  EXPECT_EQ(0xffcfb090U, unwinder.frames()[12].sp);
  EXPECT_EQ(0x5ae94d3eU, unwinder.frames()[13].pc);
  EXPECT_EQ(0xffcfb0f0U, unwinder.frames()[13].sp);
  EXPECT_EQ(0x5ae958b4U, unwinder.frames()[14].pc);
  EXPECT_EQ(0xffcfb160U, unwinder.frames()[14].sp);
  EXPECT_EQ(0x5aea4cb0U, unwinder.frames()[15].pc);
  EXPECT_EQ(0xffcfb1d0U, unwinder.frames()[15].sp);
  EXPECT_EQ(0x5aea470fU, unwinder.frames()[16].pc);
  EXPECT_EQ(0xffcfb270U, unwinder.frames()[16].sp);
  EXPECT_EQ(0x5aebc31eU, unwinder.frames()[17].pc);
  EXPECT_EQ(0xffcfb2c0U, unwinder.frames()[17].sp);
  EXPECT_EQ(0x5aebb9e9U, unwinder.frames()[18].pc);
  EXPECT_EQ(0xffcfc3c0U, unwinder.frames()[18].sp);
  EXPECT_EQ(0xec161646U, unwinder.frames()[19].pc);
  EXPECT_EQ(0xffcfc3f0U, unwinder.frames()[19].sp);
}

// This test has a libc.so where the __restore_rt has been changed so
// that the signal handler match does not occur and it uses the
// fde to do the unwind.
TEST_F(UnwindOfflineTest, signal_fde_x86_64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "signal_fde_x86_64/", .arch = ARCH_X86_64},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x5bb41271e15bU, unwinder.frames()[0].pc);
  EXPECT_EQ(0x707eb5aa8320U, unwinder.frames()[0].sp);
  EXPECT_EQ(0x5bb41271e168U, unwinder.frames()[1].pc);
  EXPECT_EQ(0x707eb5aa8330U, unwinder.frames()[1].sp);
  EXPECT_EQ(0x5bb41271e178U, unwinder.frames()[2].pc);
  EXPECT_EQ(0x707eb5aa8340U, unwinder.frames()[2].sp);
  EXPECT_EQ(0x5bb412724c77U, unwinder.frames()[3].pc);
  EXPECT_EQ(0x707eb5aa8350U, unwinder.frames()[3].sp);
  EXPECT_EQ(0x707eb2ca5d10U, unwinder.frames()[4].pc);
  EXPECT_EQ(0x707eb5aa8380U, unwinder.frames()[4].sp);
  EXPECT_EQ(0x0U, unwinder.frames()[5].pc);
  EXPECT_EQ(0x7ffcaadde078U, unwinder.frames()[5].sp);
  EXPECT_EQ(0x5bb41271e244U, unwinder.frames()[6].pc);
  EXPECT_EQ(0x7ffcaadde078U, unwinder.frames()[6].sp);
  EXPECT_EQ(0x5bb41271eb44U, unwinder.frames()[7].pc);
  EXPECT_EQ(0x7ffcaadde1a0U, unwinder.frames()[7].sp);
  EXPECT_EQ(0x5bb41271eb64U, unwinder.frames()[8].pc);
  EXPECT_EQ(0x7ffcaadde1c0U, unwinder.frames()[8].sp);
  EXPECT_EQ(0x5bb412722457U, unwinder.frames()[9].pc);
  EXPECT_EQ(0x7ffcaadde1e0U, unwinder.frames()[9].sp);
  EXPECT_EQ(0x5bb412722f67U, unwinder.frames()[10].pc);
  EXPECT_EQ(0x7ffcaadde510U, unwinder.frames()[10].sp);
  EXPECT_EQ(0x5bb412773c38U, unwinder.frames()[11].pc);
  EXPECT_EQ(0x7ffcaadde530U, unwinder.frames()[11].sp);
  EXPECT_EQ(0x5bb412774f9aU, unwinder.frames()[12].pc);
  EXPECT_EQ(0x7ffcaadde560U, unwinder.frames()[12].sp);
  EXPECT_EQ(0x5bb412775a46U, unwinder.frames()[13].pc);
  EXPECT_EQ(0x7ffcaadde5b0U, unwinder.frames()[13].sp);
  EXPECT_EQ(0x5bb4127844c6U, unwinder.frames()[14].pc);
  EXPECT_EQ(0x7ffcaadde5f0U, unwinder.frames()[14].sp);
  EXPECT_EQ(0x5bb412783f61U, unwinder.frames()[15].pc);
  EXPECT_EQ(0x7ffcaadde6c0U, unwinder.frames()[15].sp);
  EXPECT_EQ(0x5bb41279a155U, unwinder.frames()[16].pc);
  EXPECT_EQ(0x7ffcaadde720U, unwinder.frames()[16].sp);
  EXPECT_EQ(0x707eb2c9c405U, unwinder.frames()[17].pc);
  EXPECT_EQ(0x7ffcaaddf870U, unwinder.frames()[17].sp);
}

TEST_F(UnwindOfflineTest, pauth_pc_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "pauth_pc_arm64/", .arch = ARCH_ARM64},
                           &error_msg))
    FAIL() << error_msg;

  static_cast<RegsArm64*>(offline_utils_.GetRegs())->SetPACMask(0x007fff8000000000ULL);

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);
  EXPECT_EQ(0x5c390884a8U, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7ff3511750U, unwinder.frames()[0].sp);
  EXPECT_EQ(0x5c39088270U, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7ff3511770U, unwinder.frames()[1].sp);
  EXPECT_EQ(0x5c39074640U, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7ff3511930U, unwinder.frames()[2].sp);
  EXPECT_EQ(0x5c39074588U, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7ff3511960U, unwinder.frames()[3].sp);
  EXPECT_EQ(0x5c390746a8U, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7ff35119a0U, unwinder.frames()[4].sp);
  EXPECT_EQ(0x5c39074588U, unwinder.frames()[5].pc);
  EXPECT_EQ(0x7ff35119d0U, unwinder.frames()[5].sp);
  EXPECT_EQ(0x5c390746a8U, unwinder.frames()[6].pc);
  EXPECT_EQ(0x7ff3511a10U, unwinder.frames()[6].sp);
  EXPECT_EQ(0x5c39074588U, unwinder.frames()[7].pc);
  EXPECT_EQ(0x7ff3511a40U, unwinder.frames()[7].sp);
  EXPECT_EQ(0x5c390746a8U, unwinder.frames()[8].pc);
  EXPECT_EQ(0x7ff3511a80U, unwinder.frames()[8].sp);
  EXPECT_EQ(0x5c39074588U, unwinder.frames()[9].pc);
  EXPECT_EQ(0x7ff3511ab0U, unwinder.frames()[9].sp);
  EXPECT_EQ(0x5c390746a8U, unwinder.frames()[10].pc);
  EXPECT_EQ(0x7ff3511af0U, unwinder.frames()[10].sp);
  EXPECT_EQ(0x5c39074588U, unwinder.frames()[11].pc);
  EXPECT_EQ(0x7ff3511b20U, unwinder.frames()[11].sp);
  EXPECT_EQ(0x5c390746a8U, unwinder.frames()[12].pc);
  EXPECT_EQ(0x7ff3511b60U, unwinder.frames()[12].sp);
  EXPECT_EQ(0x5c39074588U, unwinder.frames()[13].pc);
  EXPECT_EQ(0x7ff3511b90U, unwinder.frames()[13].sp);
  EXPECT_EQ(0x5c390746a8U, unwinder.frames()[14].pc);
  EXPECT_EQ(0x7ff3511bd0U, unwinder.frames()[14].sp);
  EXPECT_EQ(0x5c39074588U, unwinder.frames()[15].pc);
  EXPECT_EQ(0x7ff3511c00U, unwinder.frames()[15].sp);
  EXPECT_EQ(0x5c390746a8U, unwinder.frames()[16].pc);
  EXPECT_EQ(0x7ff3511c40U, unwinder.frames()[16].sp);
  EXPECT_EQ(0x5c39074588U, unwinder.frames()[17].pc);
  EXPECT_EQ(0x7ff3511c70U, unwinder.frames()[17].sp);
  EXPECT_EQ(0x5c390746a8U, unwinder.frames()[18].pc);
  EXPECT_EQ(0x7ff3511cb0U, unwinder.frames()[18].sp);
  EXPECT_EQ(0x5c39074588U, unwinder.frames()[19].pc);
  EXPECT_EQ(0x7ff3511ce0U, unwinder.frames()[19].sp);
  EXPECT_EQ(0x5c390746a8U, unwinder.frames()[20].pc);
  EXPECT_EQ(0x7ff3511d20U, unwinder.frames()[20].sp);
  EXPECT_EQ(0x5c39086e54U, unwinder.frames()[21].pc);
  EXPECT_EQ(0x7ff3511d50U, unwinder.frames()[21].sp);
  EXPECT_EQ(0x5c3907c834U, unwinder.frames()[22].pc);
  EXPECT_EQ(0x7ff3511db0U, unwinder.frames()[22].sp);
  EXPECT_EQ(0x5c3907c2ccU, unwinder.frames()[23].pc);
  EXPECT_EQ(0x7ff3511dc0U, unwinder.frames()[23].sp);
  EXPECT_EQ(0x5c3907c8b4U, unwinder.frames()[24].pc);
  EXPECT_EQ(0x7ff3511e40U, unwinder.frames()[24].sp);
  EXPECT_EQ(0x7e4ede29d8U, unwinder.frames()[25].pc);
  EXPECT_EQ(0x7ff3511e70U, unwinder.frames()[25].sp);
}

TEST_F(UnwindOfflineTest, profiler_like_multi_process) {
  ConsecutiveUnwindTest(std::vector<UnwindSampleInfo>{
      {.offline_files_dir = "bluetooth_arm64/pc_1/", .arch = ARCH_ARM64},
      {.offline_files_dir = "jit_debug_arm/",
       .arch = ARCH_ARM,
       .memory_flag = ProcessMemoryFlag::kIncludeJitMemory},
      {.offline_files_dir = "photos_reset_arm64/", .arch = ARCH_ARM64},
      {.offline_files_dir = "youtube_compiled_arm64/", .arch = ARCH_ARM64},
      {.offline_files_dir = "yt_music_arm64/", .arch = ARCH_ARM64},
      {.offline_files_dir = "maps_compiled_arm64/28656_oat_odex_jar/", .arch = ARCH_ARM64}});
}

TEST_F(UnwindOfflineTest, profiler_like_single_process_multi_thread) {
  ConsecutiveUnwindTest(std::vector<UnwindSampleInfo>{
      {.offline_files_dir = "maps_compiled_arm64/28656_oat_odex_jar/", .arch = ARCH_ARM64},
      {.offline_files_dir = "maps_compiled_arm64/28613_main-thread/", .arch = ARCH_ARM64},
      {.offline_files_dir = "maps_compiled_arm64/28644/", .arch = ARCH_ARM64},
      {.offline_files_dir = "maps_compiled_arm64/28648/", .arch = ARCH_ARM64},
      {.offline_files_dir = "maps_compiled_arm64/28667/", .arch = ARCH_ARM64}});
}

TEST_F(UnwindOfflineTest, profiler_like_single_thread_diverse_pcs) {
  ConsecutiveUnwindTest(std::vector<UnwindSampleInfo>{
      {.offline_files_dir = "bluetooth_arm64/pc_1/", .arch = ARCH_ARM64},
      {.offline_files_dir = "bluetooth_arm64/pc_2/", .arch = ARCH_ARM64},
      {.offline_files_dir = "bluetooth_arm64/pc_3/", .arch = ARCH_ARM64},
      {.offline_files_dir = "bluetooth_arm64/pc_4/", .arch = ARCH_ARM64}});
}

static void VerifyApkRORX(Unwinder& unwinder) {
  EXPECT_EQ(0x7426d2e030U, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7fe740cc90U, unwinder.frames()[0].sp);
  EXPECT_EQ(0x7426d2e08cU, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7fe740ccd0U, unwinder.frames()[1].sp);
  EXPECT_EQ(0x7426d2e0b8U, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7fe740ccf0U, unwinder.frames()[2].sp);
  EXPECT_EQ(0x7426d2e0e4U, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7fe740cd10U, unwinder.frames()[3].sp);
  EXPECT_EQ(0x603b0c5154U, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7fe740cd30U, unwinder.frames()[4].sp);
  EXPECT_EQ(0x76b6df0b10U, unwinder.frames()[5].pc);
  EXPECT_EQ(0x7fe740cdb0U, unwinder.frames()[5].sp);
}

TEST_F(UnwindOfflineTest, apk_rorx_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "apk_rorx_arm64/", .arch = ARCH_ARM64},
                           &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);

  VerifyApkRORX(unwinder);
}

TEST_F(UnwindOfflineTest, apk_rorx_unreadable_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "apk_rorx_unreadable_arm64/", .arch = ARCH_ARM64},
                           &error_msg))
    FAIL() << error_msg;

  // Create a process memory object that holds the apk data in memory
  // along with the stack data.
  MemoryOffline* stack_memory = new MemoryOffline;
  ASSERT_TRUE(stack_memory->Init("stack.data", 0));

  MemoryOffline* apk_memory = new MemoryOffline;
  auto info1 = offline_utils_.GetMaps()->Find(0x7426d2d000);
  ASSERT_TRUE(info1 != nullptr);
  auto info2 = offline_utils_.GetMaps()->Find(0x7426d2e000);
  ASSERT_TRUE(info2 != nullptr);
  ASSERT_TRUE(
      apk_memory->Init("fake.apk", info1->offset(), info1->start(), info2->end() - info1->start()));

  std::unique_ptr<MemoryOfflineParts> parts(new MemoryOfflineParts);
  parts->Add(stack_memory);
  parts->Add(apk_memory);

  std::shared_ptr<Memory> process_memory(parts.release());

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(), process_memory);
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);

  VerifyApkRORX(unwinder);
}

static void VerifyApkRX(Unwinder& unwinder) {
  EXPECT_EQ(0x7cb0e6266cU, unwinder.frames()[0].pc);
  EXPECT_EQ(0x7fe563be90U, unwinder.frames()[0].sp);
  EXPECT_EQ(0x7cb0e626c0U, unwinder.frames()[1].pc);
  EXPECT_EQ(0x7fe563bed0U, unwinder.frames()[1].sp);
  EXPECT_EQ(0x7cb0e626ecU, unwinder.frames()[2].pc);
  EXPECT_EQ(0x7fe563bef0U, unwinder.frames()[2].sp);
  EXPECT_EQ(0x7cb0e62718U, unwinder.frames()[3].pc);
  EXPECT_EQ(0x7fe563bf10U, unwinder.frames()[3].sp);
  EXPECT_EQ(0x5e004f0154U, unwinder.frames()[4].pc);
  EXPECT_EQ(0x7fe563bf30U, unwinder.frames()[4].sp);
  EXPECT_EQ(0x7f41124b10U, unwinder.frames()[5].pc);
  EXPECT_EQ(0x7fe563bfb0U, unwinder.frames()[5].sp);
}

TEST_F(UnwindOfflineTest, apk_rx_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "apk_rx_arm64/", .arch = ARCH_ARM64}, &error_msg))
    FAIL() << error_msg;

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(),
                    offline_utils_.GetProcessMemory());
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);

  VerifyApkRX(unwinder);
}

TEST_F(UnwindOfflineTest, apk_rx_unreadable_arm64) {
  std::string error_msg;
  if (!offline_utils_.Init({.offline_files_dir = "apk_rx_unreadable_arm64/", .arch = ARCH_ARM64},
                           &error_msg))
    FAIL() << error_msg;

  // Create a process memory object that holds the apk data in memory
  // along with the stack data.
  MemoryOffline* stack_memory = new MemoryOffline;
  ASSERT_TRUE(stack_memory->Init("stack.data", 0));

  MemoryOffline* apk_memory = new MemoryOffline;
  auto info = offline_utils_.GetMaps()->Find(0x7cb0e62000);
  ASSERT_TRUE(info != nullptr);
  ASSERT_TRUE(
      apk_memory->Init("fake.apk", info->offset(), info->start(), info->end() - info->start()));

  std::unique_ptr<MemoryOfflineParts> parts(new MemoryOfflineParts);
  parts->Add(stack_memory);
  parts->Add(apk_memory);

  std::shared_ptr<Memory> process_memory(parts.release());

  Unwinder unwinder(128, offline_utils_.GetMaps(), offline_utils_.GetRegs(), process_memory);
  unwinder.Unwind();

  size_t expected_num_frames;
  if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg)) FAIL() << error_msg;
  std::string expected_frame_info;
  if (!GetExpectedSamplesFrameInfo(&expected_frame_info, &error_msg)) FAIL() << error_msg;

  std::string frame_info(DumpFrames(unwinder));
  ASSERT_EQ(expected_num_frames, unwinder.NumFrames()) << "Unwind:\n" << frame_info;
  EXPECT_EQ(expected_frame_info, frame_info);

  VerifyApkRX(unwinder);
}

}  // namespace
}  // namespace unwindstack
