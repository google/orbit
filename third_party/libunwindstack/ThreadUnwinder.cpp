/*
 * Copyright (C) 2020 The Android Open Source Project
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
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <android-base/errno_restorer.h>
#include <android-base/threads.h>

#include <unwindstack/Log.h>
#include <unwindstack/Regs.h>
#include <unwindstack/Unwinder.h>

#include "ThreadEntry.h"

namespace unwindstack {

static void SignalLogOnly(int, siginfo_t*, void*) {
  android::base::ErrnoRestorer restore;

  Log::AsyncSafe("pid %d, tid %d: Received a spurious thread signal\n", getpid(),
                 static_cast<int>(android::base::GetThreadId()));
}

static void SignalHandler(int, siginfo_t*, void* sigcontext) {
  android::base::ErrnoRestorer restore;

  ThreadEntry* entry = ThreadEntry::Get(android::base::GetThreadId(), false);
  if (!entry) {
    return;
  }

  entry->CopyUcontextFromSigcontext(sigcontext);

  // Indicate the ucontext is now valid.
  entry->Wake();
  // Pause the thread until the unwind is complete. This avoids having
  // the thread run ahead causing problems.
  // The number indicates that we are waiting for the second Wake() call
  // overall which is made by the thread requesting an unwind.
  if (entry->Wait(WAIT_FOR_UNWIND_TO_COMPLETE)) {
    // Do not remove the entry here because that can result in a deadlock
    // if the code cannot properly send a signal to the thread under test.
    entry->Wake();
  } else {
    // At this point, it is possible that entry has been freed, so just exit.
    Log::AsyncSafe("Timed out waiting for unwind thread to indicate it completed.");
  }
}

ThreadUnwinder::ThreadUnwinder(size_t max_frames, Maps* maps)
    : UnwinderFromPid(max_frames, getpid(), Regs::CurrentArch(), maps) {}

ThreadUnwinder::ThreadUnwinder(size_t max_frames, const ThreadUnwinder* unwinder)
    : UnwinderFromPid(max_frames, getpid(), Regs::CurrentArch()) {
  process_memory_ = unwinder->process_memory_;
  maps_ = unwinder->maps_;
  jit_debug_ = unwinder->jit_debug_;
  dex_files_ = unwinder->dex_files_;
  initted_ = unwinder->initted_;
}

ThreadEntry* ThreadUnwinder::SendSignalToThread(int signal, pid_t tid) {
  static std::mutex action_mutex;
  std::lock_guard<std::mutex> guard(action_mutex);

  ThreadEntry* entry = ThreadEntry::Get(tid);
  entry->Lock();
  struct sigaction new_action = {.sa_sigaction = SignalHandler,
                                 .sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK};
  struct sigaction old_action = {};
  sigemptyset(&new_action.sa_mask);
  if (sigaction(signal, &new_action, &old_action) != 0) {
    Log::AsyncSafe("sigaction failed: %s", strerror(errno));
    ThreadEntry::Remove(entry);
    last_error_.code = ERROR_SYSTEM_CALL;
    return nullptr;
  }

  if (tgkill(getpid(), tid, signal) != 0) {
    // Do not emit an error message, this might be expected. Set the
    // error and let the caller decide.
    if (errno == ESRCH) {
      last_error_.code = ERROR_THREAD_DOES_NOT_EXIST;
    } else {
      last_error_.code = ERROR_SYSTEM_CALL;
    }

    sigaction(signal, &old_action, nullptr);
    ThreadEntry::Remove(entry);
    return nullptr;
  }

  // Wait for the thread to get the ucontext. The number indicates
  // that we are waiting for the first Wake() call made by the thread.
  bool wait_completed = entry->Wait(WAIT_FOR_UCONTEXT);
  if (wait_completed) {
    return entry;
  }

  if (old_action.sa_sigaction == nullptr) {
    // If the wait failed, it could be that the signal could not be delivered
    // within the timeout. Add a signal handler that's simply going to log
    // something so that we don't crash if the signal eventually gets
    // delivered. Only do this if there isn't already an action set up.
    struct sigaction log_action = {.sa_sigaction = SignalLogOnly,
                                   .sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK};
    sigemptyset(&log_action.sa_mask);
    sigaction(signal, &log_action, nullptr);
  } else {
    sigaction(signal, &old_action, nullptr);
  }

  // Check to see if the thread has disappeared.
  if (tgkill(getpid(), tid, 0) == -1 && errno == ESRCH) {
    last_error_.code = ERROR_THREAD_DOES_NOT_EXIST;
  } else {
    last_error_.code = ERROR_THREAD_TIMEOUT;
    Log::AsyncSafe("Timed out waiting for signal handler to get ucontext data.");
  }

  ThreadEntry::Remove(entry);

  return nullptr;
}

void ThreadUnwinder::UnwindWithSignal(int signal, pid_t tid,
                                      const std::vector<std::string>* initial_map_names_to_skip,
                                      const std::vector<std::string>* map_suffixes_to_ignore) {
  ClearErrors();
  if (tid == pid_) {
    last_error_.code = ERROR_UNSUPPORTED;
    return;
  }

  if (!Init()) {
    return;
  }

  ThreadEntry* entry = SendSignalToThread(signal, tid);
  if (entry == nullptr) {
    return;
  }

  std::unique_ptr<Regs> regs(Regs::CreateFromUcontext(Regs::CurrentArch(), entry->GetUcontext()));
  SetRegs(regs.get());
  UnwinderFromPid::Unwind(initial_map_names_to_skip, map_suffixes_to_ignore);

  // Tell the signal handler to exit and release the entry.
  entry->Wake();

  // Wait for the thread to indicate it is done with the ThreadEntry.
  if (!entry->Wait(WAIT_FOR_THREAD_TO_RESTART)) {
    // Send a warning, but do not mark as a failure to unwind.
    Log::AsyncSafe("Timed out waiting for signal handler to indicate it finished.");
  }

  ThreadEntry::Remove(entry);
}

}  // namespace unwindstack
