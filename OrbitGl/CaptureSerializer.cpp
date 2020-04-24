#include "CaptureSerializer.h"

#include <fstream>
#include <memory>

#include "App.h"
#include "Callstack.h"
#include "Capture.h"
#include "Core.h"
#include "EventTracer.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Pdb.h"
#include "PrintVar.h"
#include "SamplingProfiler.h"
#include "ScopeTimer.h"
#include "Serialization.h"
#include "TextBox.h"
#include "TimeGraph.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
CaptureSerializer::CaptureSerializer() {
  m_Version = 2;
  m_TimerVersion = Timer::Version;
  m_SizeOfTimer = sizeof(Timer);
}

//-----------------------------------------------------------------------------
void CaptureSerializer::Save(const std::wstring& a_FileName) {
  Capture::PreSave();

  std::basic_ostream<char> Stream(&GStreamCounter);
  cereal::BinaryOutputArchive CountingArchive(Stream);
  GStreamCounter.Reset();
  Save(CountingArchive);
  PRINT_VAR(GStreamCounter.Size());
  GStreamCounter.Reset();

  // Binary
  m_CaptureName = ws2s(a_FileName);
  std::ofstream myfile(m_CaptureName, std::ios::binary);
  if (!myfile.fail()) {
    SCOPE_TIMER_LOG(
        absl::StrFormat("Saving capture in %s", ws2s(a_FileName).c_str()));
    cereal::BinaryOutputArchive archive(myfile);
    Save(archive);
    myfile.close();
  }
}

//-----------------------------------------------------------------------------
template <class T>
void CaptureSerializer::Save(T& a_Archive) {
  m_NumTimers = time_graph_->GetNumTimers();

  // Header
  a_Archive(cereal::make_nvp("Capture", *this));

  // Functions
  {
    ORBIT_SIZE_SCOPE("Functions");
    std::vector<Function> functions;
    for (auto& pair : Capture::GSelectedFunctionsMap) {
      Function* func = pair.second;
      if (func) {
        functions.push_back(*func);
        functions.back().SetAddress(func->Address());
      }
    }

    a_Archive(functions);
  }

  // Function Count
  a_Archive(Capture::GFunctionCountMap);

  // Process
  {
    ORBIT_SIZE_SCOPE("Capture::GTargetProcess");
    a_Archive(Capture::GTargetProcess);
  }

  // Callstacks
  {
    ORBIT_SIZE_SCOPE("Capture::GCallstacks");
    a_Archive(Capture::GCallstacks);
  }

  // Sampling profiler
  {
    ORBIT_SIZE_SCOPE("SamplingProfiler");
    a_Archive(Capture::GSamplingProfiler);
  }

  // Event buffer
  {
    ORBIT_SIZE_SCOPE("Event Buffer");
    a_Archive(GEventTracer.GetEventBuffer());
  }

  // Timers
  int numWrites = 0;
  std::vector<std::shared_ptr<TimerChain>> chains =
      time_graph_->GetAllTimerChains();
  for (const std::shared_ptr<TimerChain>& chain : chains) {
    for (const TextBox& box : *chain) {
      a_Archive(cereal::binary_data((char*)&box.GetTimer(), sizeof(Timer)));

      if (++numWrites > m_NumTimers) {
        return;
      }
    }
  }
}

//-----------------------------------------------------------------------------
void CaptureSerializer::Load(const std::wstring& a_FileName) {
  SCOPE_TIMER_LOG(
      absl::StrFormat("Loading capture %s", ws2s(a_FileName).c_str()));

#ifdef _WIN32
  // Binary
  std::ifstream file(ws2s(a_FileName), std::ios::binary);
  if (!file.fail()) {
    // header
    cereal::BinaryInputArchive archive(file);
    archive(*this);

    // functions
    std::shared_ptr<Module> module = std::make_shared<Module>();
    Capture::GTargetProcess->AddModule(module);
    module->m_Pdb = std::make_shared<Pdb>(ws2s(a_FileName).c_str());
    std::vector<std::shared_ptr<Function>> functions =
        module->m_Pdb->GetFunctions();
    archive(functions);
    module->m_Pdb->ProcessData();
    GPdbDbg = module->m_Pdb;
    Capture::GSelectedFunctionsMap.clear();
    for (auto& func : module->m_Pdb->GetFunctions()) {
      Capture::GSelectedFunctionsMap[func->GetVirtualAddress()] = func.get();
    }
    Capture::GVisibleFunctionsMap = Capture::GSelectedFunctionsMap;

    // Function count
    archive(Capture::GFunctionCountMap);

    // Process
    archive(Capture::GTargetProcess);

    // Callstacks
    archive(Capture::GCallstacks);

    // Sampling profiler
    archive(Capture::GSamplingProfiler);
    Capture::GSamplingProfiler->SortByThreadUsage();
    GOrbitApp->AddSamplingReport(Capture::GSamplingProfiler, GOrbitApp.get());
    Capture::GSamplingProfiler->SetLoadedFromFile(true);

    // Event buffer
    archive(GEventTracer.GetEventBuffer());

    // Timers
    Timer timer;
    while (file.read((char*)&timer, sizeof(Timer))) {
      time_graph_->ProcessTimer(timer);
    }

    GOrbitApp->FireRefreshCallbacks();
  }
#endif
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(CaptureSerializer, 0) {
  ORBIT_NVP_VAL(0, m_CaptureName);
  ORBIT_NVP_VAL(0, m_Version);
  ORBIT_NVP_VAL(0, m_TimerVersion);
  ORBIT_NVP_VAL(0, m_NumTimers);
  ORBIT_NVP_VAL(0, m_SizeOfTimer);
}
