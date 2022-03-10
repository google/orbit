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

#include <dlfcn.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include <android-base/strings.h>
#include <android-base/threads.h>

#include <unwindstack/AndroidUnwinder.h>
#include <unwindstack/Error.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsArm.h>
#include <unwindstack/RegsArm64.h>
#include <unwindstack/RegsGetLocal.h>
#include <unwindstack/RegsX86.h>
#include <unwindstack/RegsX86_64.h>
#include <unwindstack/UcontextArm.h>
#include <unwindstack/UcontextArm64.h>
#include <unwindstack/UcontextX86.h>
#include <unwindstack/UcontextX86_64.h>
#include <unwindstack/Unwinder.h>

#include "PidUtils.h"
#include "TestUtils.h"

namespace unwindstack {

static std::string GetBacktrace(AndroidUnwinder& unwinder, std::vector<FrameData>& frames) {
  std::string backtrace_str;
  for (auto& frame : frames) {
    backtrace_str += unwinder.FormatFrame(frame) + '\n';
  }
  return backtrace_str;
}

static pid_t ForkWaitForever() {
  pid_t pid;
  if ((pid = fork()) == 0) {
    // Do a loop that guarantees the terminating leaf frame will be in
    // the test executable and not any other library function.
    bool run = true;
    while (run) {
      DoNotOptimize(run = true);
    }
    exit(1);
  }
  return pid;
}

TEST(AndroidUnwinderDataTest, demangle_function_names) {
  AndroidUnwinderData data;

  // Add a few frames with and without demangled function names.
  data.frames.resize(4);
  data.frames[0].function_name = "no_demangle()";
  data.frames[1].function_name = "_Z4fakeb";
  data.frames[3].function_name = "_Z8demanglei";

  data.DemangleFunctionNames();
  EXPECT_EQ("no_demangle()", data.frames[0].function_name);
  EXPECT_EQ("fake(bool)", data.frames[1].function_name);
  EXPECT_EQ("", data.frames[2].function_name);
  EXPECT_EQ("demangle(int)", data.frames[3].function_name);

  // Make sure that this action is idempotent.
  data.DemangleFunctionNames();
  EXPECT_EQ("no_demangle()", data.frames[0].function_name);
  EXPECT_EQ("fake(bool)", data.frames[1].function_name);
  EXPECT_EQ("", data.frames[2].function_name);
  EXPECT_EQ("demangle(int)", data.frames[3].function_name);
}

TEST(AndroidUnwinderDataTest, get_error_string) {
  AndroidUnwinderData data;

  EXPECT_EQ("None", data.GetErrorString());
  data.error.code = ERROR_INVALID_ELF;
  EXPECT_EQ("Invalid Elf", data.GetErrorString());
  data.error.code = ERROR_MEMORY_INVALID;
  EXPECT_EQ("Memory Invalid", data.GetErrorString());
  data.error.address = 0x1000;
  EXPECT_EQ("Memory Invalid at address 0x1000", data.GetErrorString());
}

TEST(AndroidUnwinderTest, unwind_errors) {
  AndroidLocalUnwinder unwinder;

  AndroidUnwinderData data;
  void* ucontext = nullptr;
  EXPECT_FALSE(unwinder.Unwind(ucontext, data));
  EXPECT_EQ(ERROR_INVALID_PARAMETER, data.error.code);
  std::unique_ptr<Regs> regs;
  EXPECT_FALSE(unwinder.Unwind(regs.get(), data));
  EXPECT_EQ(ERROR_INVALID_PARAMETER, data.error.code);
  // Make sure that we are using a different arch from the
  // current arch.
  if (Regs::CurrentArch() == ARCH_ARM) {
    regs.reset(new RegsArm64);
  } else {
    regs.reset(new RegsArm);
  }
  EXPECT_FALSE(unwinder.Unwind(regs.get(), data));
  EXPECT_EQ(ERROR_BAD_ARCH, data.error.code);
}

TEST(AndroidUnwinderTest, create) {
  // Verify the local unwinder object is created.
  std::unique_ptr<AndroidUnwinder> unwinder(AndroidUnwinder::Create(getpid()));
  AndroidUnwinderData data;
  ASSERT_TRUE(unwinder->Unwind(data));

  pid_t pid = ForkWaitForever();
  ASSERT_NE(-1, pid);
  TestScopedPidReaper reap(pid);

  ASSERT_TRUE(RunWhenQuiesced(pid, false, [pid, &unwinder]() {
    // Verify the remote unwinder object is created.
    unwinder.reset(AndroidUnwinder::Create(pid));
    AndroidUnwinderData data;
    if (!unwinder->Unwind(data)) {
      printf("Failed to unwind %s\n", data.GetErrorString().c_str());
      return PID_RUN_FAIL;
    }
    return PID_RUN_PASS;
  }));
}

TEST(AndroidLocalUnwinderTest, initialize_before) {
  AndroidLocalUnwinder unwinder;
  ErrorData error;
  ASSERT_TRUE(unwinder.Initialize(error));

  AndroidUnwinderData data;
  ASSERT_TRUE(unwinder.Unwind(data));
}

TEST(AndroidLocalUnwinderTest, suffix_ignore) {
  AndroidLocalUnwinder unwinder(std::vector<std::string>{}, std::vector<std::string>{"so"});
  AndroidUnwinderData data;
  // This should work as long as the first frame is in the test executable.
  ASSERT_TRUE(unwinder.Unwind(data));
  // Make sure the unwind doesn't include any .so frames.
  for (const auto& frame : data.frames) {
    ASSERT_TRUE(frame.map_info == nullptr ||
                !android::base::EndsWith(frame.map_info->name(), ".so"))
        << GetBacktrace(unwinder, data.frames);
  }
}

TEST(AndroidUnwinderTest, verify_all_unwind_functions) {
  AndroidLocalUnwinder unwinder;
  AndroidUnwinderData data;
  ASSERT_TRUE(unwinder.Unwind(data));
  ASSERT_TRUE(unwinder.Unwind(std::nullopt, data));
  ASSERT_TRUE(unwinder.Unwind(getpid(), data));
  std::unique_ptr<Regs> regs(Regs::CreateFromLocal());
  RegsGetLocal(regs.get());

  void* ucontext;
  switch (regs->Arch()) {
    case ARCH_ARM: {
      arm_ucontext_t* arm_ucontext =
          reinterpret_cast<arm_ucontext_t*>(malloc(sizeof(arm_ucontext_t)));
      ucontext = arm_ucontext;
      memcpy(&arm_ucontext->uc_mcontext.regs[0], regs->RawData(), ARM_REG_LAST * sizeof(uint32_t));
    } break;
    case ARCH_ARM64: {
      arm64_ucontext_t* arm64_ucontext =
          reinterpret_cast<arm64_ucontext_t*>(malloc(sizeof(arm64_ucontext_t)));
      ucontext = arm64_ucontext;
      memcpy(&arm64_ucontext->uc_mcontext.regs[0], regs->RawData(),
             ARM64_REG_LAST * sizeof(uint64_t));
    } break;
    case ARCH_X86: {
      x86_ucontext_t* x86_ucontext =
          reinterpret_cast<x86_ucontext_t*>(malloc(sizeof(x86_ucontext_t)));
      ucontext = x86_ucontext;
      RegsX86* regs_x86 = static_cast<RegsX86*>(regs.get());

      x86_ucontext->uc_mcontext.edi = (*regs_x86)[X86_REG_EDI];
      x86_ucontext->uc_mcontext.esi = (*regs_x86)[X86_REG_ESI];
      x86_ucontext->uc_mcontext.ebp = (*regs_x86)[X86_REG_EBP];
      x86_ucontext->uc_mcontext.esp = (*regs_x86)[X86_REG_ESP];
      x86_ucontext->uc_mcontext.ebx = (*regs_x86)[X86_REG_EBX];
      x86_ucontext->uc_mcontext.edx = (*regs_x86)[X86_REG_EDX];
      x86_ucontext->uc_mcontext.ecx = (*regs_x86)[X86_REG_ECX];
      x86_ucontext->uc_mcontext.eax = (*regs_x86)[X86_REG_EAX];
      x86_ucontext->uc_mcontext.eip = (*regs_x86)[X86_REG_EIP];
    } break;
    case ARCH_X86_64: {
      x86_64_ucontext_t* x86_64_ucontext =
          reinterpret_cast<x86_64_ucontext_t*>(malloc(sizeof(x86_64_ucontext_t)));
      ucontext = x86_64_ucontext;
      RegsX86_64* regs_x86_64 = static_cast<RegsX86_64*>(regs.get());

      memcpy(&x86_64_ucontext->uc_mcontext.r8, &(*regs_x86_64)[X86_64_REG_R8],
             8 * sizeof(uint64_t));

      x86_64_ucontext->uc_mcontext.rdi = (*regs_x86_64)[X86_64_REG_RDI];
      x86_64_ucontext->uc_mcontext.rsi = (*regs_x86_64)[X86_64_REG_RSI];
      x86_64_ucontext->uc_mcontext.rbp = (*regs_x86_64)[X86_64_REG_RBP];
      x86_64_ucontext->uc_mcontext.rbx = (*regs_x86_64)[X86_64_REG_RBX];
      x86_64_ucontext->uc_mcontext.rdx = (*regs_x86_64)[X86_64_REG_RDX];
      x86_64_ucontext->uc_mcontext.rax = (*regs_x86_64)[X86_64_REG_RAX];
      x86_64_ucontext->uc_mcontext.rcx = (*regs_x86_64)[X86_64_REG_RCX];
      x86_64_ucontext->uc_mcontext.rsp = (*regs_x86_64)[X86_64_REG_RSP];
      x86_64_ucontext->uc_mcontext.rip = (*regs_x86_64)[X86_64_REG_RIP];
    } break;
    default:
      ucontext = nullptr;
      break;
  }
  ASSERT_TRUE(ucontext != nullptr);
  ASSERT_TRUE(unwinder.Unwind(ucontext, data));
  free(ucontext);
  AndroidUnwinderData reg_data;
  ASSERT_TRUE(unwinder.Unwind(regs.get(), reg_data));
  ASSERT_EQ(data.frames.size(), reg_data.frames.size());
  // Make sure all of the frame data is exactly the same.
  for (size_t i = 0; i < data.frames.size(); i++) {
    SCOPED_TRACE("\nMismatch at Frame " + std::to_string(i) + "\nucontext trace:\n" +
                 GetBacktrace(unwinder, data.frames) + "\nregs trace:\n" +
                 GetBacktrace(unwinder, reg_data.frames));
    const auto& frame_context = data.frames[i];
    const auto& frame_reg = reg_data.frames[i];
    ASSERT_EQ(frame_context.num, frame_reg.num);
    ASSERT_EQ(frame_context.rel_pc, frame_reg.rel_pc);
    ASSERT_EQ(frame_context.pc, frame_reg.pc);
    ASSERT_EQ(frame_context.sp, frame_reg.sp);
    ASSERT_STREQ(frame_context.function_name.c_str(), frame_reg.function_name.c_str());
    ASSERT_EQ(frame_context.function_offset, frame_reg.function_offset);
    ASSERT_EQ(frame_context.map_info.get(), frame_reg.map_info.get());
  }
}

TEST(AndroidLocalUnwinderTest, unwind_current_thread) {
  AndroidLocalUnwinder unwinder;
  AndroidUnwinderData data;
  ASSERT_TRUE(unwinder.Unwind(data));
  // Verify that the libunwindstack.so does not appear in the first frame.
  ASSERT_TRUE(data.frames[0].map_info == nullptr ||
              !android::base::EndsWith(data.frames[0].map_info->name(), "/libunwindstack.so"))
      << "libunwindstack.so not removed properly\n"
      << GetBacktrace(unwinder, data.frames);
}

TEST(AndroidLocalUnwinderTest, unwind_current_thread_show_all_frames) {
  GTEST_SKIP();
  AndroidLocalUnwinder unwinder;
  AndroidUnwinderData data(true);
  ASSERT_TRUE(unwinder.Unwind(data));
  // Verify that the libunwindstack.so does appear in the first frame.
  ASSERT_TRUE(data.frames[0].map_info != nullptr &&
              android::base::EndsWith(data.frames[0].map_info->name(), "/libunwindstack.so"))
      << "libunwindstack.so was removed improperly\n"
      << GetBacktrace(unwinder, data.frames);
}

TEST(AndroidLocalUnwinderTest, unwind_different_thread) {
  std::atomic<pid_t> tid;
  std::atomic_bool keep_running = true;
  std::thread thread([&tid, &keep_running] {
    tid = android::base::GetThreadId();
    while (keep_running) {
    }
    return nullptr;
  });

  while (tid == 0) {
  }

  {
    AndroidLocalUnwinder unwinder;
    AndroidUnwinderData data;
    ASSERT_TRUE(unwinder.Unwind(data));
    // Verify that the libunwindstack.so does not appear in the first frame.
    ASSERT_TRUE(data.frames[0].map_info == nullptr ||
                !android::base::EndsWith(data.frames[0].map_info->name(), "/libunwindstack.so"))
        << "libunwindstack.so not removed properly\n"
        << GetBacktrace(unwinder, data.frames);
  }

  {
    AndroidLocalUnwinder unwinder;
    AndroidUnwinderData data(true);
    ASSERT_TRUE(unwinder.Unwind(data));
    // Verify that the libunwindstack.so does appear in the first frame.
    ASSERT_TRUE(data.frames[0].map_info != nullptr &&
                android::base::EndsWith(data.frames[0].map_info->name(), "/libunwindstack.so"))
        << "libunwindstack.so was removed improperly\n"
        << GetBacktrace(unwinder, data.frames);
  }

  // Allow the thread to terminate normally.
  keep_running = false;
  thread.join();
}

TEST(AndroidRemoteUnwinderTest, initialize_before) {
  pid_t pid = ForkWaitForever();
  ASSERT_NE(-1, pid);
  TestScopedPidReaper reap(pid);

  ASSERT_TRUE(Attach(pid));

  AndroidRemoteUnwinder unwinder(pid);
  ErrorData error;
  ASSERT_TRUE(unwinder.Initialize(error));

  AndroidUnwinderData data;
  ASSERT_TRUE(unwinder.Unwind(data));

  ASSERT_TRUE(Detach(pid));
}

static bool Verify(pid_t pid, std::function<PidRunEnum(const FrameData& frame)> fn) {
  return RunWhenQuiesced(pid, false, [pid, &fn]() {
    AndroidRemoteUnwinder unwinder(pid);
    AndroidUnwinderData data;
    if (!unwinder.Unwind(data)) {
      printf("Failed to unwind %s\n", data.GetErrorString().c_str());
      return PID_RUN_FAIL;
    }
    const auto& frame = data.frames[0];
    return fn(frame);
  });
}

TEST(AndroidRemoteUnwinderTest, skip_libraries) {
  void* test_lib = GetTestLibHandle();
  ASSERT_TRUE(test_lib != nullptr);
  int (*wait_func)() = reinterpret_cast<int (*)()>(dlsym(test_lib, "WaitForever"));
  ASSERT_TRUE(wait_func != nullptr);

  pid_t pid;
  if ((pid = fork()) == 0) {
    DoNotOptimize(wait_func());
    exit(0);
  }
  ASSERT_NE(-1, pid);
  TestScopedPidReaper reap(pid);

  ASSERT_TRUE(Verify(pid, [pid](const FrameData& frame) {
    // Make sure that the frame is in the dlopen'd library before proceeding.
    if (frame.map_info == nullptr ||
        !android::base::EndsWith(frame.map_info->name(), "/libunwindstack_local.so")) {
      return PID_RUN_KEEP_GOING;
    }

    // Do an unwind removing the libunwindstack_local.so library.
    AndroidRemoteUnwinder unwinder(pid, std::vector<std::string>{"libunwindstack_local.so"});
    AndroidUnwinderData data;
    if (!unwinder.Unwind(data)) {
      printf("Failed to unwind %s\n", data.GetErrorString().c_str());
      return PID_RUN_FAIL;
    }

    // Verify that library is properly ignored.
    if (android::base::EndsWith(data.frames[0].map_info->name(), "/libunwindstack_local.so")) {
      printf("Failed to strip libunwindstack_local.so\n%s\n",
             GetBacktrace(unwinder, data.frames).c_str());
      return PID_RUN_FAIL;
    }
    return PID_RUN_PASS;
  }));
}

TEST(AndroidRemoteUnwinderTest, suffix_ignore) {
  pid_t pid = ForkWaitForever();
  ASSERT_NE(-1, pid);
  TestScopedPidReaper reap(pid);

  ASSERT_TRUE(Verify(pid, [pid](const FrameData& frame) {
    // Wait until the forked process is no longer in libc.so.
    if (frame.map_info != nullptr && android::base::EndsWith(frame.map_info->name(), ".so")) {
      return PID_RUN_KEEP_GOING;
    }

    AndroidRemoteUnwinder unwinder(pid, std::vector<std::string>{}, std::vector<std::string>{"so"});
    AndroidUnwinderData data;
    if (!unwinder.Unwind(data)) {
      printf("Failed to unwind %s\n", data.GetErrorString().c_str());

      AndroidRemoteUnwinder normal_unwinder(pid);
      if (normal_unwinder.Unwind(data)) {
        printf("Full unwind %s\n", GetBacktrace(normal_unwinder, data.frames).c_str());
      }
      return PID_RUN_FAIL;
    }

    // Make sure the unwind doesn't include any .so frames.
    for (const auto& frame : data.frames) {
      if (frame.map_info != nullptr && android::base::EndsWith(frame.map_info->name(), ".so")) {
        printf("Found unexpected .so frame\n%s\n", GetBacktrace(unwinder, data.frames).c_str());
        return PID_RUN_FAIL;
      }
    }
    return PID_RUN_PASS;
  }));
}

}  // namespace unwindstack
