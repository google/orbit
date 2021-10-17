/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <ucontext.h>

#include <chrono>

#include <unwindstack/Log.h>

#include "ThreadEntry.h"

namespace unwindstack {

std::mutex ThreadEntry::entries_mutex_;
std::map<pid_t, ThreadEntry*> ThreadEntry::entries_;

// Assumes that ThreadEntry::entries_mutex_ has already been locked before
// creating a ThreadEntry object.
ThreadEntry::ThreadEntry(pid_t tid) : tid_(tid), ref_count_(1), wait_value_(0) {
  // Add ourselves to the global list.
  entries_[tid_] = this;
}

ThreadEntry* ThreadEntry::Get(pid_t tid, bool create) {
  ThreadEntry* entry = nullptr;

  std::lock_guard<std::mutex> guard(entries_mutex_);
  auto iter = entries_.find(tid);
  if (iter == entries_.end()) {
    if (create) {
      entry = new ThreadEntry(tid);
    }
  } else {
    entry = iter->second;
    entry->ref_count_++;
  }

  return entry;
}

void ThreadEntry::Remove(ThreadEntry* entry) {
  entry->Unlock();

  std::lock_guard<std::mutex> guard(entries_mutex_);
  if (--entry->ref_count_ == 0) {
    delete entry;
  }
}

// Assumes that ThreadEntry::entries_mutex_ has already been locked before
// deleting a ThreadEntry object.
ThreadEntry::~ThreadEntry() {
  auto iter = entries_.find(tid_);
  if (iter != entries_.end()) {
    entries_.erase(iter);
  }
}

bool ThreadEntry::Wait(WaitType type) {
  static const std::chrono::duration wait_time(std::chrono::seconds(10));
  std::unique_lock<std::mutex> lock(wait_mutex_);
  if (wait_cond_.wait_for(lock, wait_time, [this, type] { return wait_value_ == type; })) {
    return true;
  } else {
    Log::AsyncSafe("pthread_cond_timedwait for value %d failed", type);
    return false;
  }
}

void ThreadEntry::Wake() {
  wait_mutex_.lock();
  wait_value_++;
  wait_mutex_.unlock();

  wait_cond_.notify_one();
}

void ThreadEntry::CopyUcontextFromSigcontext(void* sigcontext) {
  ucontext_t* ucontext = reinterpret_cast<ucontext_t*>(sigcontext);
  // The only thing the unwinder cares about is the mcontext data.
  memcpy(&ucontext_.uc_mcontext, &ucontext->uc_mcontext, sizeof(ucontext->uc_mcontext));
}

}  // namespace unwindstack
