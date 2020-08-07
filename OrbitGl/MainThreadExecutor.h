// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_MAIN_THREAD_EXECUTOR_H_
#define ORBIT_BASE_MAIN_THREAD_EXECUTOR_H_

#include <memory>
#include <thread>

#include "OrbitBase/Action.h"

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
class MainThreadExecutor {
 public:
  MainThreadExecutor() = default;
  virtual ~MainThreadExecutor() = default;

  // Schedules the action to be performed on the main thread.
  virtual void Schedule(std::unique_ptr<Action> action) = 0;

  template <typename F>
  void Schedule(F&& functor) {
    Schedule(CreateAction(std::forward<F>(functor)));
  }
};

#endif  // ORBIT_BASE_MAIN_THREAD_EXECUTOR_H_
