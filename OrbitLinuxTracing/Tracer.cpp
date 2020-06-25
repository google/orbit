// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <OrbitBase/Logging.h>
#include <OrbitLinuxTracing/Tracer.h>

#include "TracerThread.h"

namespace LinuxTracing {

void Tracer::Run(const CaptureOptions& capture_options,
                 TracerListener* listener,
                 const std::shared_ptr<std::atomic<bool>>& exit_requested) {
  TracerThread session{capture_options};
  session.SetListener(listener);
  session.Run(exit_requested);
}

}  // namespace LinuxTracing
