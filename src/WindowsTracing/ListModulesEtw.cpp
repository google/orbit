// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsTracing/ListModulesEtw.h"

#include "Introspection/Introspection.h"
#include "KrabsTracer.h"

namespace orbit_windows_tracing {

[[nodiscard]] std::vector<orbit_windows_utils::Module> ListModulesEtw(uint32_t pid) {
  ORBIT_SCOPE_FUNCTION;
  KrabsTracer krabs_tracer(pid, /*sampling_frequency_hz=*/0, /*listener=*/nullptr,
                           KrabsTracer::ProviderFlags::kImageLoad);
  krabs_tracer.Start();
  krabs_tracer.Stop();
  return krabs_tracer.GetLoadedModules();
}

}  // namespace orbit_windows_tracing
