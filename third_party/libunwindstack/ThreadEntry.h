/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef _LIBUNWINDSTACK_THREAD_ENTRY_H
#define _LIBUNWINDSTACK_THREAD_ENTRY_H

#include <pthread.h>
#include <sys/types.h>
#include <ucontext.h>

#include <condition_variable>
#include <map>
#include <mutex>

namespace unwindstack {

enum WaitType : int {
  WAIT_FOR_UCONTEXT = 1,
  WAIT_FOR_UNWIND_TO_COMPLETE,
  WAIT_FOR_THREAD_TO_RESTART,
};

class ThreadEntry {
 public:
  static ThreadEntry* Get(pid_t tid, bool create = true);

  static void Remove(ThreadEntry* entry);

  void Wake();

  bool Wait(WaitType type);

  void CopyUcontextFromSigcontext(void* sigcontext);

  inline void Lock() {
    mutex_.lock();

    // Always reset the wait value since this could be the first or nth
    // time this entry is locked.
    wait_value_ = 0;
  }

  inline void Unlock() { mutex_.unlock(); }

  inline ucontext_t* GetUcontext() { return &ucontext_; }

 private:
  ThreadEntry(pid_t tid);
  ~ThreadEntry();

  pid_t tid_;
  int ref_count_;
  std::mutex mutex_;
  std::mutex wait_mutex_;
  std::condition_variable wait_cond_;
  int wait_value_;
  ucontext_t ucontext_;

  static std::mutex entries_mutex_;
  static std::map<pid_t, ThreadEntry*> entries_;
};

}  // namespace unwindstack

#endif  // _LIBUNWINDSTACK_THREAD_ENTRY_H
