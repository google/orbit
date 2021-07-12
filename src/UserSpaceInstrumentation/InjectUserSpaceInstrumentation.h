// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_LIB_H_
#define USER_SPACE_INSTRUMENTATION_LIB_H_

#include <cstdint>

// Payload called on entry of an instrumented function. Needs to record the return address of the
// function (in order to have it available in `ExitPayload`). `function_id` is the id of the
// instrumented function.
extern "C" void EntryPayload(uint64_t return_address, uint64_t function_id);

// Payload called on exit of an instrumented function. Needs to return the actual return address of
// the function such that the execution can be continued there.
extern "C" uint64_t ExitPayload();

#endif  // USER_SPACE_INSTRUMENTATION_LIB_H_