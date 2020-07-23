// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_SAMPLING_PROFILER_H_
#define ORBIT_CORE_SAMPLING_PROFILER_H_

#include <utility>

#include "BlockChain.h"
#include "Callstack.h"
#include "Capture.h"
#include "Core.h"
#include "EventBuffer.h"
#include "Pdb.h"
#include "SerializationMacros.h"

class Process;

//-----------------------------------------------------------------------------
struct SampledFunction {
  SampledFunction() = default;

  std::string m_Name;
  std::string m_Module;
  std::string m_File;
  float m_Exclusive = 0;
  float m_Inclusive = 0;
  int m_Line = 0;
  uint64_t m_Address = 0;
  Function* m_Function = nullptr;
};

//-----------------------------------------------------------------------------
struct ThreadSampleData {
  ThreadSampleData() { m_ThreadUsage.push_back(0); }
  std::unordered_map<CallstackID, uint32_t> m_CallstackCount;
  std::unordered_map<uint64_t, uint32_t> m_AddressCount;
  std::unordered_map<uint64_t, uint32_t> m_RawAddressCount;
  std::unordered_map<uint64_t, uint32_t> m_ExclusiveCount;
  std::multimap<uint32_t, uint64_t> m_AddressCountSorted;
  uint32_t m_NumSamples = 0;
  std::vector<SampledFunction> m_SampleReport;
  std::vector<float> m_ThreadUsage;
  float m_AverageThreadUsage = 0;
  ThreadID m_TID = 0;
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
  explicit SamplingProfiler(std::shared_ptr<Process> a_Process)
      : m_Process{std::move(a_Process)} {}
  SamplingProfiler() : SamplingProfiler{std::make_shared<Process>()} {}

  int GetNumSamples() const { return m_NumSamples; }

  void AddCallStack(CallstackEvent& callstack_event);
  void AddHashedCallStack(CallstackEvent& a_CallStack);
  void AddUniqueCallStack(CallStack& a_CallStack);

  std::shared_ptr<CallStack> GetCallStack(CallstackID a_ID) {
    return m_UniqueCallstacks.at(a_ID);
  }

  bool HasCallStack(CallstackID a_ID) {
    return m_UniqueCallstacks.count(a_ID) > 0;
  }

  std::multimap<int, CallstackID> GetCallstacksFromAddress(
      uint64_t a_Addr, ThreadID a_TID, int* o_NumCallstacks);
  std::shared_ptr<SortedCallstackReport> GetSortedCallstacksFromAddress(
      uint64_t a_Addr, ThreadID a_TID);

  BlockChain<CallstackEvent, 16 * 1024>* GetCallstacks() {
    return &m_Callstacks;
  }

  const std::vector<ThreadSampleData*>& GetThreadSampleData() const {
    return m_SortedThreadSampleData;
  }
  const ThreadSampleData* GetThreadSampleDataByThreadId(int32_t tid) const {
    auto it = m_ThreadSampleData.find(tid);
    if (it == m_ThreadSampleData.end()) {
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
  [[nodiscard]] uint32_t GetCountOfFunction(uint64_t function_address) const;

  void SaveCallstacks() {
    callstacks_vector.clear();
    for (const CallstackEvent& callstack : m_Callstacks) {
      callstacks_vector.push_back(callstack);
    }
  }

  void LoadCallstacks() {
    m_Callstacks.clear();
    for (const CallstackEvent& callstack : callstacks_vector) {
      m_Callstacks.push_back(callstack);
    }
  }

  ORBIT_SERIALIZABLE;

 protected:
  void ResolveCallstacks();
  void FillThreadSampleDataSampleReports();

 protected:
  std::shared_ptr<Process> m_Process;
  bool m_GenerateSummary = true;
  int m_NumSamples = 0;

  // Filled before ProcessSamples by AddCallstack, AddHashedCallstack.
  BlockChain<CallstackEvent, 16 * 1024> m_Callstacks;
  std::unordered_map<CallstackID, std::shared_ptr<CallStack>>
      m_UniqueCallstacks;

  // TODO(irinashkviro): remove this when move to protobuf
  std::vector<CallstackEvent> callstacks_vector;

  // Filled by ProcessSamples.
  std::unordered_map<ThreadID, ThreadSampleData> m_ThreadSampleData;
  std::unordered_map<CallstackID, std::shared_ptr<CallStack>>
      m_UniqueResolvedCallstacks;
  std::unordered_map<CallstackID, CallstackID>
      m_OriginalCallstackToResolvedCallstack;
  std::unordered_map<uint64_t, std::set<CallstackID>> m_FunctionToCallstacks;
  std::unordered_map<uint64_t, uint64_t> m_ExactAddressToFunctionAddress;
  std::unordered_map<uint64_t, std::unordered_set<uint64_t>>
      m_FunctionAddressToExactAddresses;
  std::vector<ThreadSampleData*> m_SortedThreadSampleData;
};

#endif  // ORBIT_CORE_SAMPLING_PROFILER_H_
