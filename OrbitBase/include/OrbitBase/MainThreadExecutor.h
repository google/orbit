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
//
// To consume events the main thread should periodically call
// manager->ConsumeActions();
//
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

  // Called from the main thread to perform scheduled actions.
  // The method checks that it is called from the thread specified
  // in Create method and fails otherwise.
  virtual void ConsumeActions() = 0;

  // Create executor.
  // thread_id is id of the consumer thread. To ensure all
  // actions are executed on this thread.
  static std::unique_ptr<MainThreadExecutor> Create(
      std::thread::id thread_id = std::this_thread::get_id());
};

#endif  // ORBIT_BASE_MAIN_THREAD_EXECUTOR_H_
