// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
#include "TimerChain.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
ErrorMessageOr<void> CaptureSerializer::Save(const std::string& filename) {
  Capture::PreSave();

  // Binary
  m_CaptureName = filename;

  std::ofstream file(m_CaptureName, std::ios::binary);
  if (file.fail()) {
    ERROR("Saving capture in \"%s\": %s", filename, "file.fail()");
    return outcome::failure("Error opening the file for writing");
  }

  try {
    SCOPE_TIMER_LOG(absl::StrFormat("Saving capture in \"%s\"", filename));
    Save(file);
  } catch (std::exception& e) {
    ERROR("Saving capture in \"%s\": %s", filename, e.what());
    return outcome::failure("Error serializing the capture");
  }

  return outcome::success();
}

void CaptureSerializer::Save(std::ostream& stream) {
  cereal::BinaryOutputArchive archive(stream);
  std::basic_ostream<char> Stream(&GStreamCounter);
  cereal::BinaryOutputArchive CountingArchive(Stream);
  GStreamCounter.Reset();
  SaveImpl(CountingArchive);
  PRINT_VAR(GStreamCounter.Size());
  GStreamCounter.Reset();

  SaveImpl(archive);
}

//-----------------------------------------------------------------------------
template <class T>
void CaptureSerializer::SaveImpl(T& archive) {
  CHECK(time_graph_ != nullptr);
  m_NumTimers = time_graph_->GetNumTimers();

  // Header
  archive(cereal::make_nvp("Capture", *this));

  {
    ORBIT_SIZE_SCOPE("Functions");
    std::vector<Function> functions;
    for (auto& pair : Capture::GSelectedFunctionsMap) {
      Function* func = pair.second;
      if (func != nullptr) {
        functions.push_back(*func);
      }
    }

    archive(functions);
  }

  {
    ORBIT_SIZE_SCOPE("Capture::GFunctionCountMap");
    archive(Capture::GFunctionCountMap);
  }

  {
    ORBIT_SIZE_SCOPE("Capture::GCallstacks");
    archive(Capture::GCallstacks);
  }

  {
    ORBIT_SIZE_SCOPE("Capture::GProcessId");
    archive(Capture::GProcessId);
  }

  {
    ORBIT_SIZE_SCOPE("Capture::GProcessName");
    archive(Capture::GProcessName);
  }

  {
    ORBIT_SIZE_SCOPE("Capture::GThreadNames");
    archive(Capture::GThreadNames);
  }

  {
    ORBIT_SIZE_SCOPE("Capture::GAddressInfos");
    archive(Capture::GAddressInfos);
  }

  {
    ORBIT_SIZE_SCOPE("Capture::GAddressToFunctionName");
    archive(Capture::GAddressToFunctionName);
  }

  {
    ORBIT_SIZE_SCOPE("SamplingProfiler");
    archive(Capture::GSamplingProfiler);
  }

  {
    ORBIT_SIZE_SCOPE("String manager");
    archive(*time_graph_->GetStringManager());
  }

  {
    ORBIT_SIZE_SCOPE("Event Buffer");
    archive(GEventTracer.GetEventBuffer());
  }

  // Timers
  int numWrites = 0;
  std::vector<std::shared_ptr<TimerChain>> chains =
      time_graph_->GetAllTimerChains();
  for (auto& chain : chains) {
    if (!chain) continue;
    for (TimerChainIterator it = chain->begin(); it != chain->end(); ++it) {
      TimerBlock& block = *it;
      for (uint32_t k = 0; k < block.size(); ++k) {
        archive(cereal::binary_data(&(block[k].GetTimer()), sizeof(Timer)));

        if (++numWrites > m_NumTimers) {
          return;
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
ErrorMessageOr<void> CaptureSerializer::Load(const std::string& filename) {
  SCOPE_TIMER_LOG(absl::StrFormat("Loading capture from \"%s\"", filename));

  // Binary
  std::ifstream file(filename, std::ios::binary);
  if (file.fail()) {
    ERROR("Loading capture from \"%s\": %s", filename, "file.fail()");
    return outcome::failure("Error opening the file for reading");
  }

  try {
    return Load(file);
  } catch (std::exception& e) {
    ERROR("Loading capture from \"%s\": %s", filename, e.what());
    return outcome::failure("Error parsing the capture");
  }
}

ErrorMessageOr<void> CaptureSerializer::Load(std::istream& stream) {
  // Header
  cereal::BinaryInputArchive archive(stream);
  archive(*this);

  // Functions
  std::vector<Function> functions;
  archive(functions);
  Capture::GSelectedFunctions.clear();
  Capture::GSelectedFunctionsMap.clear();
  for (const auto& function : functions) {
    std::shared_ptr<Function> function_ptr =
        std::make_shared<Function>(function);
    Capture::GSelectedFunctions.push_back(function_ptr);
    Capture::GSelectedFunctionsMap[function_ptr->GetVirtualAddress()] =
        function_ptr.get();
  }
  Capture::GVisibleFunctionsMap = Capture::GSelectedFunctionsMap;

  archive(Capture::GFunctionCountMap);

  archive(Capture::GCallstacks);

  archive(Capture::GProcessId);

  archive(Capture::GProcessName);

  archive(Capture::GThreadNames);

  archive(Capture::GAddressInfos);

  archive(Capture::GAddressToFunctionName);

  archive(Capture::GSamplingProfiler);
  if (Capture::GSamplingProfiler == nullptr) {
    Capture::GSamplingProfiler = std::make_shared<SamplingProfiler>();
  }
  Capture::GSamplingProfiler->SortByThreadUsage();

  time_graph_->Clear();

  archive(*time_graph_->GetStringManager());

  // Event buffer
  archive(GEventTracer.GetEventBuffer());

  // Timers
  Timer timer;
  while (stream.read(reinterpret_cast<char*>(&timer), sizeof(Timer))) {
    time_graph_->ProcessTimer(timer);
  }

  Capture::GState = Capture::State::kDone;

  GOrbitApp->AddSamplingReport(Capture::GSamplingProfiler);
  GOrbitApp->FireRefreshCallbacks();
  return outcome::success();
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(CaptureSerializer, 0) {
  ORBIT_NVP_VAL(0, m_CaptureName);
  ORBIT_NVP_VAL(0, m_NumTimers);
}
