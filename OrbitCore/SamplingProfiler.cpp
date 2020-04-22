//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "SamplingProfiler.h"

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "Capture.h"
#include "Injection.h"
#include "Log.h"
#include "OrbitModule.h"
#include "OrbitThread.h"
#include "Params.h"
#include "Serialization.h"

#ifdef _WIN32
#include <dia2.h>

#include "SymbolUtils.h"
#endif

double GThreadUsageSamplePeriodMs = 200.0;

//-----------------------------------------------------------------------------
SamplingProfiler::SamplingProfiler(const std::shared_ptr<Process>& a_Process) {
  m_Process = a_Process;
  m_State = SamplingState::Idle;
}

//-----------------------------------------------------------------------------
SamplingProfiler::SamplingProfiler()
    : SamplingProfiler{std::make_shared<Process>()} {}

//-----------------------------------------------------------------------------
void SamplingProfiler::StartCapture() {
  Capture::GIsSampling = true;

  if (!m_Process->GetIsRemote()) {
    m_Process->EnumerateThreads();
  }

  m_SamplingTimer.Start();
  m_ThreadUsageTimer.Start();

  m_State = Sampling;
}

//-----------------------------------------------------------------------------
void SamplingProfiler::StopCapture() { m_State = PendingStop; }

//-----------------------------------------------------------------------------
void SamplingProfiler::SampleThreadsAsync() {
#ifdef _WIN32
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
  ReserveThreadData();

  while (m_State != PendingStop) {
    if (m_ThreadUsageTimer.QueryMillis() > GThreadUsageSamplePeriodMs) {
      GetThreadsUsage();
      m_ThreadUsageTimer.Start();
    }

    for (const auto& thread : m_Process->GetThreads()) {
      DWORD result = SuspendThread(thread->m_Handle);
      if (result != (DWORD)-1) {
        int prev_priority = GetThreadPriority(thread->m_Handle);
        SetThreadPriority(thread->m_Handle, THREAD_PRIORITY_TIME_CRITICAL);
        GetThreadCallstack(thread.get());
        SetThreadPriority(thread->m_Handle, prev_priority);

        ResumeThread(thread->m_Handle);
      }
    }

    Sleep(m_PeriodMs);
  }

  ProcessSamples();
#endif
}

//-----------------------------------------------------------------------------
void SamplingProfiler::GetThreadsUsage() {
  for (const auto& thread : m_Process->GetThreads()) {
    m_ThreadSampleData[thread->m_TID].m_ThreadUsage.push_back(
        thread->GetUsage());
  }
}

//-----------------------------------------------------------------------------
void SamplingProfiler::ReserveThreadData() {
  for (const auto& thread : m_Process->GetThreads()) {
    m_ThreadSampleData[thread->m_TID].m_ThreadUsage.reserve(1024);
    m_ThreadSampleData[thread->m_TID].m_TID = thread->m_TID;
  }
}

//-----------------------------------------------------------------------------
float SamplingProfiler::GetSampleTime() {
  return m_State == Sampling ? (float)m_SamplingTimer.QuerySeconds() : 0.f;
}

//-----------------------------------------------------------------------------
bool SamplingProfiler::ShouldStop() {
  return m_State == Sampling
             ? m_SamplingTimer.QuerySeconds() > m_SampleTimeSeconds
             : false;
}

//-----------------------------------------------------------------------------
void SamplingProfiler::FireDoneProcessingCallbacks() {
  for (auto& callback : m_Callbacks) {
    callback();
  }
}

//-----------------------------------------------------------------------------
std::multimap<int, CallstackID> SamplingProfiler::GetCallstacksFromAddress(
    uint64_t a_Addr, ThreadID a_TID, int* o_NumCallstacks) {
  std::set<CallstackID>& callstacks = m_FunctionToCallstacks[a_Addr];
  return m_ThreadSampleData[a_TID].SortCallstacks(callstacks, o_NumCallstacks);
}

//-----------------------------------------------------------------------------
void SamplingProfiler::AddCallStack(CallStack& a_CallStack) {
  CallstackID hash = a_CallStack.Hash();
  if (!HasCallStack(hash)) {
    AddUniqueCallStack(a_CallStack);
  }
  CallstackEvent hashedCS;
  hashedCS.m_Id = hash;
  hashedCS.m_TID = a_CallStack.m_ThreadId;
  AddHashedCallStack(hashedCS);
}

//-----------------------------------------------------------------------------
void SamplingProfiler::AddHashedCallStack(CallstackEvent& a_CallStack) {
  if (!HasCallStack(a_CallStack.m_Id)) {
    LOG("Error: Callstacks can only be added by hash when already present.");
  }
  ScopeLock lock(m_Mutex);
  m_Callstacks.push_back(a_CallStack);
}

//-----------------------------------------------------------------------------
void SamplingProfiler::AddUniqueCallStack(CallStack& a_CallStack) {
  ScopeLock lock(m_Mutex);
  m_UniqueCallstacks[a_CallStack.Hash()] =
      std::make_shared<CallStack>(a_CallStack);
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
void SamplingProfiler::SortByThreadID() {
  m_SortedThreadSampleData.clear();
  m_SortedThreadSampleData.reserve(m_ThreadSampleData.size());

  for (auto& pair : m_ThreadSampleData) {
    ThreadSampleData& data = pair.second;
    m_SortedThreadSampleData.push_back(&data);
  }

  sort(m_SortedThreadSampleData.begin(), m_SortedThreadSampleData.end(),
       [](const ThreadSampleData* a, const ThreadSampleData* b) {
         return a->m_TID > b->m_TID;
       });
}

//-----------------------------------------------------------------------------
bool SamplingProfiler::GetLineInfo(uint64_t a_Address, LineInfo& a_LineInfo) {
  auto it = m_AddressToLineInfo.find(a_Address);
  if (it != m_AddressToLineInfo.end()) {
    a_LineInfo = it->second;
    a_LineInfo.m_File = m_FileNames[a_LineInfo.m_FileNameHash];
    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
void SamplingProfiler::Print() {
  ScopeLock lock(m_Mutex);
  for (auto& pair : m_UniqueCallstacks) {
    std::shared_ptr<CallStack> callstack = pair.second;
    if (callstack) {
      PRINT_VAR((void*)callstack->m_Hash);
      PRINT_VAR(callstack->m_Depth);
      for (uint32_t i = 0; i < callstack->m_Depth; ++i) {
        LOG("%s", m_AddressToName[callstack->m_Data[i]].c_str());
      }
    }
  }
}

//-----------------------------------------------------------------------------
void SamplingProfiler::ProcessSamplesAsync() {
  m_SamplingThread =
      std::make_unique<std::thread>(&SamplingProfiler::ProcessSamples, this);
  m_SamplingThread->detach();
}

//-----------------------------------------------------------------------------
void SamplingProfiler::ProcessSamples() {
  ScopeLock lock(m_Mutex);

  m_State = Processing;

  // Unique call stacks and per thread data
  for (const CallstackEvent& callstack : m_Callstacks) {
    if (!HasCallStack(callstack.m_Id)) {
      LOG("Error: Processed unknown callstack!");
    }

    ThreadSampleData& threadSampleData = m_ThreadSampleData[callstack.m_TID];
    threadSampleData.m_NumSamples++;
    threadSampleData.m_CallstackCount[callstack.m_Id]++;

    if (m_GenerateSummary) {
      ThreadSampleData& threadSampleDataAll = m_ThreadSampleData[0];
      threadSampleDataAll.m_NumSamples++;
      threadSampleDataAll.m_CallstackCount[callstack.m_Id]++;
    }
  }

  ResolveCallstacks();

  for (auto& dataIt : m_ThreadSampleData) {
    ThreadSampleData& threadSampleData = dataIt.second;

    threadSampleData.ComputeAverageThreadUsage();

    // Address count per sample per thread
    for (auto& stackCountIt : threadSampleData.m_CallstackCount) {
      const CallstackID callstackID = stackCountIt.first;
      const unsigned int callstackCount = stackCountIt.second;

      CallstackID resolvedCallstackID =
          m_OriginalCallstackToResolvedCallstack[callstackID];
      std::shared_ptr<CallStack>& resolvedCallstack =
          m_UniqueResolvedCallstacks[resolvedCallstackID];

      // exclusive stat
      threadSampleData.m_ExclusiveCount[resolvedCallstack->m_Data[0]] +=
          callstackCount;

      std::set<uint64_t> uniqueAddresses;
      for (uint32_t i = 0; i < resolvedCallstack->m_Depth; ++i) {
        uniqueAddresses.insert(resolvedCallstack->m_Data[i]);
      }

      for (uint64_t address : uniqueAddresses) {
        threadSampleData.m_AddressCount[address] += callstackCount;
      }
    }

    // sort thread addresses by count
    for (auto& addressCountIt : threadSampleData.m_AddressCount) {
      const uint64_t address = addressCountIt.first;
      const unsigned int count = addressCountIt.second;
      threadSampleData.m_AddressCountSorted.insert(
          std::make_pair(count, address));
    }
  }

  SortByThreadUsage();

  FillThreadSampleDataSampleReports();

  m_NumSamples = m_Callstacks.size();
  m_Callstacks.clear();
  m_State = DoneProcessing;
}

//-----------------------------------------------------------------------------
void ThreadSampleData::ComputeAverageThreadUsage() {
  m_AverageThreadUsage = 0.f;

  if (!m_ThreadUsage.empty()) {
    for (float thread_usage : m_ThreadUsage) {
      m_AverageThreadUsage += thread_usage;
    }

    m_AverageThreadUsage /= (float)m_ThreadUsage.size();
  }
}

//-----------------------------------------------------------------------------
std::multimap<int, CallstackID> ThreadSampleData::SortCallstacks(
    const std::set<CallstackID>& a_CallStacks, int* o_TotalCallStacks) {
  std::multimap<int, CallstackID> sortedCallstacks;
  int numCallstacks = 0;
  for (CallstackID id : a_CallStacks) {
    auto it = m_CallstackCount.find(id);
    if (it != m_CallstackCount.end()) {
      int count = it->second;
      sortedCallstacks.insert(std::make_pair(count, id));
      numCallstacks += count;
    }
  }

  *o_TotalCallStacks = numCallstacks;
  return sortedCallstacks;
}

//-----------------------------------------------------------------------------
void SamplingProfiler::ResolveCallstacks() {
  ScopeLock lock(m_Mutex);
  for (const auto& it : m_UniqueCallstacks) {
    CallstackID rawCallstackId = it.first;
    const std::shared_ptr<CallStack> callstack = it.second;
    // A "resolved callstack" is a callstack where every address is replaced by
    // the start address of the function (if known).
    CallStack resolved_callstack = *callstack;

    for (uint32_t i = 0; i < callstack->m_Depth; ++i) {
      uint64_t addr = callstack->m_Data[i];

      if (m_ExactAddressToFunctionAddress.find(addr) ==
          m_ExactAddressToFunctionAddress.end()) {
        AddAddress(addr);
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
void SamplingProfiler::AddAddress(uint64_t a_Address) {
  ScopeLock lock(m_Mutex);
#ifdef _WIN32
  if (!m_IsLinuxPerf) {
    unsigned char buffer[1024];
    memset(buffer, 0, 1024);
    SYMBOL_INFOW* symbol_info = (SYMBOL_INFOW*)buffer;
    symbol_info->SizeOfStruct = sizeof(SYMBOL_INFOW);
    symbol_info->MaxNameLen =
        ((sizeof(buffer) - sizeof(SYMBOL_INFOW)) / sizeof(WCHAR)) - 1;
    uint64_t displacement = 0;

    if (m_Process) {
      SymFromAddrW(m_Process->GetHandle(), (uint64_t)a_Address, &displacement,
                   symbol_info);
    }

    std::string symName = ws2s(symbol_info->Name);
    if (symName.empty()) {
      symName = absl::StrFormat("%" PRIx64, a_Address);
      PRINT_VAR(GetLastErrorAsString());

      std::shared_ptr<OrbitDiaSymbol> symbol =
          m_Process->SymbolFromAddress(a_Address);
      if (symbol->m_Symbol) {
        BSTR bstrName;
        if (symbol->m_Symbol->get_name(&bstrName) == S_OK) {
          symName = ws2s(bstrName);
          SysFreeString(bstrName);
        }
      }
    }

    m_ExactAddressToFunctionAddress[a_Address] =
        symbol_info->Address ? symbol_info->Address : a_Address;
    m_AddressToName[a_Address] = symName;
    m_AddressToName[symbol_info->Address] = symName;

    LineInfo lineInfo;
    if (SymUtils::GetLineInfo(a_Address, lineInfo)) {
      uint64_t hash = StringHash(lineInfo.m_File);
      lineInfo.m_FileNameHash = hash;
      m_FileNames[hash] = lineInfo.m_File;
      lineInfo.m_File = "";
      m_AddressToLineInfo[a_Address] = lineInfo;
    }
  } else
#endif
  {
    LinuxAddressInfo* address_info = m_Process->GetLinuxAddressInfo(a_Address);

    Function* function = m_Process->GetFunctionFromAddress(a_Address, false);

    // Find the start address of the function this address falls inside. (In the
    // Windows code in the if branch, symbol_info->Address already contains the
    // function's start address, but it's not the case here.) Use the Function
    // returned by Process::GetFunctionFromAddress, and when this fails (e.g.,
    // the module containing the function has not been loaded) use (for now)
    // the LinuxAddressInfo that is collected for every address in a callstack.
    // SamplingProfiler relies heavily on the association between address and
    // function address held by m_ExactAddressToFunctionAddress, otherwise each
    // address is considered a different function.
    uint64_t function_address;
    if (function != nullptr) {
      function_address = function->GetVirtualAddress();
    } else if (address_info != nullptr) {
      function_address = a_Address - address_info->offset_in_function;
    } else {
      function_address = a_Address;
    }
    m_ExactAddressToFunctionAddress[a_Address] = function_address;

    std::string function_name = "???";
    if (address_info == nullptr) {
      if (function != nullptr) {
        function_name = function->PrettyName();
      }
    } else if (absl::StrContains(address_info->function_name, "[unknown]")) {
      if (function != nullptr) {
        function_name = function->PrettyName();
        address_info->function_name = function->PrettyName();
      }
    } else {
      function_name = address_info->function_name;
    }
    m_AddressToName[a_Address] = function_name;
    m_AddressToName[function_address] = function_name;
  }
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
      unsigned int numOccurences = sortedIt->first;
      uint64_t address = sortedIt->second;
      float inclusive_percent =
          100.f * (float)numOccurences / (float)threadSampleData.m_NumSamples;

      SampledFunction function;
      function.m_Name = m_AddressToName[address];
      function.m_Inclusive = inclusive_percent;
      function.m_Exclusive = 0.f;
      auto it = threadSampleData.m_ExclusiveCount.find(address);
      if (it != threadSampleData.m_ExclusiveCount.end()) {
        function.m_Exclusive =
            100.f * (float)it->second / (float)threadSampleData.m_NumSamples;
      }
      function.m_Address = address;

      std::shared_ptr<Module> module = m_Process->GetModuleFromAddress(address);
      function.m_Module = module ? module->m_Name : "???";

      const LineInfo& lineInfo = m_AddressToLineInfo[address];
      function.m_Line = lineInfo.m_Line;
      function.m_File = lineInfo.m_File;
      sampleReport.push_back(function);
    }
  }
}

//-----------------------------------------------------------------------------
void SamplingProfiler::GetThreadCallstack(Thread* a_Thread) {
#ifdef _WIN32
  StackFrame frame(a_Thread->m_Handle);

  unsigned int depth = 0;
  while (StackWalk64(frame.m_ImageType, m_Process->GetHandle(),
                     a_Thread->m_Handle, &frame.m_StackFrame, &frame.m_Context,
                     nullptr, &SymFunctionTableAccess64, &SymGetModuleBase64,
                     nullptr) &&
         frame.m_StackFrame.AddrPC.Offset && depth < ORBIT_STACK_SIZE) {
    frame.m_Callstack.m_Data[depth++] = frame.m_StackFrame.AddrPC.Offset;
  }

  if (depth > 0) {
    frame.m_Callstack.m_Depth = depth;
    frame.m_Callstack.m_ThreadId = a_Thread->m_TID;
    AddCallStack(frame.m_Callstack);
  }
#else
  UNUSED(a_Thread);
#endif
}

//-----------------------------------------------------------------------------
std::string SamplingProfiler::GetSymbolFromAddress(uint64_t a_Address) {
  ScopeLock lock(m_Mutex);

  auto it = m_AddressToName.find(a_Address);
  if (it != m_AddressToName.end()) {
    return it->second;
  } else if (!m_LoadedFromFile) {
    AddAddress(a_Address);
    it = m_AddressToName.find(a_Address);
    if (it != m_AddressToName.end()) {
      return it->second;
    }
  }

  return "UnknownSymbol";
}

//-----------------------------------------------------------------------------
bool SampledFunction::GetSelected() {
  if (m_Function == nullptr) {
    m_Function =
        Capture::GTargetProcess->GetFunctionFromAddress(m_Address, false);
  }
  return m_Function ? m_Function->IsSelected() : false;
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE_WSTRING(SampledFunction, 0) {
  ORBIT_NVP_VAL(0, m_Name);
  ORBIT_NVP_VAL(0, m_Module);
  ORBIT_NVP_VAL(0, m_File);
  ORBIT_NVP_VAL(0, m_Exclusive);
  ORBIT_NVP_VAL(0, m_Inclusive);
  ORBIT_NVP_VAL(0, m_Line);
  ORBIT_NVP_VAL(0, m_Address);
  // Function* m_Function;
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE_WSTRING(SamplingProfiler, 1) {
  ORBIT_NVP_VAL(0, m_PeriodMs);
  ORBIT_NVP_VAL(0, m_NumSamples);
  ORBIT_NVP_DEBUG(0, m_ThreadSampleData);
  ORBIT_NVP_DEBUG(0, m_UniqueCallstacks);
  ORBIT_NVP_DEBUG(0, m_UniqueResolvedCallstacks);
  ORBIT_NVP_DEBUG(0, m_OriginalCallstackToResolvedCallstack);
  ORBIT_NVP_DEBUG(0, m_FunctionToCallstacks);
  ORBIT_NVP_DEBUG(0, m_ExactAddressToFunctionAddress);
  ORBIT_NVP_DEBUG(0, m_AddressToName);
  ORBIT_NVP_DEBUG(0, m_AddressToLineInfo);
  ORBIT_NVP_DEBUG(1, m_FileNames);
  // ORBIT_NVP_VAL( 0, m_Callbacks );
  // ORBIT_NVP_VAL( 0, m_SortedThreadSampleData );
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE_WSTRING(SortedCallstackReport, 0) {
  ORBIT_NVP_VAL(0, m_NumCallStacksTotal);
  ORBIT_NVP_VAL(0, m_CallStacks);
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE_WSTRING(CallstackCount, 0) {
  ORBIT_NVP_VAL(0, m_Count);
  ORBIT_NVP_VAL(0, m_CallstackId);
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE_WSTRING(ThreadSampleData, 0) {
  ORBIT_NVP_VAL(0, m_CallstackCount);
  ORBIT_NVP_VAL(0, m_AddressCount);
  ORBIT_NVP_VAL(0, m_ExclusiveCount);
  ORBIT_NVP_VAL(0, m_AddressCountSorted);
  ORBIT_NVP_VAL(0, m_NumSamples);
  ORBIT_NVP_VAL(0, m_SampleReport);
  ORBIT_NVP_VAL(0, m_ThreadUsage);
  ORBIT_NVP_VAL(0, m_AverageThreadUsage);
  ORBIT_NVP_VAL(0, m_TID);
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE_WSTRING(LineInfo, 1) {
  ORBIT_NVP_VAL(0, m_File);
  ORBIT_NVP_VAL(0, m_Line);
  ORBIT_NVP_VAL(0, m_Address);
  ORBIT_NVP_VAL(1, m_FileNameHash);
}
