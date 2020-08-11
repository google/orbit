// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_TRACER_H_
#define ORBIT_LINUX_TRACING_TRACER_H_

#include <OrbitLinuxTracing/TracerListener.h>
#include <unistd.h>

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

#include "capture.pb.h"

namespace LinuxTracing {

class Tracer {
 public:
  explicit Tracer(orbit_grpc_protos::CaptureOptions capture_options)
      : capture_options_{std::move(capture_options)} {}

  ~Tracer() { Stop(); }

  Tracer(const Tracer&) = delete;
  Tracer& operator=(const Tracer&) = delete;
  Tracer(Tracer&&) = default;
  Tracer& operator=(Tracer&&) = default;

  void SetListener(TracerListener* listener) { listener_ = listener; }

  void Start() {
    *exit_requested_ = false;
    thread_ = std::make_shared<std::thread>(&Tracer::Run, capture_options_,
                                            listener_, exit_requested_);
  }

  bool IsTracing() { return thread_ != nullptr && thread_->joinable(); }

  void Stop() {
    *exit_requested_ = true;
    if (thread_ != nullptr && thread_->joinable()) {
      thread_->join();
    }
    thread_.reset();
  }

 private:
  orbit_grpc_protos::CaptureOptions capture_options_;

  TracerListener* listener_ = nullptr;

  // exit_requested_ must outlive this object because it is used by thread_.
  // The control block of shared_ptr is thread safe (i.e., reference counting
  // and pointee's lifetime management are atomic and thread safe).
  std::shared_ptr<std::atomic<bool>> exit_requested_ =
      std::make_unique<std::atomic<bool>>(true);
  std::shared_ptr<std::thread> thread_;

  static void Run(const orbit_grpc_protos::CaptureOptions& capture_options,
                  TracerListener* listener,
                  const std::shared_ptr<std::atomic<bool>>& exit_requested);
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_TRACER_H_
