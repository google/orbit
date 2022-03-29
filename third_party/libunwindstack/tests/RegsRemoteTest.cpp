/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <memory>

#include <gtest/gtest.h>

#include <unwindstack/Regs.h>

#include "TestUtils.h"

namespace unwindstack {

class RegsRemoteTest : public ::testing::Test {
 protected:
  void SetUp() override {
    if ((pid_ = fork()) == 0) {
      volatile bool run = true;
      while (!run) {
      }
      exit(1);
    }
    ASSERT_TRUE(pid_ != -1);
    ASSERT_TRUE(TestAttach(pid_));
    ASSERT_TRUE(TestQuiescePid(pid_));
  }

  void TearDown() override {
    if (pid_ == -1) {
      return;
    }
    EXPECT_TRUE(TestDetach(pid_));
    kill(pid_, SIGKILL);
    waitpid(pid_, nullptr, 0);
  }

  pid_t pid_ = -1;
};

TEST_F(RegsRemoteTest, remote_get) {
  std::unique_ptr<Regs> regs(Regs::RemoteGet(pid_));
#if defined(__arm__)
  ASSERT_EQ(ARCH_ARM, regs->Arch());
#elif defined(__aarch64__)
  ASSERT_EQ(ARCH_ARM64, regs->Arch());
#elif defined(__i386__)
  ASSERT_EQ(ARCH_X86, regs->Arch());
#elif defined(__x86_64__)
  ASSERT_EQ(ARCH_X86_64, regs->Arch());
#else
  ASSERT_EQ(nullptr, regs.get());
#endif
}

TEST_F(RegsRemoteTest, remote_get_arch) {
#if defined(__arm__)
  ASSERT_EQ(ARCH_ARM, Regs::RemoteGetArch(pid_));
#elif defined(__aarch64__)
  ASSERT_EQ(ARCH_ARM64, Regs::RemoteGetArch(pid_));
#elif defined(__i386__)
  ASSERT_EQ(ARCH_X86, Regs::RemoteGetArch(pid_));
#elif defined(__x86_64__)
  ASSERT_EQ(ARCH_X86_64, Regs::RemoteGetArch(pid_));
#else
  ASSERT_EQ(ARCH_NONE, Regs::RemoteGetArch(pid_));
#endif
}

}  // namespace unwindstack
