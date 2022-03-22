// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_MAIN_THREAD_EXECUTOR_H_
#define ORBIT_BASE_MAIN_THREAD_EXECUTOR_H_

#include <absl/types/span.h>

#include <chrono>

#include "OrbitBase/Executor.h"
#include "OrbitBase/Future.h"

namespace orbit_base {

// This class implements a mechanism for landing
// actions to the main thread. As a general rule
// waiting on sockets and processing should be
// happening off the main thread and the main thread
// should only be responsible for updating user
// interface and models.
//
// Usage example:
// /* A caller who wants to process something on the main thread,
//  * note that this is not-blocking operation and will be processed
//  * at some time in the future on the main thread.
//  */
//
// manager->Schedule(CreateAction([data]{
//   UpdateSomethingWith(data);
// }));
class MainThreadExecutor : public Executor {
 public:
  enum class WaitResult { kCompleted, kTimedOut, kAborted };
  [[nodiscard]] virtual WaitResult WaitFor(const Future<void>& future,
                                           std::chrono::milliseconds timeout) = 0;

  [[nodiscard]] virtual WaitResult WaitFor(const Future<void>& future) = 0;

  [[nodiscard]] virtual WaitResult WaitForAll(absl::Span<Future<void>> futures,
                                              std::chrono::milliseconds timeout) = 0;

  [[nodiscard]] virtual WaitResult WaitForAll(absl::Span<Future<void>> futures) = 0;

  virtual void AbortWaitingJobs() = 0;
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_MAIN_THREAD_EXECUTOR_H_
