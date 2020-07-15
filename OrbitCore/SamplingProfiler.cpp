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
#include "SamplingUtils.h"
#include "Serialization.h"

namespace {

std::multimap<int, CallstackID> SortCallstacks(const ThreadSampleData& data,
                                               const Uint64List& a_CallStacks,
                                               int* o_TotalCallStacks) {
  std::multimap<int, CallstackID> sortedCallstacks;
  int numCallstacks = 0;
  for (CallstackID id : a_CallStacks.data()) {
    auto it = data.callstack_count().find(id);
    if (it != data.callstack_count().end()) {
      int count = it->second;
      sortedCallstacks.insert(std::make_pair(count, id));
      numCallstacks += count;
    }
  }

  *o_TotalCallStacks = numCallstacks;
  return sortedCallstacks;
}

void ComputeAverageThreadUsage(ThreadSampleData* data) {
  data->set_average_thread_usage(0.f);

  if (!data->thread_usage().empty()) {
    float total_usage = 0.f;
    for (float thread_usage : data->thread_usage()) {
      total_usage += thread_usage;
    }

    data->set_average_thread_usage(total_usage / data->thread_usage_size());
  }
}

}  // namespace

//-----------------------------------------------------------------------------
SamplingProfiler::SamplingProfiler(const std::shared_ptr<Process>& a_Process) {
  m_Process = a_Process;
  m_State = SamplingState::Idle;
  data = std::make_unique<SamplingProfilerData>();
}

SamplingProfiler::SamplingProfiler(
    std::unique_ptr<SamplingProfilerData> data_ptr) {
  data = std::move(data_ptr);
}

//-----------------------------------------------------------------------------
SamplingProfiler::SamplingProfiler()
    : SamplingProfiler{std::make_shared<Process>()} {}

//-----------------------------------------------------------------------------
void SamplingProfiler::StartCapture() {
  Capture::GIsSampling = true;

  m_SamplingTimer.Start();

  m_State = Sampling;
}

//-----------------------------------------------------------------------------
void SamplingProfiler::StopCapture() { m_State = PendingStop; }

//-----------------------------------------------------------------------------
float SamplingProfiler::GetSampleTime() {
  return m_State == Sampling
             ? static_cast<float>(m_SamplingTimer.QuerySeconds())
             : 0.f;
}

//-----------------------------------------------------------------------------
std::multimap<int, CallstackID> SamplingProfiler::GetCallstacksFromAddress(
    uint64_t a_Addr, ThreadID a_TID, int* o_NumCallstacks) {
  Uint64List callstacks = (*data->mutable_function_to_callstacks_set())[a_Addr];
  return SortCallstacks(GetThreadSampleData(a_TID), callstacks,
                        o_NumCallstacks);
}

//-----------------------------------------------------------------------------
void SamplingProfiler::AddCallStack(Callstack& a_CallStack) {
  CallstackID hash = SamplingUtils::InitAndGetCallstackHash(&a_CallStack);
  if (!HasCallStack(hash)) {
    AddUniqueCallStack(a_CallStack);
  }
  CallstackEvent hashed_cs;
  hashed_cs.set_callstack_id(hash);
  hashed_cs.set_thread_id(a_CallStack.thread_id());
  // Note: a_CallStack doesn't carry a timestamp so hashed_cs.m_Time is not
  // filled, but that is not a problem because SamplingProfiler doesn't use it.
  AddHashedCallStack(hashed_cs);
}

//-----------------------------------------------------------------------------
void SamplingProfiler::AddHashedCallStack(CallstackEvent& a_CallStack) {
  if (!HasCallStack(a_CallStack.callstack_id())) {
    ERROR("Callstacks can only be added by hash when already present.");
    return;
  }
  ScopeLock lock(m_Mutex);
  m_Callstacks.push_back(a_CallStack);
}

//-----------------------------------------------------------------------------
void SamplingProfiler::AddUniqueCallStack(Callstack& a_CallStack) {
  ScopeLock lock(m_Mutex);
  CallstackID hash = SamplingUtils::InitAndGetCallstackHash(&a_CallStack);
  (*data->mutable_unique_callstacks())[hash] = a_CallStack;
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
  m_SortedThreadSampleData.reserve(data->thread_id_to_sample_data_size());

  // "All"
  GetThreadSampleData(0).set_average_thread_usage(100.f);

  for (auto& pair : (*data->mutable_thread_id_to_sample_data())) {
    ThreadSampleData& data = pair.second;
    data.set_thread_id(pair.first);
    m_SortedThreadSampleData.push_back(&data);
  }

  sort(m_SortedThreadSampleData.begin(), m_SortedThreadSampleData.end(),
       [](const ThreadSampleData* a, const ThreadSampleData* b) {
         return a->average_thread_usage() > b->average_thread_usage();
       });
}

//-----------------------------------------------------------------------------
void SamplingProfiler::ProcessSamples() {
  ScopeLock lock(m_Mutex);

  m_State = Processing;

  // Clear the result of a previous call to ProcessSamples.
  data->clear_thread_id_to_sample_data();
  data->clear_unique_resolved_callstacks();
  data->clear_original_callstack_to_resolved_callstack();
  data->clear_function_to_callstacks_set();
  data->clear_exact_address_to_function_address();
  data->clear_function_address_to_exact_addresses_set();
  m_SortedThreadSampleData.clear();

  // Unique call stacks and per thread data
  for (const CallstackEvent& callstack : m_Callstacks) {
    if (!HasCallStack(callstack.callstack_id())) {
      ERROR("Processed unknown callstack!");
      continue;
    }

    ThreadSampleData& threadSampleData =
        GetThreadSampleData(callstack.thread_id());
    threadSampleData.set_num_samples(threadSampleData.num_samples() + 1);
    (*threadSampleData.mutable_callstack_count())[callstack.callstack_id()] +=
        1;
    if (data->unique_callstacks().contains(callstack.callstack_id())) {
      for (uint64_t address :
           data->unique_callstacks().at(callstack.callstack_id()).pcs()) {
        (*threadSampleData.mutable_raw_address_count())[address] += 1;
      }
    }

    if (m_GenerateSummary) {
      ThreadSampleData& threadSampleDataAll = GetThreadSampleData(0);
      threadSampleDataAll.set_num_samples(threadSampleDataAll.num_samples() +
                                          1);
      (*threadSampleDataAll
            .mutable_callstack_count())[callstack.callstack_id()] += 1;
      if (data->unique_callstacks().contains(callstack.callstack_id())) {
        for (uint64_t address :
             data->unique_callstacks().at(callstack.callstack_id()).pcs()) {
          (*threadSampleDataAll.mutable_raw_address_count())[address] += 1;
        }
      }
    }
  }

  ResolveCallstacks();

  for (auto& dataIt : (*data->mutable_thread_id_to_sample_data())) {
    ThreadSampleData& threadSampleData = dataIt.second;

    ComputeAverageThreadUsage(&threadSampleData);

    // Address count per sample per thread
    for (auto& stackCountIt : threadSampleData.callstack_count()) {
      const CallstackID callstackID = stackCountIt.first;
      const unsigned int callstackCount = stackCountIt.second;

      CallstackID resolvedCallstackID =
          (*data->mutable_original_callstack_to_resolved_callstack())
              [callstackID];
      Callstack& resolvedCallstack =
          (*data->mutable_unique_resolved_callstacks())[resolvedCallstackID];

      // exclusive stat
      uint64_t id = resolvedCallstack.pcs(0);
      (*threadSampleData.mutable_exclusive_count())[id] += callstackCount;

      std::set<uint64_t> uniqueAddresses;
      for (int i = 0; i < resolvedCallstack.pcs_size(); ++i) {
        uniqueAddresses.insert(resolvedCallstack.pcs(i));
      }

      for (uint64_t address : uniqueAddresses) {
        (*threadSampleData.mutable_address_count())[address] += callstackCount;
      }
    }
  }

  SortByThreadUsage();

  FillThreadSampleDataSampleReports();

  data->set_num_samples(m_Callstacks.size());

  // Don't clear m_Callstacks, so that ProcessSamples can be called again, e.g.
  // when new callstacks have been added or after a module has been loaded.

  m_State = DoneProcessing;
}

//-----------------------------------------------------------------------------
void SamplingProfiler::ResolveCallstacks() {
  ScopeLock lock(m_Mutex);
  for (const auto& it : data->unique_callstacks()) {
    CallstackID rawCallstackId = it.first;
    const Callstack& callstack = it.second;
    // A "resolved callstack" is a callstack where every address is replaced by
    // the start address of the function (if known).
    Callstack resolved_callstack = callstack;

    for (int i = 0; i < callstack.pcs_size(); ++i) {
      const uint64_t addr = callstack.pcs(i);

      if (!data->exact_address_to_function_address().contains(addr)) {
        UpdateAddressInfo(addr);
      }

      const uint64_t function_addr =
          data->exact_address_to_function_address().at(addr);
      resolved_callstack.set_pcs(i, function_addr);
      SamplingUtils::InsertToUint64Set(
          &(*data->mutable_function_to_callstacks_set())[function_addr],
          rawCallstackId);
    }

    CallstackID resolvedCallstackId =
        SamplingUtils::InitAndGetCallstackHash(&resolved_callstack);
    if (!data->unique_resolved_callstacks().contains(resolvedCallstackId)) {
      (*data->mutable_unique_resolved_callstacks())[resolvedCallstackId] =
          std::move(resolved_callstack);
    }

    (*data->mutable_original_callstack_to_resolved_callstack())
        [rawCallstackId] = resolvedCallstackId;
  }
}

//-----------------------------------------------------------------------------
const ThreadSampleData* SamplingProfiler::GetSummary() const {
  auto summary_it = data->thread_id_to_sample_data().find(0);
  if (summary_it == data->thread_id_to_sample_data().end()) {
    return nullptr;
  }
  return &((*summary_it).second);
}

//-----------------------------------------------------------------------------
unsigned int SamplingProfiler::GetCountOfFunction(
    uint64_t function_address) const {
  auto function_to_address = data->function_address_to_exact_addresses_set();
  auto addresses_of_functions_itr = function_to_address.find(function_address);
  if (addresses_of_functions_itr == function_to_address.end()) {
    return 0;
  }

  unsigned int result = 0;
  const ThreadSampleData* summary = GetSummary();
  if (summary == nullptr) {
    return 0;
  }

  const auto& function_addresses = addresses_of_functions_itr->second;
  for (uint64_t address : function_addresses.data()) {
    auto count_itr = summary->raw_address_count().find(address);
    if (count_itr != summary->raw_address_count().end()) {
      result += (*count_itr).second;
    }
  }
  return result;
}

//-----------------------------------------------------------------------------
void SamplingProfiler::UpdateAddressInfo(uint64_t address) {
  ScopeLock lock(m_Mutex);

  AddressInfo* address_info = Capture::GetAddressInfo(address);
  Function* function = m_Process->GetFunctionFromAddress(address, false);

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

  (*data->mutable_exact_address_to_function_address())[address] =
      function_address;
  SamplingUtils::InsertToUint64Set(
      &(*data->mutable_function_address_to_exact_addresses_set())
          [function_address],
      address);

  (*Capture::GData.mutable_address_to_function_name())[address] = function_name;
  (*Capture::GData.mutable_address_to_function_name())[function_address] =
      function_name;
}

//-----------------------------------------------------------------------------
void SamplingProfiler::FillThreadSampleDataSampleReports() {
  for (auto& data : (*data->mutable_thread_id_to_sample_data())) {
    ThreadID threadID = data.first;
    ThreadSampleData& thread_sample_data = data.second;

    ORBIT_LOGV(threadID);
    ORBIT_LOGV(thread_sample_data.num_samples());

    std::multimap<uint32_t, uint64_t> sorted_count_address;
    for (const auto& address_count : thread_sample_data.address_count()) {
      sorted_count_address.insert(
          std::make_pair(address_count.second, address_count.first));
    }
    for (auto sortedIt = sorted_count_address.rbegin();
         sortedIt != sorted_count_address.rend(); ++sortedIt) {
      unsigned int numOccurences = sortedIt->first;
      uint64_t address = sortedIt->second;
      float inclusive_percent =
          100.f * numOccurences / thread_sample_data.num_samples();

      SampledFunction* function = thread_sample_data.add_sample_report();
      function->set_name(
          (*Capture::GData.mutable_address_to_function_name())[address]);
      function->set_inclusive(inclusive_percent);
      function->set_exclusive(0.f);
      auto it = thread_sample_data.exclusive_count().find(address);
      if (it != thread_sample_data.exclusive_count().end()) {
        function->set_exclusive(100.f * it->second /
                                thread_sample_data.num_samples());
      }
      function->set_address(address);

      std::shared_ptr<Module> module = m_Process->GetModuleFromAddress(address);
      function->set_module(module ? module->m_Name : "???");
    }
  }
}

ThreadSampleData& SamplingProfiler::GetThreadSampleData(ThreadID thread_id) {
  if (!data->thread_id_to_sample_data().contains(thread_id)) {
    (*data->mutable_thread_id_to_sample_data())[thread_id] =
        SamplingUtils::CreateThreadSampleData();
  }
  return data->mutable_thread_id_to_sample_data()->at(thread_id);
}
