// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_THREAD_POOL_H_
#define ORBIT_BASE_THREAD_POOL_H_

#include <absl/time/time.h>
#include <stddef.h>

#include <memory>
#include <utility>

#include "OrbitBase/Executor.h"

namespace orbit_base {

// This class implements a thread pool. ThreadPool allows to execute
// actions in another thread without the need to manage creation and destruction
// of that thread.
//
// Usage example:
//
// /* Schedule an action on the thread-pool */
// thread_poool->Schedule(CreateAction([data]{
//   DoSomethingWith(data);
// }));
//
// /* Shutdown thread-pool, blocks until all currently executed actions are
//    complete and stops all worker threads */
//
// thread_pool->Shutdown();
// thread_pool->Wait();
//
class ThreadPool : public orbit_base::Executor {
 public:
  // Initiates shutdown, any Schedule after this call will fail.
  virtual void Shutdown() = 0;

  // Wait until all tasks are complete. This should be called
  // before the object is destroyed, usually after calling
  // Shutdown().
  virtual void Wait() = 0;

  void ShutdownAndWait() {
    Shutdown();
    Wait();
  }

  [[nodiscard]] virtual size_t GetPoolSize() = 0;
  [[nodiscard]] virtual size_t GetNumberOfBusyThreads() = 0;

  // Create ThreadPool with specified minimum and maximum number of worker
  // threads.
  //
  // Whenever an action is Scheduled the thread pool puts it in an internal
  // queue. Worker threads pick actions from the queue and execute them.
  // If at the time of scheduling new action there are no idle worker threads,
  // the thread pool creates a new worker thread if current number of worker
  // threads is less than maximum pool size.
  //
  // If queue is empty thread_pool reduces number of worker threads
  // until they hit thread_pool_min_size. thread_ttl specifies the duration
  // for the such a thread to remain in idle state before it finishes the
  // execution.
  //
  // The run_action parameter allows to specify a function that will run an
  // Action in a different way than simply calling Action::Execute. This can for
  // example be used to run some operation before and/or after the original
  // Action.
  [[nodiscard]] static std::shared_ptr<ThreadPool> Create(
      size_t thread_pool_min_size, size_t thread_pool_max_size, absl::Duration thread_ttl,
      std::function<void(const std::unique_ptr<Action>&)> run_action = nullptr);

  // Globally accessible ThreadPool with a fixed number of threads that equal to or slightly smaller
  // than the number of available cores on the system.
  [[nodiscard]] static std::shared_ptr<ThreadPool> GetGlobalWorkerThreadPool();
};

}  // namespace orbit_base

#endif  // ORBIT_BASE_THREAD_POOL_H_
