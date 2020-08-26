// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CallstackData.h"

#include "Callstack.h"

using orbit_client_protos::CallstackEvent;

void CallstackData::AddCallstackEvent(CallstackEvent callstack_event) {
  CallstackID hash = callstack_event.callstack_hash();
  CHECK(HasCallStack(hash));
  callstack_events_.push_back(std::move(callstack_event));
}

void CallstackData::AddUniqueCallStack(CallStack call_stack) {
  absl::MutexLock lock(&unique_callstacks_mutex_);
  CallstackID hash = call_stack.GetHash();
  unique_callstacks_[hash] = std::make_shared<CallStack>(std::move(call_stack));
}

void CallstackData::AddCallStackFromKnownCallstackData(
    const orbit_client_protos::CallstackEvent& event, const CallstackData* known_callstack_data) {
  uint64_t hash = event.callstack_hash();
  std::shared_ptr<CallStack> unique_callstack = known_callstack_data->GetCallstackPtr(hash);
  if (unique_callstack == nullptr) {
    return;
  }

  if (!HasCallStack(hash)) {
    absl::MutexLock lock(&unique_callstacks_mutex_);
    unique_callstacks_[hash] = std::move(unique_callstack);
  }
  callstack_events_.push_back(CallstackEvent(event));
}
