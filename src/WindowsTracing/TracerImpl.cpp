// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracerImpl.h"

#include "GrpcProtos/module.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "WindowsTracing/ListModulesETW.h"
#include "WindowsUtils/ListModules.h"
#include "WindowsUtils/ListThreads.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModulesSnapshot;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadNamesSnapshot;
using orbit_windows_utils::Module;
using orbit_windows_utils::Thread;

namespace orbit_windows_tracing {

TracerImpl::TracerImpl(orbit_grpc_protos::CaptureOptions capture_options, TracerListener* listener)
    : capture_options_(std::move(capture_options)), listener_(listener) {}

void TracerImpl::Start() {
  ORBIT_CHECK(krabs_tracer_ == nullptr);
  SendModulesSnapshot();
  SendThreadNamesSnapshot();
  krabs_tracer_ = std::make_unique<KrabsTracer>(capture_options_.pid(),
                                                capture_options_.samples_per_second(), listener_);
  krabs_tracer_->Start();
}

void TracerImpl::Stop() {
  ORBIT_CHECK(krabs_tracer_ != nullptr);
  krabs_tracer_->Stop();
  krabs_tracer_ = nullptr;
}

void TracerImpl::SendModulesSnapshot() {
  uint32_t pid = capture_options_.pid();
  ModulesSnapshot modules_snapshot;
  modules_snapshot.set_pid(pid);
  modules_snapshot.set_timestamp_ns(orbit_base::CaptureTimestampNs());

  std::vector<Module> modules = orbit_windows_utils::ListModules(pid);
  if (modules.empty()) {
    // Fallback on etw module enumeration which involves more work.
    modules = ListModulesEtw(pid);
  }

  if (modules.empty()) {
    ORBIT_ERROR("Unable to load modules for %u", pid);
    return;
  }

  for (const Module& module : modules) {
    ModuleInfo* module_info = modules_snapshot.add_modules();
    module_info->set_name(module.name);
    module_info->set_file_path(module.full_path);
    module_info->set_address_start(module.address_start);
    module_info->set_address_end(module.address_end);
    module_info->set_build_id(module.build_id);
    module_info->set_load_bias(module.load_bias);
    module_info->set_object_file_type(ModuleInfo::kCoffFile);
    for (const ModuleInfo::ObjectSegment& section : module.sections) {
      *module_info->add_object_segments() = section;
    }
  }

  listener_->OnModulesSnapshot(std::move(modules_snapshot));
}

void TracerImpl::SendThreadNamesSnapshot() {
  uint64_t timestamp = orbit_base::CaptureTimestampNs();
  ThreadNamesSnapshot thread_names_snapshot;
  thread_names_snapshot.set_timestamp_ns(timestamp);

  std::vector<Thread> threads = orbit_windows_utils::ListAllThreads();
  if (threads.empty()) {
    ORBIT_ERROR("Unable to list threads");
    return;
  }

  for (const Thread& thread : threads) {
    ThreadName* thread_name = thread_names_snapshot.add_thread_names();
    thread_name->set_pid(thread.pid);
    thread_name->set_tid(thread.tid);
    thread_name->set_name(thread.name);
    thread_name->set_timestamp_ns(timestamp);
  }

  listener_->OnThreadNamesSnapshot(std::move(thread_names_snapshot));
}

}  // namespace orbit_windows_tracing
