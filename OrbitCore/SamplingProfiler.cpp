// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingProfiler.h"

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "Capture.h"
#include "FunctionUtils.h"
#include "Injection.h"
#include "Log.h"
#include "OrbitModule.h"
#include "Serialization.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::LinuxAddressInfo;

namespace {

std::multimap<int, CallstackID> SortCallstacks(
    const ThreadSampleData& data, const std::set<CallstackID>& a_CallStacks,
    int* o_TotalCallStacks) {
  std::multimap<int, CallstackID> sortedCallstacks;
  int numCallstacks = 0;
  for (CallstackID id : a_CallStacks) {
    auto it = data.m_CallstackCount.find(id);
    if (it != data.m_CallstackCount.end()) {
      int count = it->second;
      sortedCallstacks.insert(std::make_pair(count, id));
      numCallstacks += count;
    }
  }

  *o_TotalCallStacks = numCallstacks;
  return sortedCallstacks;
}

void ComputeAverageThreadUsage(ThreadSampleData* data) {
  data->m_AverageThreadUsage = 0.f;

  if (!data->m_ThreadUsage.empty()) {
    for (float thread_usage : data->m_ThreadUsage) {
      data->m_AverageThreadUsage += thread_usage;
    }

    data->m_AverageThreadUsage /= data->m_ThreadUsage.size();
  }
}

}  // namespace

//-----------------------------------------------------------------------------
std::multimap<int, CallstackID> SamplingProfiler::GetCallstacksFromAddress(
    uint64_t a_Addr, ThreadID a_TID, int* o_NumCallstacks) {
  std::set<CallstackID>& callstacks = m_FunctionToCallstacks[a_Addr];
  return SortCallstacks(m_ThreadSampleData[a_TID], callstacks, o_NumCallstacks);
}

//-----------------------------------------------------------------------------
void SamplingProfiler::AddCallStack(CallstackEvent& callstack_event) {
  CallstackID hash = callstack_event.callstack_hash();
  if (!HasCallStack(hash)) {
    std::shared_ptr<CallStack> callstack =
        Capture::GSamplingProfiler->GetCallStack(hash);
    AddUniqueCallStack(*callstack);
  }

  m_Callstacks.push_back(callstack_event);
}

//-----------------------------------------------------------------------------
void SamplingProfiler::AddUniqueCallStack(CallStack& a_CallStack) {
  m_UniqueCallstacks[a_CallStack.Hash()] =
      std::make_shared<CallStack>(a_CallStack);
}

std::shared_ptr<CallStack> SamplingProfiler::GetResolvedCallstack(
    CallstackID raw_callstack_id) const {
  auto resolved_callstack_id_it =
      m_OriginalCallstackToResolvedCallstack.find(raw_callstack_id);
  if (resolved_callstack_id_it ==
      m_OriginalCallstackToResolvedCallstack.end()) {
    return nullptr;
  }

  auto resolved_callstack_it =
      m_UniqueResolvedCallstacks.find(resolved_callstack_id_it->second);
  if (resolved_callstack_it == m_UniqueResolvedCallstacks.end()) {
    return nullptr;
  }
  return resolved_callstack_it->second;
}

//-----------------------------------------------------------------------------
std::shared_ptr<SortedCallstackReport>
SamplingProfiler::GetSortedCallstacksFromAddress(uint64_t a_Addr,
                                                 ThreadID a_TID) {
  std::shared_ptr<SortedCallstackReport> report =
      std::make_shared<SortedCallstackReport>();
  std::multimap<int, CallstackID> multiMap =
      GetCallstacksFromAddress(a_Addr, a_TID, &report->m_NumCallStacksTotal);
  size_t numUniqueCallstacks = multiMap.size();
  report->m_CallStacks.resize(numUniqueCallstacks);
  size_t index = numUniqueCallstacks;

  for (auto& pair : multiMap) {
    CallstackCount& callstack = report->m_CallStacks[--index];
    callstack.m_Count = pair.first;
    callstack.m_CallstackId = pair.second;
  }

  return report;
}

//-----------------------------------------------------------------------------
void SamplingProfiler::SortByThreadUsage() {
  m_SortedThreadSampleData.clear();
  m_SortedThreadSampleData.reserve(m_ThreadSampleData.size());

  // "All"
  m_ThreadSampleData[0].m_AverageThreadUsage = 100.f;

  for (auto& pair : m_ThreadSampleData) {
    ThreadSampleData& data = pair.second;
    data.m_TID = pair.first;
    m_SortedThreadSampleData.push_back(&data);
  }

  sort(m_SortedThreadSampleData.begin(), m_SortedThreadSampleData.end(),
       [](const ThreadSampleData* a, const ThreadSampleData* b) {
         return a->m_AverageThreadUsage > b->m_AverageThreadUsage;
       });
}

//-----------------------------------------------------------------------------
void SamplingProfiler::ProcessSamples() {
  // Clear the result of a previous call to ProcessSamples.
  m_ThreadSampleData.clear();
  m_UniqueResolvedCallstacks.clear();
  m_OriginalCallstackToResolvedCallstack.clear();
  m_FunctionToCallstacks.clear();
  m_ExactAddressToFunctionAddress.clear();
  m_FunctionAddressToExactAddresses.clear();
  m_SortedThreadSampleData.clear();

  // Unique call stacks and per thread data
  for (const CallstackEvent& callstack : m_Callstacks) {
    if (!HasCallStack(callstack.callstack_hash())) {
      ERROR("Processed unknown callstack!");
      continue;
    }

    ThreadSampleData& threadSampleData =
        m_ThreadSampleData[callstack.thread_id()];
    threadSampleData.m_NumSamples++;
    threadSampleData.m_CallstackCount[callstack.callstack_hash()]++;
    for (uint64_t address :
         m_UniqueCallstacks[callstack.callstack_hash()]->m_Data) {
      threadSampleData.m_RawAddressCount[address]++;
    }

    if (m_GenerateSummary) {
      ThreadSampleData& threadSampleDataAll = m_ThreadSampleData[0];
      threadSampleDataAll.m_NumSamples++;
      threadSampleDataAll.m_CallstackCount[callstack.callstack_hash()]++;
      for (uint64_t address :
           m_UniqueCallstacks[callstack.callstack_hash()]->m_Data) {
        threadSampleDataAll.m_RawAddressCount[address]++;
      }
    }
  }

  ResolveCallstacks();

  for (auto& dataIt : m_ThreadSampleData) {
    ThreadSampleData& threadSampleData = dataIt.second;

    ComputeAverageThreadUsage(&threadSampleData);

    // Address count per sample per thread
    for (auto& stackCountIt : threadSampleData.m_CallstackCount) {
      const CallstackID callstackID = stackCountIt.first;
      const uint32_t callstackCount = stackCountIt.second;

      CallstackID resolvedCallstackID =
          m_OriginalCallstackToResolvedCallstack[callstackID];
      std::shared_ptr<CallStack>& resolvedCallstack =
          m_UniqueResolvedCallstacks[resolvedCallstackID];

      // exclusive stat
      threadSampleData.m_ExclusiveCount[resolvedCallstack->m_Data[0]] +=
          callstackCount;

      std::set<uint64_t> uniqueAddresses;
      for (uint32_t i = 0; i < resolvedCallstack->m_Data.size(); ++i) {
        uniqueAddresses.insert(resolvedCallstack->m_Data[i]);
      }

      for (uint64_t address : uniqueAddresses) {
        threadSampleData.m_AddressCount[address] += callstackCount;
      }
    }

    // sort thread addresses by count
    for (auto& addressCountIt : threadSampleData.m_AddressCount) {
      const uint64_t address = addressCountIt.first;
      const uint32_t count = addressCountIt.second;
      threadSampleData.m_AddressCountSorted.insert(
          std::make_pair(count, address));
    }
  }

  SortByThreadUsage();

  FillThreadSampleDataSampleReports();

  m_NumSamples = m_Callstacks.size();

  // Don't clear m_Callstacks, so that ProcessSamples can be called again, e.g.
  // when new callstacks have been added or after a module has been loaded.
}

//-----------------------------------------------------------------------------
void SamplingProfiler::ResolveCallstacks() {
  for (const auto& it : m_UniqueCallstacks) {
    CallstackID rawCallstackId = it.first;
    const std::shared_ptr<CallStack> callstack = it.second;
    // A "resolved callstack" is a callstack where every address is replaced by
    // the start address of the function (if known).
    CallStack resolved_callstack = *callstack;

    for (uint32_t i = 0; i < callstack->m_Data.size(); ++i) {
      uint64_t addr = callstack->m_Data[i];

      if (m_ExactAddressToFunctionAddress.find(addr) ==
          m_ExactAddressToFunctionAddress.end()) {
        UpdateAddressInfo(addr);
      }

      auto addrIt = m_ExactAddressToFunctionAddress.find(addr);
      if (addrIt != m_ExactAddressToFunctionAddress.end()) {
        const uint64_t& functionAddr = addrIt->second;
        resolved_callstack.m_Data[i] = functionAddr;
        m_FunctionToCallstacks[functionAddr].insert(rawCallstackId);
      }
    }

    CallstackID resolvedCallstackId = resolved_callstack.Hash();
    if (m_UniqueResolvedCallstacks.find(resolvedCallstackId) ==
        m_UniqueResolvedCallstacks.end()) {
      m_UniqueResolvedCallstacks[resolvedCallstackId] =
          std::make_shared<CallStack>(resolved_callstack);
    }

    m_OriginalCallstackToResolvedCallstack[rawCallstackId] =
        resolvedCallstackId;
  }
}

//-----------------------------------------------------------------------------
const ThreadSampleData* SamplingProfiler::GetSummary() const {
  auto summary_it = m_ThreadSampleData.find(0);
  if (summary_it == m_ThreadSampleData.end()) {
    return nullptr;
  }
  return &(summary_it->second);
}

//-----------------------------------------------------------------------------
uint32_t SamplingProfiler::GetCountOfFunction(uint64_t function_address) const {
  auto addresses_of_functions_itr =
      m_FunctionAddressToExactAddresses.find(function_address);
  if (addresses_of_functions_itr == m_FunctionAddressToExactAddresses.end()) {
    return 0;
  }
  uint32_t result = 0;
  const ThreadSampleData* summary = GetSummary();
  if (summary == nullptr) {
    return 0;
  }
  const auto& function_addresses = addresses_of_functions_itr->second;
  for (uint64_t address : function_addresses) {
    auto count_itr = summary->m_RawAddressCount.find(address);
    if (count_itr != summary->m_RawAddressCount.end()) {
      result += count_itr->second;
    }
  }
  return result;
}

//-----------------------------------------------------------------------------
void SamplingProfiler::UpdateAddressInfo(uint64_t address) {
  LinuxAddressInfo* address_info = Capture::GetAddressInfo(address);
  FunctionInfo* function = m_Process->GetFunctionFromAddress(address, false);

  // Find the start address of the function this address falls inside.
  // Use the Function returned by Process::GetFunctionFromAddress, and
  // when this fails (e.g., the module containing the function has not
  // been loaded) use (for now) the LinuxAddressInfo that is collected
  // for every address in a callstack. SamplingProfiler relies heavily
  // on the association between address and function address held by
  // m_ExactAddressToFunctionAddress, otherwise each address is
  // considered a different function.
  uint64_t function_address;
  std::string function_name = "???";
  if (function != nullptr) {
    function_address = FunctionUtils::GetAbsoluteAddress(*function);
    function_name = FunctionUtils::GetDisplayName(*function);
  } else if (address_info != nullptr) {
    function_address = address - address_info->offset_in_function();
    if (!address_info->function_name().empty()) {
      function_name = address_info->function_name();
    }
  } else {
    function_address = address;
  }

  if (function != nullptr && address_info != nullptr) {
    address_info->set_function_name(FunctionUtils::GetDisplayName(*function));
  }

  m_ExactAddressToFunctionAddress[address] = function_address;
  m_FunctionAddressToExactAddresses[function_address].insert(address);

  Capture::GAddressToFunctionName[address] = function_name;
  Capture::GAddressToFunctionName[function_address] = function_name;

  std::string module_name = "???";
  std::shared_ptr<Module> module = m_Process->GetModuleFromAddress(address);
  if (module != nullptr) {
    module_name = module->m_Name;
  } else if (address_info != nullptr) {
    module_name = Path::GetFileName(address_info->module_name());
  }
  Capture::GAddressToModuleName[address] = module_name;
  Capture::GAddressToModuleName[function_address] = module_name;
}

//-----------------------------------------------------------------------------
void SamplingProfiler::FillThreadSampleDataSampleReports() {
  for (auto& data : m_ThreadSampleData) {
    ThreadID threadID = data.first;
    ThreadSampleData& threadSampleData = data.second;
    std::vector<SampledFunction>& sampleReport =
        threadSampleData.m_SampleReport;

    ORBIT_LOGV(threadID);
    ORBIT_LOGV(threadSampleData.m_NumSamples);

    for (auto sortedIt = threadSampleData.m_AddressCountSorted.rbegin();
         sortedIt != threadSampleData.m_AddressCountSorted.rend(); ++sortedIt) {
      uint32_t numOccurences = sortedIt->first;
      uint64_t address = sortedIt->second;
      float inclusive_percent =
          100.f * numOccurences / threadSampleData.m_NumSamples;

      SampledFunction function;
      // GAddressToFunctionName and GAddressToModuleName should be filled in
      // UpdateAddressInfo()
      CHECK(Capture::GAddressToFunctionName.count(address) > 0);
      function.m_Name = Capture::GAddressToFunctionName.at(address);
      function.m_Inclusive = inclusive_percent;
      function.m_Exclusive = 0.f;
      auto it = threadSampleData.m_ExclusiveCount.find(address);
      if (it != threadSampleData.m_ExclusiveCount.end()) {
        function.m_Exclusive =
            100.f * it->second / threadSampleData.m_NumSamples;
      }
      function.m_Address = address;
      CHECK(Capture::GAddressToModuleName.count(address) > 0);
      function.m_Module = Capture::GAddressToModuleName.at(address);

      sampleReport.push_back(function);
    }
  }
}
