//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "BpfTrace.h"

#include <fstream>
#include <sstream>

#include "Capture.h"
#include "CoreApp.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Params.h"
#include "ScopeTimer.h"
#include "Utils.h"

#if __linux__
#include <linux/perf_event.h>
#include <linux/version.h>

#include <map>
#include <vector>

#include "BpfTraceVisitor.h"
#include "LinuxPerfEvent.h"
#include "LinuxPerfEventProcessor.h"
#include "LinuxPerfRingBuffer.h"
#include "LinuxPerfUtils.h"
#include "LinuxUtils.h"
#include "OrbitFunction.h"
#include "Pdb.h"

using namespace LinuxPerfUtils;
#endif

//-----------------------------------------------------------------------------
BpfTrace::BpfTrace(Callback a_Callback) {
#if __linux__

  // Until perf_events are fixed...
  GParams.m_UseBpftrace = true;

  // TODO: we shouldn't hijack the BpfTrace class and move perf_event related
  //       code to its own class.

  // Uprobe perf_events not supported until Kernel 4.17
  if (LinuxPerfUtils::supports_perf_event_uprobes()) {
    m_UsePerfEvents = !GParams.m_UseBpftrace && !GParams.m_BpftraceCallstacks;
  } else {
    m_UsePerfEvents = false;
  }

  PRINT_VAR(m_UsePerfEvents);

  m_Callback = a_Callback ? a_Callback : [this](const std::string& a_Buffer) {
    if (GParams.m_BpftraceCallstacks)
      CommandCallbackWithCallstacks(a_Buffer);
    else
      CommandCallback(a_Buffer);
  };

  m_ScriptFileName = ws2s(Path::GetBasePath()) + "orbit.bt";
#endif
}

//-----------------------------------------------------------------------------
void BpfTrace::Start() {
#if __linux__
  m_ExitRequested = false;
  m_TimerStacks.clear();

  if (!WriteBpfScript()) return;

  m_BpfCommand = std::string("bpftrace ") + m_ScriptFileName;

  if (!m_UsePerfEvents) {
    m_Thread = std::make_shared<std::thread>(&LinuxUtils::StreamCommandOutput,
                                             m_BpfCommand.c_str(), m_Callback,
                                             &m_ExitRequested);
  } else {
    m_Thread = std::make_shared<std::thread>(BpfTrace::RunPerfEventOpen,
                                             &m_ExitRequested);
  }

  m_Thread->detach();
#endif
}

//-----------------------------------------------------------------------------
void BpfTrace::Stop() { m_ExitRequested = true; }

//-----------------------------------------------------------------------------
std::string BpfTrace::GetBpfScript() {
  if (!m_Script.empty()) {
    return m_Script;
  }

  std::stringstream ss;

  for (Function* func : Capture::GTargetProcess->GetFunctions()) {
    if (func->IsSelected()) {
      uint64_t virtual_address = (uint64_t)func->GetVirtualAddress();
      Capture::GSelectedFunctionsMap[func->m_Address] = func;

      if (GParams.m_BpftraceCallstacks) {
        ss << "   uprobe:" << func->m_Probe << R"({ printf("b )"
           << std::to_string(virtual_address)
           << R"( %u %lld\n%s\n\nd\n\n", tid, nsecs, ustack(perf)); })"
           << std::endl;
      } else {
        ss << "   uprobe:" << func->m_Probe << R"({ printf("b )"
           << std::to_string(virtual_address)
           << R"( %u %lld\n", tid, nsecs); })" << std::endl;
      }

      ss << "uretprobe:" << func->m_Probe << R"({ printf("e )"
         << std::to_string(virtual_address) << R"( %u %lld\n", tid, nsecs); })"
         << std::endl;
    }
  }

  return ss.str();
}

//-----------------------------------------------------------------------------
bool BpfTrace::WriteBpfScript() {
  std::string script = GetBpfScript();
  if (script.empty()) return false;

  std::ofstream outFile;
  outFile.open(m_ScriptFileName);
  if (outFile.fail()) return false;

  outFile << script;
  outFile.close();
  return true;
}

//-----------------------------------------------------------------------------
uint64_t BpfTrace::ProcessString(const std::string& a_String) {
  auto hash = StringHash(a_String);
  if (m_StringMap.find(hash) == m_StringMap.end()) {
    m_StringMap[hash] = a_String;
  }

  return hash;
}

//-----------------------------------------------------------------------------
void BpfTrace::CommandCallback(const std::string& a_Line) {
  auto tokens = Tokenize(a_Line);
  const std::string& mode = tokens[0];
  const std::string& functionAddress = tokens[1];
  const std::string& threadName = tokens[2];
  const std::string& timestamp = tokens[3];

  bool isBegin = mode == "b";
  bool isEnd = !isBegin;

  if (isBegin) {
    Timer timer;
    timer.m_TID = atoi(threadName.c_str());
    uint64_t nanos = std::stoull(timestamp);
    timer.m_Start = nanos;
    timer.m_Depth = (uint8_t)m_TimerStacks[threadName].size();
    timer.m_FunctionAddress = std::stoull(functionAddress);
    m_TimerStacks[threadName].push_back(timer);
  }

  if (isEnd) {
    std::vector<Timer>& timers = m_TimerStacks[threadName];
    if (timers.size()) {
      Timer& timer = timers.back();
      uint64_t nanos = std::stoull(timestamp);
      timer.m_End = nanos;
      GCoreApp->ProcessTimer(&timer, functionAddress);
      timers.pop_back();
    }
  }
}

//-----------------------------------------------------------------------------
void BpfTrace::CommandCallbackWithCallstacks(const std::string& a_Line) {
  if (a_Line.empty() || a_Line == "\n") return;

  auto tokens = Tokenize(a_Line);

  const std::string& mode = tokens[0];
  bool isBegin = mode == "b";
  bool isEnd = mode == "e";

  bool isStackLine = StartsWith(a_Line, "\t");
  bool isEndOfStack = a_Line == "d\n";

  if (!isBegin && !isEnd && !isStackLine && !isEndOfStack) {
    if (StartsWith(a_Line, "Lost")) {
      PRINT(a_Line.c_str());
      return;
    }

    if (StartsWith(a_Line, "Attaching")) return;

    // if the line does not start with one of the above,
    // we might have a broken line, e.g. due to a small buffer
    PRINT("read unexpected line:%s\nthe buffer might be to small.",
          a_Line.c_str());
    return;
  }

  if (isStackLine) {
    const std::string& addressStr = LTrim(tokens[0]);
    uint64_t address = std::stoull(addressStr, nullptr, 16);

    std::string function = tokens[1];

    std::string moduleRaw = tokens[2];
    std::string module = Replace(moduleRaw.substr(1), ")\n", "");

    // TODO: this is copy&paste from LinuxPerf.cpp
    std::wstring moduleName = ToLower(Path::GetFileName(s2ws(module)));
    std::shared_ptr<Module> moduleFromName =
        Capture::GTargetProcess->GetModuleFromName(ws2s(moduleName));

    if (moduleFromName) {
      uint64_t new_address = moduleFromName->ValidateAddress(address);
      address = new_address;
    }

    m_CallStack.m_Data.push_back(address);
    if (Capture::GTargetProcess &&
        !Capture::GTargetProcess->HasSymbol(address)) {
      GCoreApp->AddSymbol(address, module, function);
    }

    return;
  }

  if (isEndOfStack) {
    if (m_CallStack.m_Data.size()) {
      m_CallStack.m_Depth = (uint32_t)m_CallStack.m_Data.size();
      m_CallStack.m_ThreadId = atoi(m_LastThreadName.c_str());
      std::vector<Timer>& timers = m_TimerStacks[m_LastThreadName];
      if (timers.size()) {
        Timer& timer = timers.back();
        timer.m_CallstackHash = m_CallStack.Hash();
        GCoreApp->ProcessCallStack(m_CallStack);
      }
    }

    m_CallStack.Clear();
    m_LastThreadName = "";
    return;
  }

  const std::string& functionAddress = tokens[1];
  const std::string& threadName = tokens[2];
  const std::string& timestamp = tokens[3];

  m_LastThreadName = threadName;

  if (isBegin) {
    Timer timer;
    timer.m_TID = atoi(threadName.c_str());
    uint64_t nanos = std::stoull(timestamp);
    timer.m_Start = nanos;
    timer.m_Depth = (uint8_t)m_TimerStacks[threadName].size();
    timer.m_FunctionAddress = std::stoull(functionAddress);
    m_TimerStacks[threadName].push_back(timer);
    return;
  }

  if (isEnd) {
    std::vector<Timer>& timers = m_TimerStacks[threadName];
    if (timers.size()) {
      Timer& timer = timers.back();
      uint64_t nanos = std::stoull(timestamp);
      timer.m_End = nanos;
      GCoreApp->ProcessTimer(&timer, functionAddress);
      timers.pop_back();
    }
    return;
  }
}

//-----------------------------------------------------------------------------
void BpfTrace::RunPerfEventOpen(bool* a_ExitRequested) {
#if __linux__
  SCOPE_TIMER_FUNC;
  int ROUND_ROBIN_BATCH_SIZE = 5;
  std::vector<uint64_t> fds;
  std::map<Function*, LinuxPerfRingBuffer> uprobe_ring_buffers;
  std::map<Function*, LinuxPerfRingBuffer> uretprobe_ring_buffers;

  for (const auto& pair : Capture::GSelectedFunctionsMap) {
    // gather function information
    Function* function = pair.second;
    uint64_t offset = function->m_Address;
    std::string module = ws2s(function->m_Pdb->GetFileName());

    // create uprobe for that function on that PID profiling all cpus
    uint64_t uprobe_fd = 0;
    if (GParams.m_BpftraceCallstacks)
      uprobe_fd = LinuxPerfUtils::uprobe_event_open(
          module.c_str(), offset, Capture::GTargetProcess->GetID(), -1,
          PERF_SAMPLE_STACK_USER | PERF_SAMPLE_REGS_USER);
    else
      uprobe_fd = LinuxPerfUtils::uprobe_event_open(
          module.c_str(), offset, Capture::GTargetProcess->GetID(), -1);

    uprobe_ring_buffers.emplace(function, uprobe_fd);
    fds.push_back(uprobe_fd);

    // create uretprobe for that function on that PID profiling all cpus
    uint64_t uretprobe_fd = LinuxPerfUtils::uretprobe_event_open(
        module.c_str(), offset, Capture::GTargetProcess->GetID(), -1);
    uretprobe_ring_buffers.emplace(function, uretprobe_fd);
    fds.push_back(uretprobe_fd);

    // start capturing
    LinuxPerfUtils::start_capturing(uprobe_fd);
    LinuxPerfUtils::start_capturing(uretprobe_fd);
  }

  LinuxPerfEventProcessor event_buffer(std::make_unique<BpfTraceVisitor>());

  bool new_events = false;

  while (!(*a_ExitRequested)) {
    // Lets sleep a bit, such that we are not constantly reading from the
    // buffers and thus wasting cpu time. 10000 microseconds are still small
    // enough to not have our buffers overflown and therefore losing events.
    if (!new_events) {
      usleep(10000);
    }

    new_events = false;

    // read from all ring buffers, create events and store them in the
    // event_queue
    // TODO: implement a better scheduling strategy.
    for (auto& pair : uprobe_ring_buffers) {
      auto& ring_buffer = pair.second;
      int i = 0;
      // read everything that is new
      while (ring_buffer.HasNewData() && i < ROUND_ROBIN_BATCH_SIZE) {
        i++;
        new_events = true;
        perf_event_header header{};
        ring_buffer.ReadHeader(&header);

        // perf_event_header::type contains the type of record,
        // defined in enum perf_event_type in perf_event.h.
        switch (header.type) {
          case PERF_RECORD_SAMPLE: {
            SCOPE_TIMER_LOG("PERF_RECORD_SAMPLE");
            if (GParams.m_BpftraceCallstacks) {
              auto record =
                  ring_buffer.ConsumeRecord<LinuxUprobeEventWithStack>(header);
              record.SetFunction(pair.first);
              event_buffer.Push(std::make_unique<LinuxUprobeEventWithStack>(
                  std::move(record)));
            } else {
              auto record = ring_buffer.ConsumeRecord<LinuxUprobeEvent>(header);
              record.SetFunction(pair.first);
              event_buffer.Push(
                  std::make_unique<LinuxUprobeEvent>(std::move(record)));
            }

          } break;
          case PERF_RECORD_LOST: {
            auto lost = ring_buffer.ConsumeRecord<LinuxPerfLostEvent>(header);
            PRINT("Lost %u Events\n", lost.Lost());
          } break;
          default:
            PRINT("Unexpected Perf Sample Type: %u", header.type);
            ring_buffer.SkipRecord(header);
            break;
        }
      }
    }

    for (auto& pair : uretprobe_ring_buffers) {
      auto& ring_buffer = pair.second;
      int i = 0;
      // read everything that is new
      while (ring_buffer.HasNewData() && i < ROUND_ROBIN_BATCH_SIZE) {
        i++;
        new_events = true;
        perf_event_header header{};
        ring_buffer.ReadHeader(&header);

        // perf_event_header::type contains the type of record,
        // defined in enum perf_event_type in perf_event.h.
        switch (header.type) {
          case PERF_RECORD_SAMPLE: {
            PRINT("Consuming uretprobe record...");
            auto record =
                ring_buffer.ConsumeRecord<LinuxUretprobeEvent>(header);
            record.SetFunction(pair.first);
            event_buffer.Push(
                std::make_unique<LinuxUretprobeEvent>(std::move(record)));
          } break;
          case PERF_RECORD_LOST: {
            auto lost = ring_buffer.ConsumeRecord<LinuxPerfLostEvent>(header);
            PRINT("Lost %u Events\n", lost.Lost());
          } break;
          default:
            PRINT("Unexpected Perf Sample Type: %u", header.type);
            ring_buffer.SkipRecord(header);
            break;
        }
      }
    }

    event_buffer.ProcessTillOffset();
  }

  for (auto fd : fds) {
    LinuxPerfUtils::stop_capturing(fd);
  }

  event_buffer.ProcessAll();
#endif
}

//-----------------------------------------------------------------------------
void BpfTrace::RunPerfEventOpenSingleBuffers(bool* a_ExitRequested) {
#if __linux__
  SCOPE_TIMER_FUNC;

  int ROUND_ROBIN_BATCH_SIZE = 5;
  std::vector<uint64_t> fds;
  std::shared_ptr<LinuxPerfRingBuffer> uprobe_ring_buffer;
  std::shared_ptr<LinuxPerfRingBuffer> uretprobe_ring_buffer;
  uint64_t sampleType = GParams.m_BpftraceCallstacks
                            ? (PERF_SAMPLE_STACK_USER | PERF_SAMPLE_REGS_USER)
                            : 0;

  int32_t master_uprobe_fd = -1;
  int32_t master_uretprobe_fd = -1;
  Function* dummyFunction = nullptr;

  // Functions to instument.
  std::set<Function*> selectedFunctions;
  for (const auto& pair : Capture::GSelectedFunctionsMap) {
    selectedFunctions.insert(pair.second);
    dummyFunction = pair.second;
  }

  for (Function* function : selectedFunctions) {
    // gather function information
    uint64_t offset = function->m_Address;
    std::string module = ws2s(function->m_Pdb->GetFileName());
    PRINT_VAR(offset);
    PRINT_VAR(module);

    // create uprobe for that function on that PID profiling all cpus
    int32_t uprobe_fd = LinuxPerfUtils::uprobe_event_open(
        module.c_str(), offset, Capture::GTargetProcess->GetID(), -1,
        master_uprobe_fd, sampleType);

    // create uretprobe for that function on that PID profiling all cpus
    int32_t uretprobe_fd = LinuxPerfUtils::uretprobe_event_open(
        module.c_str(), offset, Capture::GTargetProcess->GetID(), -1, -1);

    // Check that both uprobe and uretprobe were created successfully
    if (uprobe_fd == -1 || uretprobe_fd == -1) {
      PRINT("Uprobe/Uretprobe error. uprobe_fd:%i uretprobe_fd:%i", uprobe_fd,
            uretprobe_fd);
      continue;
    }

    if (master_uprobe_fd == -1) {
      master_uprobe_fd = uprobe_fd;
      master_uretprobe_fd = uretprobe_fd;
      uprobe_ring_buffer = std::make_shared<LinuxPerfRingBuffer>(uprobe_fd);
      uretprobe_ring_buffer =
          std::make_shared<LinuxPerfRingBuffer>(uretprobe_fd);
    } else {
      // Redirect events to master event buffer
      int ret = ioctl(uprobe_fd, PERF_EVENT_IOC_SET_OUTPUT, master_uprobe_fd);
      if (ret) PRINT("PERF_EVENT_IOC_SET_OUTPUT error: %i.\n", ret);
      ret = ioctl(uretprobe_fd, PERF_EVENT_IOC_SET_OUTPUT, master_uretprobe_fd);
      if (ret) PRINT("PERF_EVENT_IOC_SET_OUTPUT error: %i.\n", ret);
    }

    fds.push_back(uprobe_fd);
    fds.push_back(uretprobe_fd);

    PRINT_VAR(uprobe_fd);
    PRINT_VAR(uretprobe_fd);
  }

  // start capturing
  for (auto fd : fds) {
    LinuxPerfUtils::start_capturing(fd);
  }

  LinuxPerfEventProcessor event_buffer(std::make_unique<BpfTraceVisitor>());

  bool new_events = false;

  while (!(*a_ExitRequested)) {
    // Lets sleep a bit, such that we are not constantly reading from the
    // buffers and thus wasting cpu time. 10000 microseconds are still small
    // enough to not have our buffers overflown and therefore losing events.
    if (!new_events) {
      usleep(10000);
    }

    new_events = false;

    // read from all ring buffers, create events and store them in the
    // event_queue
    // TODO: implement a better scheduling strategy.

    // uprobes
    {
      auto ring_buffer = uprobe_ring_buffer;
      int i = 0;

      // read everything that is new
      while (ring_buffer->HasNewData() && i < ROUND_ROBIN_BATCH_SIZE) {
        i++;
        new_events = true;
        perf_event_header header{};
        ring_buffer->ReadHeader(&header);

        // perf_event_header::type contains the type of record,
        // defined in enum perf_event_type in perf_event.h.
        switch (header.type) {
          case PERF_RECORD_SAMPLE: {
            PRINT("Consuming uprobe buffer\n");
            if (GParams.m_BpftraceCallstacks) {
              auto record =
                  ring_buffer->ConsumeRecord<LinuxUprobeEventWithStack>(header);
              record.SetFunction(dummyFunction);
              event_buffer.Push(std::make_unique<LinuxUprobeEventWithStack>(
                  std::move(record)));
            } else {
              auto record =
                  ring_buffer->ConsumeRecord<LinuxUprobeEvent>(header);
              record.SetFunction(dummyFunction);
              event_buffer.Push(
                  std::make_unique<LinuxUprobeEvent>(std::move(record)));
            }

          } break;
          case PERF_RECORD_LOST: {
            auto lost = ring_buffer->ConsumeRecord<LinuxPerfLostEvent>(header);
            PRINT("Lost %u Events\n", lost.Lost());
          } break;
          default:
            PRINT("Unexpected Perf Sample Type: %u", header.type);
            ring_buffer->SkipRecord(header);
            break;
        }
      }
    }

    // uretprobes
    {
      auto& ring_buffer = uretprobe_ring_buffer;
      int i = 0;
      // read everything that is new
      while (ring_buffer->HasNewData() && i < ROUND_ROBIN_BATCH_SIZE) {
        i++;
        new_events = true;
        perf_event_header header{};
        ring_buffer->ReadHeader(&header);

        // perf_event_header::type contains the type of record,
        // defined in enum perf_event_type in perf_event.h.
        switch (header.type) {
          case PERF_RECORD_SAMPLE: {
            // PRINT("Consuming uretprobe record...");
            auto record =
                ring_buffer->ConsumeRecord<LinuxUretprobeEvent>(header);
            record.SetFunction(dummyFunction);
            event_buffer.Push(
                std::make_unique<LinuxUretprobeEvent>(std::move(record)));
          } break;
          case PERF_RECORD_LOST: {
            auto lost = ring_buffer->ConsumeRecord<LinuxPerfLostEvent>(header);
            PRINT("Lost %u Events\n", lost.Lost());
          } break;
          default:
            PRINT("Unexpected Perf Sample Type: %u", header.type);
            ring_buffer->SkipRecord(header);
            break;
        }
      }
    }

    event_buffer.ProcessTillOffset();
  }

  for (auto fd : fds) {
    LinuxPerfUtils::stop_capturing(fd);
  }

  event_buffer.ProcessAll();
#endif
}
