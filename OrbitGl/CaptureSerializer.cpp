// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureSerializer.h"

#include <fstream>
#include <memory>

#include "App.h"
#include "Capture.h"
#include "Core.h"
#include "EventTracer.h"
#include "FunctionUtils.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Pdb.h"
#include "PrintVar.h"
#include "SamplingProfiler.h"
#include "ScopeTimer.h"
#include "TextBox.h"
#include "TimeGraph.h"
#include "TimerChain.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
ErrorMessageOr<void> CaptureSerializer::Save(const std::string& filename) {
  Capture::PreSave();

  // Binary
  header.set_capture_name(filename);
  header.set_version(CAPTURE_VERSION);

  std::ofstream file(header.capture_name(), std::ios::binary);
  if (file.fail()) {
    ERROR("Saving capture in \"%s\": %s", filename, "file.fail()");
    return ErrorMessage("Error opening the file for writing");
  }

  try {
    SCOPE_TIMER_LOG(absl::StrFormat("Saving capture in \"%s\"", filename));
    Save(file);
  } catch (std::exception& e) {
    ERROR("Saving capture in \"%s\": %s", filename, e.what());
    return ErrorMessage("Error serializing the capture");
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
  header.set_num_timers(time_graph_->GetNumTimers());

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

        if (++numWrites > header.num_timers()) {
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
    return ErrorMessage("Error opening the file for reading");
  }

  try {
    return Load(file);
  } catch (std::exception& e) {
    ERROR("Loading capture from \"%s\": %s", filename, e.what());
    return ErrorMessage("Error parsing the capture");
  }
}

ErrorMessageOr<void> CaptureSerializer::Load(std::istream& stream) {
  // Header
  cereal::BinaryInputArchive archive(stream);

  time_graph_->Clear();

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
