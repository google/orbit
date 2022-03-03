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

#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <android-base/stringprintf.h>
#include <android-base/threads.h>

#include <unwindstack/Maps.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsGetLocal.h>
#include <unwindstack/Unwinder.h>

#include "MemoryRemote.h"
#include "TestUtils.h"

namespace unwindstack {

enum TestTypeEnum : uint8_t {
  TEST_TYPE_LOCAL_UNWINDER = 0,
  TEST_TYPE_LOCAL_UNWINDER_FROM_PID,
  TEST_TYPE_LOCAL_WAIT_FOR_FINISH,
  TEST_TYPE_REMOTE,
  TEST_TYPE_REMOTE_WITH_INVALID_CALL,
};

static volatile bool g_ready_for_remote;
static volatile bool g_signal_ready_for_remote;
// In order to avoid the compiler not emitting the unwind entries for
// the InnerFunction code that loops waiting for g_finish, always make
// g_finish a volatile instead of an atomic. This issue was only ever
// observerd on the arm architecture.
static volatile bool g_finish;
static std::atomic_uintptr_t g_ucontext;
static std::atomic_int g_waiters;

static void ResetGlobals() {
  g_ready_for_remote = false;
  g_signal_ready_for_remote = false;
  g_finish = false;
  g_ucontext = 0;
  g_waiters = 0;
}

static std::vector<const char*> kFunctionOrder{"OuterFunction", "MiddleFunction", "InnerFunction"};

static std::vector<const char*> kFunctionSignalOrder{"OuterFunction",        "MiddleFunction",
                                                     "InnerFunction",        "SignalOuterFunction",
                                                     "SignalMiddleFunction", "SignalInnerFunction"};

static void SignalHandler(int, siginfo_t*, void* sigcontext) {
  g_ucontext = reinterpret_cast<uintptr_t>(sigcontext);
  while (!g_finish) {
  }
}

extern "C" void SignalInnerFunction() {
  g_signal_ready_for_remote = true;
  // Avoid any function calls because not every instruction will be
  // unwindable.
  // This method of looping is only used when testing a remote unwind.
  while (true) {
  }
}

extern "C" void SignalMiddleFunction() {
  SignalInnerFunction();
}

extern "C" void SignalOuterFunction() {
  SignalMiddleFunction();
}

static void SignalCallerHandler(int, siginfo_t*, void*) {
  SignalOuterFunction();
}

static std::string ErrorMsg(const std::vector<const char*>& function_names, Unwinder* unwinder) {
  std::string unwind;
  for (size_t i = 0; i < unwinder->NumFrames(); i++) {
    unwind += unwinder->FormatFrame(i) + '\n';
  }

  return std::string(
             "Unwind completed without finding all frames\n"
             "  Unwinder error: ") +
         unwinder->LastErrorCodeString() + "\n" +
         "  Looking for function: " + function_names.front() + "\n" + "Unwind data:\n" + unwind;
}

static void VerifyUnwindFrames(Unwinder* unwinder,
                               std::vector<const char*> expected_function_names) {
  for (auto& frame : unwinder->frames()) {
    if (frame.function_name == expected_function_names.back()) {
      expected_function_names.pop_back();
      if (expected_function_names.empty()) {
        break;
      }
    }
  }

  ASSERT_TRUE(expected_function_names.empty()) << ErrorMsg(expected_function_names, unwinder);

  // Verify that the load bias of every map with a MapInfo is has been initialized.
  for (auto& frame : unwinder->frames()) {
    if (frame.map_info == nullptr) {
      continue;
    }
    ASSERT_NE(UINT64_MAX, frame.map_info->GetLoadBias()) << "Frame " << frame.num << " failed";
  }
}

static void VerifyUnwind(Unwinder* unwinder, std::vector<const char*> expected_function_names) {
  unwinder->Unwind();

  VerifyUnwindFrames(unwinder, expected_function_names);
}

static void VerifyUnwind(pid_t pid, Maps* maps, Regs* regs,
                         std::vector<const char*> expected_function_names) {
  auto process_memory(Memory::CreateProcessMemory(pid));

  Unwinder unwinder(512, maps, regs, process_memory);
  VerifyUnwind(&unwinder, expected_function_names);
}

// This test assumes that this code is compiled with optimizations turned
// off. If this doesn't happen, then all of the calls will be optimized
// away.
extern "C" void InnerFunction(TestTypeEnum test_type) {
  // Use a switch statement to force the compiler to create unwinding information
  // for each case.
  switch (test_type) {
    case TEST_TYPE_LOCAL_WAIT_FOR_FINISH: {
      g_waiters++;
      while (!g_finish) {
      }
      break;
    }

    case TEST_TYPE_REMOTE:
    case TEST_TYPE_REMOTE_WITH_INVALID_CALL: {
      g_ready_for_remote = true;
      if (test_type == TEST_TYPE_REMOTE_WITH_INVALID_CALL) {
        void (*crash_func)() = nullptr;
        crash_func();
      }
      while (true) {
      }
      break;
    }

    default: {
      std::unique_ptr<Unwinder> unwinder;
      std::unique_ptr<Regs> regs(Regs::CreateFromLocal());
      RegsGetLocal(regs.get());
      std::unique_ptr<Maps> maps;

      if (test_type == TEST_TYPE_LOCAL_UNWINDER) {
        maps.reset(new LocalMaps());
        ASSERT_TRUE(maps->Parse());
        auto process_memory(Memory::CreateProcessMemory(getpid()));
        unwinder.reset(new Unwinder(512, maps.get(), regs.get(), process_memory));
      } else {
        UnwinderFromPid* unwinder_from_pid = new UnwinderFromPid(512, getpid());
        unwinder_from_pid->SetRegs(regs.get());
        unwinder.reset(unwinder_from_pid);
      }
      VerifyUnwind(unwinder.get(), kFunctionOrder);
      break;
    }
  }
}

extern "C" void MiddleFunction(TestTypeEnum test_type) {
  InnerFunction(test_type);
}

extern "C" void OuterFunction(TestTypeEnum test_type) {
  MiddleFunction(test_type);
}

class UnwindTest : public ::testing::Test {
 public:
  void SetUp() override { ResetGlobals(); }
};

TEST_F(UnwindTest, local) {
  OuterFunction(TEST_TYPE_LOCAL_UNWINDER);
}

TEST_F(UnwindTest, local_use_from_pid) {
  OuterFunction(TEST_TYPE_LOCAL_UNWINDER_FROM_PID);
}

static void LocalUnwind(void* data) {
  TestTypeEnum* test_type = reinterpret_cast<TestTypeEnum*>(data);
  OuterFunction(*test_type);
}

TEST_F(UnwindTest, local_check_for_leak) {
  TestTypeEnum test_type = TEST_TYPE_LOCAL_UNWINDER;
  TestCheckForLeaks(LocalUnwind, &test_type);
}

TEST_F(UnwindTest, local_use_from_pid_check_for_leak) {
  TestTypeEnum test_type = TEST_TYPE_LOCAL_UNWINDER_FROM_PID;
  TestCheckForLeaks(LocalUnwind, &test_type);
}

void WaitForRemote(pid_t pid, uint64_t addr, bool leave_attached, bool* completed) {
  *completed = false;
  // Need to sleep before attempting first ptrace. Without this, on the
  // host it becomes impossible to attach and ptrace sets errno to EPERM.
  usleep(1000);
  for (size_t i = 0; i < 4000; i++) {
    ASSERT_TRUE(TestAttach(pid));

    MemoryRemote memory(pid);
    // Read the remote value to see if we are ready.
    bool value;
    if (memory.ReadFully(addr, &value, sizeof(value)) && value) {
      *completed = true;
    }
    if (!*completed || !leave_attached) {
      ASSERT_TRUE(TestDetach(pid));
    }
    if (*completed) {
      break;
    }
    usleep(5000);
  }
}

TEST_F(UnwindTest, remote) {
  pid_t pid;
  if ((pid = fork()) == 0) {
    OuterFunction(TEST_TYPE_REMOTE);
    exit(0);
  }
  ASSERT_NE(-1, pid);
  TestScopedPidReaper reap(pid);

  bool completed;
  WaitForRemote(pid, reinterpret_cast<uint64_t>(&g_ready_for_remote), true, &completed);
  ASSERT_TRUE(completed) << "Timed out waiting for remote process to be ready.";

  RemoteMaps maps(pid);
  ASSERT_TRUE(maps.Parse());
  std::unique_ptr<Regs> regs(Regs::RemoteGet(pid));
  ASSERT_TRUE(regs.get() != nullptr);

  VerifyUnwind(pid, &maps, regs.get(), kFunctionOrder);

  ASSERT_TRUE(TestDetach(pid));
}

TEST_F(UnwindTest, unwind_from_pid_remote) {
  pid_t pid;
  if ((pid = fork()) == 0) {
    OuterFunction(TEST_TYPE_REMOTE);
    exit(0);
  }
  ASSERT_NE(-1, pid);
  TestScopedPidReaper reap(pid);

  bool completed;
  WaitForRemote(pid, reinterpret_cast<uint64_t>(&g_ready_for_remote), true, &completed);
  ASSERT_TRUE(completed) << "Timed out waiting for remote process to be ready.";

  std::unique_ptr<Regs> regs(Regs::RemoteGet(pid));
  ASSERT_TRUE(regs.get() != nullptr);

  UnwinderFromPid unwinder(512, pid);
  unwinder.SetRegs(regs.get());

  VerifyUnwind(&unwinder, kFunctionOrder);

  ASSERT_TRUE(TestDetach(pid));
}

static void RemoteCheckForLeaks(void (*unwind_func)(void*)) {
  pid_t pid;
  if ((pid = fork()) == 0) {
    OuterFunction(TEST_TYPE_REMOTE);
    exit(0);
  }
  ASSERT_NE(-1, pid);
  TestScopedPidReaper reap(pid);

  bool completed;
  WaitForRemote(pid, reinterpret_cast<uint64_t>(&g_ready_for_remote), true, &completed);
  ASSERT_TRUE(completed) << "Timed out waiting for remote process to be ready.";

  TestCheckForLeaks(unwind_func, &pid);

  ASSERT_TRUE(TestDetach(pid));
}

static void RemoteUnwind(void* data) {
  pid_t* pid = reinterpret_cast<pid_t*>(data);

  RemoteMaps maps(*pid);
  ASSERT_TRUE(maps.Parse());
  std::unique_ptr<Regs> regs(Regs::RemoteGet(*pid));
  ASSERT_TRUE(regs.get() != nullptr);

  VerifyUnwind(*pid, &maps, regs.get(), kFunctionOrder);
}

TEST_F(UnwindTest, remote_check_for_leaks) {
  RemoteCheckForLeaks(RemoteUnwind);
}

static void RemoteUnwindFromPid(void* data) {
  pid_t* pid = reinterpret_cast<pid_t*>(data);

  std::unique_ptr<Regs> regs(Regs::RemoteGet(*pid));
  ASSERT_TRUE(regs.get() != nullptr);

  UnwinderFromPid unwinder(512, *pid);
  unwinder.SetRegs(regs.get());

  VerifyUnwind(&unwinder, kFunctionOrder);
}

TEST_F(UnwindTest, remote_unwind_for_pid_check_for_leaks) {
  RemoteCheckForLeaks(RemoteUnwindFromPid);
}

TEST_F(UnwindTest, from_context) {
  std::atomic_int tid(0);
  std::thread thread([&]() {
    tid = syscall(__NR_gettid);
    OuterFunction(TEST_TYPE_LOCAL_WAIT_FOR_FINISH);
  });

  struct sigaction act, oldact;
  memset(&act, 0, sizeof(act));
  act.sa_sigaction = SignalHandler;
  act.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
  ASSERT_EQ(0, sigaction(SIGUSR1, &act, &oldact));
  // Wait for the tid to get set.
  for (size_t i = 0; i < 100; i++) {
    if (tid.load() != 0) {
      break;
    }
    usleep(1000);
  }
  ASSERT_NE(0, tid.load());
  ASSERT_EQ(0, tgkill(getpid(), tid.load(), SIGUSR1)) << "Error: " << strerror(errno);

  // Wait for context data.
  void* ucontext;
  for (size_t i = 0; i < 2000; i++) {
    ucontext = reinterpret_cast<void*>(g_ucontext.load());
    if (ucontext != nullptr) {
      break;
    }
    usleep(1000);
  }
  ASSERT_TRUE(ucontext != nullptr) << "Timed out waiting for thread to respond to signal.";

  LocalMaps maps;
  ASSERT_TRUE(maps.Parse());
  std::unique_ptr<Regs> regs(Regs::CreateFromUcontext(Regs::CurrentArch(), ucontext));

  VerifyUnwind(getpid(), &maps, regs.get(), kFunctionOrder);

  ASSERT_EQ(0, sigaction(SIGUSR1, &oldact, nullptr));

  g_finish = true;
  thread.join();
}

static void RemoteThroughSignal(int signal, unsigned int sa_flags) {
  pid_t pid;
  if ((pid = fork()) == 0) {
    struct sigaction act, oldact;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = SignalCallerHandler;
    act.sa_flags = SA_RESTART | SA_ONSTACK | sa_flags;
    ASSERT_EQ(0, sigaction(signal, &act, &oldact));

    OuterFunction(signal != SIGSEGV ? TEST_TYPE_REMOTE : TEST_TYPE_REMOTE_WITH_INVALID_CALL);
    exit(0);
  }
  ASSERT_NE(-1, pid);
  TestScopedPidReaper reap(pid);

  bool completed;
  if (signal != SIGSEGV) {
    WaitForRemote(pid, reinterpret_cast<uint64_t>(&g_ready_for_remote), false, &completed);
    ASSERT_TRUE(completed) << "Timed out waiting for remote process to be ready.";
    ASSERT_EQ(0, kill(pid, SIGUSR1));
  }
  WaitForRemote(pid, reinterpret_cast<uint64_t>(&g_signal_ready_for_remote), true, &completed);
  ASSERT_TRUE(completed) << "Timed out waiting for remote process to be in signal handler.";

  RemoteMaps maps(pid);
  ASSERT_TRUE(maps.Parse());
  std::unique_ptr<Regs> regs(Regs::RemoteGet(pid));
  ASSERT_TRUE(regs.get() != nullptr);

  VerifyUnwind(pid, &maps, regs.get(), kFunctionSignalOrder);

  ASSERT_TRUE(TestDetach(pid));
}

TEST_F(UnwindTest, remote_through_signal) {
  RemoteThroughSignal(SIGUSR1, 0);
}

TEST_F(UnwindTest, remote_through_signal_sa_siginfo) {
  RemoteThroughSignal(SIGUSR1, SA_SIGINFO);
}

TEST_F(UnwindTest, remote_through_signal_with_invalid_func) {
  RemoteThroughSignal(SIGSEGV, 0);
}

TEST_F(UnwindTest, remote_through_signal_sa_siginfo_with_invalid_func) {
  RemoteThroughSignal(SIGSEGV, SA_SIGINFO);
}

// Verify that using the same map while unwinding multiple threads at the
// same time doesn't cause problems.
TEST_F(UnwindTest, multiple_threads_unwind_same_map) {
  static constexpr size_t kNumConcurrentThreads = 100;

  LocalMaps maps;
  ASSERT_TRUE(maps.Parse());
  auto process_memory(Memory::CreateProcessMemory(getpid()));

  std::vector<std::thread*> threads;

  std::atomic_bool wait;
  wait = true;
  size_t frames[kNumConcurrentThreads];
  for (size_t i = 0; i < kNumConcurrentThreads; i++) {
    std::thread* thread = new std::thread([i, &frames, &maps, &process_memory, &wait]() {
      while (wait) {
      }
      std::unique_ptr<Regs> regs(Regs::CreateFromLocal());
      RegsGetLocal(regs.get());

      Unwinder unwinder(512, &maps, regs.get(), process_memory);
      unwinder.Unwind();
      frames[i] = unwinder.NumFrames();
      ASSERT_LE(3U, frames[i]) << "Failed for thread " << i;
    });
    threads.push_back(thread);
  }
  wait = false;
  for (auto thread : threads) {
    thread->join();
    delete thread;
  }
}

TEST_F(UnwindTest, thread_unwind) {
  ResetGlobals();

  std::atomic_int tid(0);
  std::thread thread([&tid]() {
    tid = android::base::GetThreadId();
    OuterFunction(TEST_TYPE_LOCAL_WAIT_FOR_FINISH);
  });

  while (tid.load() == 0) {
  }

  ThreadUnwinder unwinder(512);
  ASSERT_TRUE(unwinder.Init());
  unwinder.UnwindWithSignal(SIGRTMIN, tid);
  VerifyUnwindFrames(&unwinder, kFunctionOrder);

  g_finish = true;
  thread.join();
}

TEST_F(UnwindTest, thread_unwind_copy_regs) {
  ResetGlobals();

  std::atomic_int tid(0);
  std::thread thread([&tid]() {
    tid = android::base::GetThreadId();
    OuterFunction(TEST_TYPE_LOCAL_WAIT_FOR_FINISH);
  });

  while (tid.load() == 0) {
  }

  ThreadUnwinder unwinder(512);
  ASSERT_TRUE(unwinder.Init());
  std::unique_ptr<Regs> initial_regs;
  unwinder.UnwindWithSignal(SIGRTMIN, tid, &initial_regs);
  ASSERT_TRUE(initial_regs != nullptr);
  // Verify the initial registers match the first frame pc/sp.
  ASSERT_TRUE(unwinder.NumFrames() != 0);
  auto initial_frame = unwinder.frames()[0];
  ASSERT_EQ(initial_regs->pc(), initial_frame.pc);
  ASSERT_EQ(initial_regs->sp(), initial_frame.sp);
  VerifyUnwindFrames(&unwinder, kFunctionOrder);

  g_finish = true;
  thread.join();
}

TEST_F(UnwindTest, thread_unwind_with_external_maps) {
  ResetGlobals();

  std::atomic_int tid(0);
  std::thread thread([&tid]() {
    tid = android::base::GetThreadId();
    OuterFunction(TEST_TYPE_LOCAL_WAIT_FOR_FINISH);
  });

  while (tid.load() == 0) {
  }

  LocalMaps maps;
  ASSERT_TRUE(maps.Parse());

  ThreadUnwinder unwinder(512, &maps);
  ASSERT_EQ(&maps, unwinder.GetMaps());
  ASSERT_TRUE(unwinder.Init());
  ASSERT_EQ(&maps, unwinder.GetMaps());
  unwinder.UnwindWithSignal(SIGRTMIN, tid);
  VerifyUnwindFrames(&unwinder, kFunctionOrder);
  ASSERT_EQ(&maps, unwinder.GetMaps());

  g_finish = true;
  thread.join();
}

TEST_F(UnwindTest, thread_unwind_cur_pid) {
  ThreadUnwinder unwinder(512);
  ASSERT_TRUE(unwinder.Init());
  unwinder.UnwindWithSignal(SIGRTMIN, getpid());
  EXPECT_EQ(0U, unwinder.NumFrames());
  EXPECT_EQ(ERROR_UNSUPPORTED, unwinder.LastErrorCode());
}

TEST_F(UnwindTest, thread_unwind_cur_thread) {
  std::thread thread([]() {
    ThreadUnwinder unwinder(512);
    ASSERT_TRUE(unwinder.Init());
    unwinder.UnwindWithSignal(SIGRTMIN, android::base::GetThreadId());
    EXPECT_EQ(0U, unwinder.NumFrames());
    EXPECT_EQ(ERROR_UNSUPPORTED, unwinder.LastErrorCode());
  });
  thread.join();
}

TEST_F(UnwindTest, thread_unwind_cur_pid_from_thread) {
  std::thread thread([]() {
    ThreadUnwinder unwinder(512);
    ASSERT_TRUE(unwinder.Init());
    unwinder.UnwindWithSignal(SIGRTMIN, getpid());
    EXPECT_NE(0U, unwinder.NumFrames());
    EXPECT_NE(ERROR_UNSUPPORTED, unwinder.LastErrorCode());
  });
  thread.join();
}

static std::thread* CreateUnwindThread(std::atomic_int& tid, ThreadUnwinder& unwinder,
                                       std::atomic_bool& start_unwinding,
                                       std::atomic_int& unwinders) {
  return new std::thread([&tid, &unwinder, &start_unwinding, &unwinders]() {
    while (!start_unwinding.load()) {
    }

    ThreadUnwinder thread_unwinder(512, &unwinder);
    // Allow the unwind to timeout since this will be doing multiple
    // unwinds at once.
    for (size_t i = 0; i < 3; i++) {
      thread_unwinder.UnwindWithSignal(SIGRTMIN, tid);
      if (thread_unwinder.LastErrorCode() != ERROR_THREAD_TIMEOUT) {
        break;
      }
    }
    VerifyUnwindFrames(&thread_unwinder, kFunctionOrder);
    ++unwinders;
  });
}

TEST_F(UnwindTest, thread_unwind_same_thread_from_threads) {
  static constexpr size_t kNumThreads = 300;
  ResetGlobals();

  std::atomic_int tid(0);
  std::thread thread([&tid]() {
    tid = android::base::GetThreadId();
    OuterFunction(TEST_TYPE_LOCAL_WAIT_FOR_FINISH);
  });

  while (g_waiters.load() != 1) {
  }

  ThreadUnwinder unwinder(512);
  ASSERT_TRUE(unwinder.Init());

  std::atomic_bool start_unwinding(false);
  std::vector<std::thread*> threads;
  std::atomic_int unwinders(0);
  for (size_t i = 0; i < kNumThreads; i++) {
    threads.push_back(CreateUnwindThread(tid, unwinder, start_unwinding, unwinders));
  }

  start_unwinding = true;
  while (unwinders.load() != kNumThreads) {
  }

  for (auto* thread : threads) {
    thread->join();
    delete thread;
  }

  g_finish = true;
  thread.join();
}

TEST_F(UnwindTest, thread_unwind_multiple_thread_from_threads) {
  static constexpr size_t kNumThreads = 100;
  ResetGlobals();

  std::atomic_int tids[kNumThreads] = {};
  std::vector<std::thread*> threads;
  for (size_t i = 0; i < kNumThreads; i++) {
    std::thread* thread = new std::thread([&tids, i]() {
      tids[i] = android::base::GetThreadId();
      OuterFunction(TEST_TYPE_LOCAL_WAIT_FOR_FINISH);
    });
    threads.push_back(thread);
  }

  while (g_waiters.load() != kNumThreads) {
  }

  ThreadUnwinder unwinder(512);
  ASSERT_TRUE(unwinder.Init());

  std::atomic_bool start_unwinding(false);
  std::vector<std::thread*> unwinder_threads;
  std::atomic_int unwinders(0);
  for (size_t i = 0; i < kNumThreads; i++) {
    unwinder_threads.push_back(CreateUnwindThread(tids[i], unwinder, start_unwinding, unwinders));
  }

  start_unwinding = true;
  while (unwinders.load() != kNumThreads) {
  }

  for (auto* thread : unwinder_threads) {
    thread->join();
    delete thread;
  }

  g_finish = true;

  for (auto* thread : threads) {
    thread->join();
    delete thread;
  }
}

TEST_F(UnwindTest, thread_unwind_multiple_thread_from_threads_updatable_maps) {
  static constexpr size_t kNumThreads = 100;
  ResetGlobals();

  // Do this before the threads are started so that the maps needed to
  // unwind are not created yet, and this verifies the dynamic nature
  // of the LocalUpdatableMaps object.
  LocalUpdatableMaps maps;
  ASSERT_TRUE(maps.Parse());

  std::atomic_int tids[kNumThreads] = {};
  std::vector<std::thread*> threads;
  for (size_t i = 0; i < kNumThreads; i++) {
    std::thread* thread = new std::thread([&tids, i]() {
      tids[i] = android::base::GetThreadId();
      OuterFunction(TEST_TYPE_LOCAL_WAIT_FOR_FINISH);
    });
    threads.push_back(thread);
  }

  while (g_waiters.load() != kNumThreads) {
  }

  ThreadUnwinder unwinder(512, &maps);
  ASSERT_TRUE(unwinder.Init());

  std::atomic_bool start_unwinding(false);
  std::vector<std::thread*> unwinder_threads;
  std::atomic_int unwinders(0);
  for (size_t i = 0; i < kNumThreads; i++) {
    unwinder_threads.push_back(CreateUnwindThread(tids[i], unwinder, start_unwinding, unwinders));
  }

  start_unwinding = true;
  while (unwinders.load() != kNumThreads) {
  }

  for (auto* thread : unwinder_threads) {
    thread->join();
    delete thread;
  }

  g_finish = true;

  for (auto* thread : threads) {
    thread->join();
    delete thread;
  }
}

}  // namespace unwindstack
