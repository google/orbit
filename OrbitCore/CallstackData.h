// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CALLSTACK_DATA_H_
#define ORBIT_CORE_CALLSTACK_DATA_H_

#include <memory>

#include "BlockChain.h"
#include "Callstack.h"
#include "CallstackTypes.h"
#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "capture_data.pb.h"

class CallstackData {
 public:
  explicit CallstackData() = default;

  CallstackData(const CallstackData& other) = delete;
  CallstackData& operator=(const CallstackData& other) = delete;
  CallstackData(CallstackData&& other) = delete;
  CallstackData& operator=(CallstackData&& other) = delete;

  ~CallstackData() = default;

  // Assume that callstack_event.callstack_hash is filled correctly and the
  // CallStack with corresponding hash is already in unique_callstacks_
  void AddCallstackEvent(orbit_client_protos::CallstackEvent callstack_event);
  void AddUniqueCallStack(CallStack call_stack);
  void AddCallStackFromKnownCallstackData(const orbit_client_protos::CallstackEvent& event,
                                          const CallstackData* known_callstack_data);

  [[nodiscard]] const BlockChain<orbit_client_protos::CallstackEvent, 16 * 1024>& callstack_events()
      const {
    return callstack_events_;
  };

  [[nodiscard]] uint32_t GetCallstackEventsSize() const { return callstack_events_.size(); };

  [[nodiscard]] const CallStack* GetCallStack(CallstackID callstack_id) const {
    absl::MutexLock lock(&unique_callstacks_mutex_);
    auto it = unique_callstacks_.find(callstack_id);
    if (it != unique_callstacks_.end()) {
      return it->second.get();
    }
    return nullptr;
  };

  [[nodiscard]] bool HasCallStack(CallstackID callstack_id) const {
    absl::MutexLock lock(&unique_callstacks_mutex_);
    return unique_callstacks_.count(callstack_id) > 0;
  }

  void ForEachUniqueCallstack(const std::function<void(const CallStack&)>& action) const {
    absl::MutexLock lock(&unique_callstacks_mutex_);
    for (const auto& it : unique_callstacks_) {
      action(*it.second);
    }
  }

  void ForEachFrameInCallstack(uint64_t callstack_id,
                               const std::function<void(uint64_t)>& action) const {
    absl::MutexLock lock(&unique_callstacks_mutex_);
    for (uint64_t frame : unique_callstacks_.at(callstack_id)->GetFrames()) {
      action(frame);
    }
  }

  [[nodiscard]] absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>>
  GetUniqueCallstacksCopy() const {
    absl::MutexLock lock(&unique_callstacks_mutex_);
    return unique_callstacks_;
  }

 private:
  [[nodiscard]] std::shared_ptr<CallStack> GetCallstackPtr(CallstackID callstack_id) const {
    absl::MutexLock lock(&unique_callstacks_mutex_);
    auto it = unique_callstacks_.find(callstack_id);
    if (it != unique_callstacks_.end()) {
      return unique_callstacks_.at(callstack_id);
    }
    return nullptr;
  };

 private:
  BlockChain<orbit_client_protos::CallstackEvent, 16 * 1024> callstack_events_;

  mutable absl::Mutex unique_callstacks_mutex_;
  absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> unique_callstacks_;
};

#endif  // ORBIT_CORE_CALLSTACK_DATA_H_
