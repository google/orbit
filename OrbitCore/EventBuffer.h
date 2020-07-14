// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_EVENT_BUFFER_H_
#define ORBIT_CORE_EVENT_BUFFER_H_

#include <set>

#include <google/protobuf/map.h>

#include "BlockChain.h"
#include "CallstackTypes.h"
#include "Core.h"
#include "SerializationMacros.h"
#include "capture.pb.h"

#ifdef __linux
#include "LinuxUtils.h"
#endif

//-----------------------------------------------------------------------------
class EventBuffer {
 public:
  EventBuffer() {
    data_ = std::make_unique<EventBufferData>();
    Reset();
  }

  void Reset() {
    data_->clear_callstack_events();
    data_->set_min_time(LLONG_MAX);
    data_->set_max_time(0);
  }

  const std::unique_ptr<EventBufferData>& GetData() const { return data_; }
  void SetData(std::unique_ptr<EventBufferData> data_ptr) {
    data_ = std::move(data_ptr);
  }

  const google::protobuf::Map<int32_t, Uint64ToCallstackEvent>& GetCallstacks()
      const {
    return data_->callstack_events();
  }

  const Uint64ToCallstackEvent& GetCallstack(int id) {
    return (*data_->mutable_callstack_events())[id];
  }
  Mutex& GetMutex() { return m_Mutex; }
  std::vector<CallstackEvent> GetCallstackEvents(uint64_t a_TimeBegin,
                                                 uint64_t a_TimeEnd,
                                                 ThreadID a_ThreadId = 0);
  uint64_t GetMaxTime() const { return data_->max_time(); }
  uint64_t GetMinTime() const { return data_->min_time(); }
  bool HasEvent() {
    ScopeLock lock(m_Mutex);
    return !data_->callstack_events().empty();
  }

#ifdef __linux__
  size_t GetNumEvents() const;
#endif

  //-----------------------------------------------------------------------------
  void RegisterTime(uint64_t a_Time) {
    if (a_Time > data_->max_time()) {
      data_->set_max_time(a_Time);
    }
    if (a_Time > 0 && a_Time < data_->min_time()) {
      data_->set_min_time(a_Time);
    }
  }

  //-----------------------------------------------------------------------------
  void AddCallstackEvent(uint64_t time, CallstackID cs_hash,
                         ThreadID thread_id);

 private:
  Mutex m_Mutex;
  std::unique_ptr<EventBufferData> data_;
};

#endif  // ORBIT_CORE_EVENT_BUFFER_H_
