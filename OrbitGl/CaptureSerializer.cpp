// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureSerializer.h"

#include <fstream>
#include <memory>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>

#include "App.h"
#include "Capture.h"
#include "Core.h"
#include "EventTracer.h"
#include "FunctionUtils.h"
#include "OrbitBase/Logging.h"
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
  std::basic_ostream<char> Stream(&GStreamCounter);
  google::protobuf::io::OstreamOutputStream out_counting_stream(&Stream);
  google::protobuf::io::CodedOutputStream coded_counting_output(
      &out_counting_stream);

  GStreamCounter.Reset();
  SaveImpl(&coded_counting_output);
  PRINT_VAR(GStreamCounter.Size());
  GStreamCounter.Reset();

  google::protobuf::io::OstreamOutputStream out_stream(&stream);
  google::protobuf::io::CodedOutputStream coded_output(&out_stream);
  SaveImpl(&coded_output);
}

void WriteMessage(const google::protobuf::Message* message,
                  google::protobuf::io::CodedOutputStream* output) {
  uint32_t message_size = message->ByteSizeLong();
  output->WriteLittleEndian32(message_size);
  message->SerializeToCodedStream(output);
}

//-----------------------------------------------------------------------------
void CaptureSerializer::SaveImpl(
    google::protobuf::io::CodedOutputStream* output) {
  CHECK(time_graph_ != nullptr);

  header.set_num_timers(time_graph_->GetNumTimers());

  WriteMessage(&header, output);

  {
    ORBIT_SIZE_SCOPE("Functions");
    FunctionList functions;
    for (auto& pair : Capture::GSelectedFunctionsMap) {
      Function* func = pair.second;
      if (func != nullptr) {
        functions.add_data()->CopyFrom(*pair.second);
      }
    }
    WriteMessage(&functions, output);
  }

  {
    ORBIT_SIZE_SCOPE("Capture::GData");
    WriteMessage(&Capture::GData, output);
  }

  {
    ORBIT_SIZE_SCOPE("SamplingProfiler");
    WriteMessage(Capture::GSamplingProfiler->GetData().get(), output);
  }

  {
    ORBIT_SIZE_SCOPE("String manager");
    WriteMessage(time_graph_->GetStringManager()->GetData().get(), output);
  }

  {
    ORBIT_SIZE_SCOPE("Event Buffer");
    WriteMessage(GEventTracer.GetEventBuffer().GetData().get(), output);
  }

  // Timers
  uint32_t num_writes = 0;
  std::vector<std::shared_ptr<TimerChain>> chains =
      time_graph_->GetAllTimerChains();
  for (auto& chain : chains) {
    if (!chain) continue;
    for (TimerChainIterator it = chain->begin(); it != chain->end(); ++it) {
      TimerBlock& block = *it;
      for (uint32_t k = 0; k < block.size(); ++k) {
        WriteMessage(&block[k].GetTimerData(), output);
        if (++num_writes >= header.num_timers()) {
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
    return ErrorMessage(
        "Error parsing the capture\nNote: If the capture "
        "was taken in a previous Orbit version, it could be incompatible. "
        "Please check release notes for more information.");
  }
}

void ReadMessage(google::protobuf::Message* message,
                 google::protobuf::io::CodedInputStream* input) {
  uint32_t message_size;
  input->ReadLittleEndian32(&message_size);

  char* buffer = new char[message_size];
  input->ReadRaw(buffer, message_size);
  message->ParseFromArray(buffer, message_size);
  delete[] buffer;
}

ErrorMessageOr<void> CaptureSerializer::Load(std::istream& stream) {
  google::protobuf::io::IstreamInputStream input_stream(&stream);
  google::protobuf::io::CodedInputStream coded_input(&input_stream);

  ReadMessage(&header, &coded_input);
  if (header.version() < MIN_CAPTURE_VERSION) {
    std::string error_message =
        "Capture was taken in a previous Orbit version and incompatible with "
        "the current Orbit version.";
    ERROR("%s", error_message);
    return ErrorMessage(error_message);
  }

  // Functions
  FunctionList functions;
  ReadMessage(&functions, &coded_input);
  Capture::GSelectedFunctions.clear();
  Capture::GSelectedFunctionsMap.clear();
  for (const auto& function : functions.data()) {
    std::shared_ptr<Function> function_ptr =
        std::make_shared<Function>(function);
    Capture::GSelectedFunctions.push_back(function_ptr);
    Capture::GSelectedFunctionsMap[FunctionUtils::GetAbsoluteAddress(
        *function_ptr)] = function_ptr.get();
  }
  Capture::GVisibleFunctionsMap = Capture::GSelectedFunctionsMap;

  ReadMessage(&Capture::GData, &coded_input);

  // SamplingProfiler
  auto profiler_data = std::make_unique<SamplingProfilerData>();
  ReadMessage(profiler_data.get(), &coded_input);
  Capture::GSamplingProfiler =
      std::make_shared<SamplingProfiler>(std::move(profiler_data));
  Capture::GSamplingProfiler->SortByThreadUsage();

  time_graph_->Clear();

  // StringManager
  auto string_manager_data = std::make_unique<Uint64ToString>();
  ReadMessage(string_manager_data.get(), &coded_input);
  time_graph_->GetStringManager()->SetData(std::move(string_manager_data));

  // EventBuffer
  auto event_buffer_data = std::make_unique<EventBufferData>();
  ReadMessage(event_buffer_data.get(), &coded_input);
  GEventTracer.GetEventBuffer().SetData(std::move(event_buffer_data));

  // Timers
  TimerData timer_data;
  for (uint32_t i = 0; i < header.num_timers(); ++i) {
    ReadMessage(&timer_data, &coded_input);
    time_graph_->ProcessTimer(timer_data);
  }

  Capture::GState = Capture::State::kDone;

  GOrbitApp->AddSamplingReport(Capture::GSamplingProfiler);
  GOrbitApp->FireRefreshCallbacks();
  return outcome::success();
}
