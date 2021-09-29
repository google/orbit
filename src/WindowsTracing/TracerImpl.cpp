// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracerImpl.h"

#include <OrbitLib/OrbitLib.h>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "WindowsUtils/ListModules.h"
#include "WindowsUtils/ListThreads.h"
#include "capture.pb.h"

using orbit_windows_utils::Module;
using orbit_windows_utils::Thread;

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModulesSnapshot;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadNamesSnapshot;

namespace orbit_windows_tracing {

TracerImpl::TracerImpl(orbit_grpc_protos::CaptureOptions capture_options, TracerListener* listener)
    : capture_options_(std::move(capture_options)), listener_(listener) {}

void TracerImpl::Start() {
  SendThreadSnapshot();
  SendModuleSnapshot();

  CHECK(krabs_tracer_ == nullptr);
  krabs_tracer_ = std::make_unique<KrabsTracer>(capture_options_, listener_);
  krabs_tracer_->Start();
  CHECK(dynamic_instrumentation_manager_ == nullptr);
  dynamic_instrumentation_manager_ = std::make_unique<DynamicInstrumentationManager>();
  // dynamic_instrumentation_manager_->Start(capture_options_, listener_);
}

void TracerImpl::Stop() {
  CHECK(dynamic_instrumentation_manager_ != nullptr);
  // dynamic_instrumentation_manager_->Stop();
  dynamic_instrumentation_manager_ = nullptr;

  CHECK(krabs_tracer_ != nullptr);
  krabs_tracer_->Stop();
  krabs_tracer_ = nullptr;

  SendThreadSnapshot();
  SendModuleSnapshot();
}

void TracerImpl::SendThreadSnapshot() const {
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  std::vector<Thread> threads = orbit_windows_utils::ListAllThreads();
  if (threads.empty()) {
    ERROR("Unable to send thread snapshot");
    return;
  }

  ThreadNamesSnapshot thread_names_snapshot;
  thread_names_snapshot.set_timestamp_ns(timestamp_ns);
  for (const Thread& thread : threads) {
    ThreadName* thread_name = thread_names_snapshot.add_thread_names();
    thread_name->set_tid(thread.tid);
    thread_name->set_pid(thread.pid);
    thread_name->set_name(thread.name);
    thread_name->set_timestamp_ns(timestamp_ns);
  }

  listener_->OnThreadNamesSnapshot(std::move(thread_names_snapshot));
}

void TracerImpl::SendModuleSnapshot() const {
  const uint32_t pid = capture_options_.pid();
  std::vector<Module> modules = orbit_windows_utils::ListModules(pid);

  ModulesSnapshot modules_snapshot;
  modules_snapshot.set_pid(pid);
  modules_snapshot.set_timestamp_ns(orbit_base::CaptureTimestampNs());
  if (modules.empty()) {
    ERROR("Unable to send module snapshot for process %d", pid);
    return;
  }

  for (const Module& module : modules) {
    ModuleInfo* module_info = modules_snapshot.add_modules();
    module_info->set_name(module.name);
    module_info->set_file_path(module.full_path);
    module_info->set_address_start(module.address_start);
    module_info->set_address_end(module.address_end);
    module_info->set_build_id(module.build_id);
  }

  listener_->OnModulesSnapshot(std::move(modules_snapshot));
}

}  // namespace orbit_windows_tracing
