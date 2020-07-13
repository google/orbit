// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_SAMPLING_PROFILER_H_
#define ORBIT_CORE_SAMPLING_PROFILER_H_

#include "BlockChain.h"
#include "Capture.h"
#include "Core.h"
#include "EventBuffer.h"
#include "Pdb.h"
#include "SerializationMacros.h"
#include "capture.pb.h"

class Process;

//-----------------------------------------------------------------------------
struct LineInfo {
  LineInfo() = default;

  std::string m_File;
  uint32_t m_Line = 0;
  uint64_t m_Address = 0;
  uint64_t m_FileNameHash = 0;
};

//-----------------------------------------------------------------------------
struct CallstackCount {
  CallstackCount() = default;

  int m_Count = 0;
  CallstackID m_CallstackId = 0;
};

//-----------------------------------------------------------------------------
struct SortedCallstackReport {
  SortedCallstackReport() = default;
  int m_NumCallStacksTotal = 0;
  std::vector<CallstackCount> m_CallStacks;
};

//-----------------------------------------------------------------------------
class SamplingProfiler {
 public:
  explicit SamplingProfiler(const std::shared_ptr<Process>& a_Process);
  explicit SamplingProfiler(std::unique_ptr<SamplingProfilerData> data_ptr);
  SamplingProfiler();

  void StartCapture();
  void StopCapture();
  int GetNumSamples() const { return data->num_samples(); }
  float GetSampleTime();
  float GetSampleTimeTotal() const { return m_SampleTimeSeconds; }

  void AddCallStack(Callstack& a_CallStack);
  void AddHashedCallStack(CallstackEvent& a_CallStack);
  void AddUniqueCallStack(Callstack& a_CallStack);

  std::shared_ptr<Callstack> GetCallStack(CallstackID a_ID) {
    ScopeLock lock(m_Mutex);
    return std::make_shared<Callstack>(
        (*data->mutable_unique_callstacks())[a_ID]);
  }

  bool HasCallStack(CallstackID a_ID) {
    ScopeLock lock(m_Mutex);
    return data->unique_callstacks().contains(a_ID);
  }

  std::multimap<int, CallstackID> GetCallstacksFromAddress(
      uint64_t a_Addr, ThreadID a_TID, int* o_NumCallstacks);
  std::shared_ptr<SortedCallstackReport> GetSortedCallstacksFromAddress(
      uint64_t a_Addr, ThreadID a_TID);

  enum SamplingState {
    Idle,
    Sampling,
    PendingStop,
    Processing,
    DoneProcessing
  };
  SamplingState GetState() const { return m_State; }
  void SetState(SamplingState a_State) { m_State = a_State; }
  const std::vector<ThreadSampleData*>& GetThreadSampleData() const {
    return m_SortedThreadSampleData;
  }
  const ThreadSampleData* GetThreadSampleDataByThreadId(int32_t tid) const {
    auto it = data->thread_id_to_sample_data().find(tid);
    if (it == data->thread_id_to_sample_data().end()) {
      return nullptr;
    }

    return &it->second;
  }

  void SetGenerateSummary(bool a_Value) { m_GenerateSummary = a_Value; }
  bool GetGenerateSummary() const { return m_GenerateSummary; }
  void SortByThreadUsage();
  void ProcessSamples();
  void UpdateAddressInfo(uint64_t address);
  [[nodiscard]] const ThreadSampleData* GetSummary() const;
  [[nodiscard]] unsigned int GetCountOfFunction(
      uint64_t function_address) const;

  const std::unique_ptr<SamplingProfilerData>& GetData() { return data; }

 protected:
  void ResolveCallstacks();
  void FillThreadSampleDataSampleReports();
  ThreadSampleData& GetThreadSampleData(ThreadID thread_id);

 protected:
  std::shared_ptr<Process> m_Process;
  std::atomic<SamplingState> m_State;
  Timer m_SamplingTimer;
  float m_SampleTimeSeconds = FLT_MAX;
  bool m_GenerateSummary = true;
  Mutex m_Mutex;

  // Filled before ProcessSamples by AddCallstack, AddHashedCallstack.
  BlockChain<CallstackEvent, 16 * 1024> m_Callstacks;

  // Filled by ProcessSamples.
  std::vector<ThreadSampleData*> m_SortedThreadSampleData;

  std::unique_ptr<SamplingProfilerData> data;
};

#endif  // ORBIT_CORE_SAMPLING_PROFILER_H_
