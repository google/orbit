// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <libfuzzer/libfuzzer_macro.h>

#include <cstdio>
#include <filesystem>

#include "App.h"
#include "OrbitClientModel/CaptureDeserializer.h"
#include "OrbitClientModel/CaptureSerializer.h"
#include "TimeGraph.h"
#include "absl/flags/flag.h"
#include "capture_data.pb.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"

// Hack: This is declared in a header we include here
// and the definition needs to take place somewhere.
ABSL_FLAG(bool, enable_stale_features, false,
          "Enable obsolete features that are not working or are not "
          "implemented in the client's UI");
ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");
ABSL_FLAG(bool, local, false, "Connects to local instance of OrbitService");
ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");
ABSL_FLAG(bool, enable_frame_pointer_validator, false, "Enable validation of frame pointers");
ABSL_FLAG(bool, show_return_values, false, "Show return values on time slices");
ABSL_FLAG(bool, enable_tracepoint_feature, false,
          "Enable the setting of the panel of kernel tracepoints");
ABSL_FLAG(bool, thread_state, false, "Collect thread states");

DEFINE_PROTO_FUZZER(const orbit_client_protos::CaptureDeserializerFuzzerInfo& info) {
  std::string buffer{};
  {
    google::protobuf::io::StringOutputStream stream{&buffer};
    google::protobuf::io::CodedOutputStream coded_stream{&stream};
    orbit_client_protos::CaptureHeader capture_header{};
    capture_header.set_version("1.55");

    capture_serializer::WriteMessage(&capture_header, &coded_stream);
    capture_serializer::WriteMessage(&info.capture_info(), &coded_stream);
    for (const auto& timer : info.timers()) {
      capture_serializer::WriteMessage(&timer, &coded_stream);
    }
  }

  std::unique_ptr<OrbitApp> app = OrbitApp::Create(nullptr);

  app->SetCaptureStartedCallback([]() {});
  app->SetCaptureStoppedCallback([]() {});
  app->SetCaptureFailedCallback([]() {});
  app->SetCaptureClearedCallback([]() {});
  app->SetOpenCaptureCallback([]() {});
  app->SetOpenCaptureFailedCallback([]() {});
  app->SetOpenCaptureFinishedCallback([]() {});
  app->SetSelectLiveTabCallback([]() {});
  app->SetErrorMessageCallback([](const std::string& /*title*/, const std::string& /*text*/) {});
  app->SetRefreshCallback([](DataViewType /*type*/) {});
  app->SetSamplingReportCallback(
      [](DataView* /*view*/, std::shared_ptr<SamplingReport> /*report*/) {});
  app->SetTopDownViewCallback([](std::unique_ptr<CallTreeView> /*view*/) {});
  app->SetBottomUpViewCallback([](std::unique_ptr<CallTreeView> /*view*/) {});

  TimeGraph time_graph{14, app.get()};
  GCurrentTimeGraph = &time_graph;
  auto string_manager = std::make_shared<StringManager>();
  time_graph.SetStringManager(string_manager);
  app->ClearCapture();

  // NOLINTNEXTLINE
  std::istringstream input_stream{std::move(buffer)};
  std::atomic<bool> cancellation_requested = false;

  orbit_client_data::ModuleManager module_manager;
  capture_deserializer::Load(input_stream, "", app.get(), &module_manager, &cancellation_requested);

  app->GetThreadPool()->ShutdownAndWait();
}
