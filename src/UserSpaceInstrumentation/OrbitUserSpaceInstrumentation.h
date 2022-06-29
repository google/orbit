// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_USER_SPACE_INSTRUMENTATION_H_
#define ORBIT_USER_SPACE_INSTRUMENTATION_H_

#include <sys/types.h>

#include <cstdint>

// Needs to be called when a capture starts. `capture_start_timestamp_ns` should be a current
// timestamp as obtained from orbit_base::CaptureTimestampNs.
extern "C" void StartNewCapture(uint64_t capture_start_timestamp_ns);

// InitializeInstrumentation needs to be called once after this library is injected into the target
// process. It sets up the communication to OrbitService.
extern "C" void InitializeInstrumentation();

// We'll spawn six threads when injecting this library. This happens immediatly after the call to
// InitializeInstrumentation above. These threads facilitate the grpc communication with
// OrbitService. OrbitService will detect the threads and call SetOrbitThreads to set the thread ids
// such that events from these threads can be ignored in EntryPayload.
extern "C" void SetOrbitThreads(pid_t tid_0, pid_t tid_1, pid_t tid_2, pid_t tid_3, pid_t tid_4,
                                pid_t tid_5);

// Payload called on entry of an instrumented function. Needs to record the return address of the
// function (in order to have it available in `ExitPayload`) and the stack pointer (i.e., the
// address of the return address). `function_id` is the id of the instrumented function. Also needs
// to overwrite the return address stored at `stack_pointer` with the `return_trampoline_address`.
extern "C" void EntryPayload(uint64_t return_address, uint64_t function_id, uint64_t stack_pointer,
                             uint64_t return_trampoline_address);

// Payload called on exit of an instrumented function. Needs to return the actual return address of
// the function such that the execution can be continued there.
extern "C" uint64_t ExitPayload();

#endif  // ORBIT_USER_SPACE_INSTRUMENTATION_H_