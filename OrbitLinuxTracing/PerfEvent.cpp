// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEvent.h"

#include "PerfEventVisitor.h"

namespace LinuxTracing {

// These cannot be implemented in the header PerfEvent.h, because there
// PerfEventVisitor needs to be an incomplete type to avoid the circular
// dependency between PerfEvent.h and PerfEventVisitor.h.

void ForkPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void ExitPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void ContextSwitchPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void SystemWideContextSwitchPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void StackSamplePerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void CallchainSamplePerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void UprobesPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void UretprobesPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void LostPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void MapsPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void TaskNewtaskPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void TaskRenamePerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void AmdgpuCsIoctlPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void AmdgpuSchedRunJobPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void DmaFenceSignaledPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->visit(this); }

void SchedSwitchPerfEvent::Accept(PerfEventVisitor*) {}

}  // namespace LinuxTracing
