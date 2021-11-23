// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracerImpl.h"

#include "GrpcProtos/module.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "WindowsUtils/ListModules.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModulesSnapshot;
using orbit_windows_utils::Module;

namespace orbit_windows_tracing {

TracerImpl::TracerImpl(orbit_grpc_protos::CaptureOptions capture_options, TracerListener* listener)
    : capture_options_(std::move(capture_options)), listener_(listener) {}

void TracerImpl::Start() {
  CHECK(krabs_tracer_ == nullptr);
  SendModulesSnapshot();
  krabs_tracer_ = std::make_unique<KrabsTracer>(capture_options_, listener_);
  krabs_tracer_->Start();
}

void TracerImpl::Stop() {
  CHECK(krabs_tracer_ != nullptr);
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
    ERROR("Unable to load modules for %u", pid);
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
