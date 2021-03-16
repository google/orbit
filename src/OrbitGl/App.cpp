// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "App.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <imgui.h>

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <outcome.hpp>
#include <ratio>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

#include "CallStackDataView.h"
#include "CaptureWindow.h"
#include "CoreUtils.h"
#include "Disassembler.h"
#include "DisassemblyReport.h"
#include "ElfUtils/ElfFile.h"
#include "FrameTrackOnlineProcessor.h"
#include "FunctionsDataView.h"
#include "GlCanvas.h"
#include "GrpcProtos/Constants.h"
#include "ImGuiOrbit.h"
#include "MainThreadExecutor.h"
#include "MetricsUploader/ScopedMetric.h"
#include "ModulesDataView.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/JoinFutures.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitBase/Tracing.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "OrbitClientData/Callstack.h"
#include "OrbitClientData/CallstackData.h"
#include "OrbitClientData/FunctionUtils.h"
#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ModuleManager.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientData/UserDefinedCaptureData.h"
#include "OrbitClientModel/CaptureDeserializer.h"
#include "OrbitClientModel/CaptureSerializer.h"
#include "OrbitClientModel/SamplingDataPostProcessor.h"
#include "Path.h"
#include "PresetsDataView.h"
#include "SamplingReport.h"
#include "SourceCodeReport.h"
#include "StringManager.h"
#include "SymbolHelper.h"
#include "TimeGraph.h"
#include "Timer.h"
#include "TimerChain.h"
#include "TimerInfosIterator.h"
#include "TrackManager.h"
#include "capture_data.pb.h"
#include "module.pb.h"
#include "orbit_log_event.pb.h"
#include "preset.pb.h"
#include "process.pb.h"
#include "symbol.pb.h"

#ifdef _WIN32
#include "oqpi.hpp"

#define OQPI_USE_DEFAULT
#endif

ABSL_DECLARE_FLAG(bool, devmode);
ABSL_DECLARE_FLAG(bool, local);
ABSL_DECLARE_FLAG(bool, enable_tracepoint_feature);
ABSL_DECLARE_FLAG(bool, enable_source_code_view);

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::FunctionStats;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::PresetFile;
using orbit_client_protos::PresetInfo;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::CrashOrbitServiceRequest_CrashType;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::TracepointInfo;

using orbit_base::Future;

using orbit_metrics_uploader::ScopedMetric;

namespace {
PresetLoadState GetPresetLoadStateForProcess(
    const std::shared_ptr<orbit_client_protos::PresetFile>& preset, const ProcessData* process) {
  if (process == nullptr) {
    return PresetLoadState::kNotLoadable;
  }

  int modules_not_found_count = 0;
  for (const auto& pair : preset->preset_info().path_to_module()) {
    const std::string& module_path = pair.first;
    if (!process->IsModuleLoaded(module_path)) {
      modules_not_found_count++;
    }
  }

  // Empty preset is also loadable
  if (modules_not_found_count == 0) {
    return PresetLoadState::kLoadable;
  }

  if (modules_not_found_count == preset->preset_info().path_to_module_size()) {
    return PresetLoadState::kNotLoadable;
  }

  return PresetLoadState::kPartiallyLoadable;
}
}  // namespace

bool DoZoom = false;

OrbitApp::OrbitApp(orbit_gl::MainWindowInterface* main_window,
                   MainThreadExecutor* main_thread_executor,
                   const orbit_base::CrashHandler* crash_handler,
                   orbit_metrics_uploader::MetricsUploader* metrics_uploader)
    : main_window_{main_window},
      main_thread_executor_(main_thread_executor),
      crash_handler_(crash_handler),
      metrics_uploader_(metrics_uploader) {
  CHECK(main_window_ != nullptr);

  thread_pool_ = ThreadPool::Create(4 /*min_size*/, 256 /*max_size*/, absl::Seconds(1));
  main_thread_id_ = std::this_thread::get_id();
  data_manager_ = std::make_unique<DataManager>(main_thread_id_);
  module_manager_ = std::make_unique<orbit_client_data::ModuleManager>();
  manual_instrumentation_manager_ = std::make_unique<ManualInstrumentationManager>();
}

OrbitApp::~OrbitApp() {
  AbortCapture();

  try {
    thread_pool_->ShutdownAndWait();
  } catch (const std::exception& e) {
    FATAL("Exception occurred in ThreadPool::ShutdownAndWait(): %s", e.what());
  }

#ifdef _WIN32
  oqpi::default_helpers::stop_scheduler();
#endif
}

void OrbitApp::OnCaptureStarted(ProcessData&& process,
                                absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions,
                                TracepointInfoSet selected_tracepoints,
                                absl::flat_hash_set<uint64_t> frame_track_function_ids) {
  // We need to block until initialization is complete to
  // avoid races when capture thread start processing data.
  absl::Mutex mutex;
  absl::MutexLock mutex_lock(&mutex);
  bool initialization_complete = false;

  main_thread_executor_->Schedule(
      [this, &initialization_complete, &mutex, process = std::move(process),
       selected_functions = std::move(selected_functions),
       selected_tracepoints = std::move(selected_tracepoints),
       frame_track_function_ids = std::move(frame_track_function_ids)]() mutable {
        const bool has_selected_functions = !selected_functions.empty();

        ClearCapture();

        // It is safe to do this write on the main thread, as the capture thread is suspended until
        // this task is completely executed.
        capture_data_ =
            CaptureData(std::move(process), module_manager_.get(), std::move(selected_functions),
                        std::move(selected_tracepoints), std::move(frame_track_function_ids));
        capture_window_->CreateTimeGraph(&capture_data_.value());

        frame_track_online_processor_ =
            orbit_gl::FrameTrackOnlineProcessor(GetCaptureData(), GetMutableTimeGraph());

        CHECK(capture_started_callback_);
        capture_started_callback_();

        if (has_selected_functions) {
          CHECK(select_live_tab_callback_);
          select_live_tab_callback_();
        }

        FireRefreshCallbacks();

        absl::MutexLock lock(&mutex);
        initialization_complete = true;
      });

  bool (*IsTrue)(bool*) = [](bool* value) { return *value; };
  mutex.Await(absl::Condition(IsTrue, &initialization_complete));
}

void OrbitApp::OnCaptureComplete() {
  GetMutableCaptureData().FilterBrokenCallstacks();
  PostProcessedSamplingData post_processed_sampling_data =
      orbit_client_model::CreatePostProcessedSamplingData(*GetCaptureData().GetCallstackData(),
                                                          GetCaptureData());

  main_thread_executor_->Schedule(
      [this, sampling_profiler = std::move(post_processed_sampling_data)]() mutable {
        ORBIT_SCOPE("OnCaptureComplete");
        RefreshFrameTracks();
        GetMutableCaptureData().set_post_processed_sampling_data(sampling_profiler);
        RefreshCaptureView();

        SetSamplingReport(std::move(sampling_profiler),
                          GetCaptureData().GetCallstackData()->GetUniqueCallstacksCopy());
        SetTopDownView(GetCaptureData());
        SetBottomUpView(GetCaptureData());

        CHECK(capture_stopped_callback_);
        capture_stopped_callback_();

        FireRefreshCallbacks();
      });
}

void OrbitApp::OnCaptureCancelled() {
  main_thread_executor_->Schedule([this]() mutable {
    ORBIT_SCOPE("OnCaptureCancelled");
    CHECK(capture_failed_callback_);
    capture_failed_callback_();

    ClearCapture();
  });
}

void OrbitApp::OnCaptureFailed(ErrorMessage error_message) {
  main_thread_executor_->Schedule([this, error_message = std::move(error_message)]() mutable {
    ORBIT_SCOPE("OnCaptureFailed");
    CHECK(capture_failed_callback_);
    capture_failed_callback_();

    ClearCapture();
    SendErrorToUi("Error in capture", error_message.message());
  });
}

void OrbitApp::OnTimer(const TimerInfo& timer_info) {
  if (timer_info.function_id() == 0) {
    GetMutableTimeGraph()->ProcessTimer(timer_info, nullptr);
    return;
  }

  CaptureData& capture_data = GetMutableCaptureData();
  const FunctionInfo& func = capture_data.instrumented_functions().at(timer_info.function_id());
  uint64_t elapsed_nanos = timer_info.end() - timer_info.start();
  capture_data.UpdateFunctionStats(func, elapsed_nanos);
  GetMutableTimeGraph()->ProcessTimer(timer_info, &func);
  frame_track_online_processor_.ProcessTimer(timer_info, func);
}

void OrbitApp::OnKeyAndString(uint64_t key, std::string str) {
  string_manager_.AddIfNotPresent(key, std::move(str));
}

void OrbitApp::OnUniqueCallStack(CallStack callstack) {
  GetMutableCaptureData().AddUniqueCallStack(std::move(callstack));
}

void OrbitApp::OnCallstackEvent(CallstackEvent callstack_event) {
  GetMutableCaptureData().AddCallstackEvent(std::move(callstack_event));
}

void OrbitApp::OnThreadName(int32_t thread_id, std::string thread_name) {
  GetMutableCaptureData().AddOrAssignThreadName(thread_id, std::move(thread_name));
}

void OrbitApp::OnThreadStateSlice(orbit_client_protos::ThreadStateSliceInfo thread_state_slice) {
  GetMutableCaptureData().AddThreadStateSlice(std::move(thread_state_slice));
}

void OrbitApp::OnAddressInfo(LinuxAddressInfo address_info) {
  GetMutableCaptureData().InsertAddressInfo(std::move(address_info));
}

void OrbitApp::OnUniqueTracepointInfo(uint64_t key,
                                      orbit_grpc_protos::TracepointInfo tracepoint_info) {
  GetMutableCaptureData().AddUniqueTracepointEventInfo(key, std::move(tracepoint_info));
}

void OrbitApp::OnTracepointEvent(orbit_client_protos::TracepointEventInfo tracepoint_event_info) {
  int32_t capture_process_id = GetCaptureData().process_id();
  bool is_same_pid_as_target = capture_process_id == tracepoint_event_info.pid();

  GetMutableCaptureData().AddTracepointEventAndMapToThreads(
      tracepoint_event_info.time(), tracepoint_event_info.tracepoint_info_key(),
      tracepoint_event_info.pid(), tracepoint_event_info.tid(), tracepoint_event_info.cpu(),
      is_same_pid_as_target);
}

void OrbitApp::OnValidateFramePointers(std::vector<const ModuleData*> modules_to_validate) {
  thread_pool_->Schedule([modules_to_validate = std::move(modules_to_validate), this] {
    frame_pointer_validator_client_->AnalyzeModules(modules_to_validate);
  });
}

std::unique_ptr<OrbitApp> OrbitApp::Create(
    orbit_gl::MainWindowInterface* main_window, MainThreadExecutor* main_thread_executor,
    const orbit_base::CrashHandler* crash_handler,
    orbit_metrics_uploader::MetricsUploader* metrics_uploader) {
  auto app = std::make_unique<OrbitApp>(main_window, main_thread_executor, crash_handler,
                                        metrics_uploader);

#ifdef _WIN32
  oqpi::default_helpers::start_default_scheduler();
#endif

  return app;
}

void OrbitApp::PostInit(bool is_connected) {
  if (is_connected) {
    CHECK(process_manager_ != nullptr);

    capture_client_ = std::make_unique<CaptureClient>(grpc_channel_, this);

    if (GetTargetProcess() != nullptr) {
      UpdateProcessAndModuleList();
    }

    frame_pointer_validator_client_ =
        std::make_unique<FramePointerValidatorClient>(this, grpc_channel_);

    if (absl::GetFlag(FLAGS_devmode)) {
      crash_manager_ = CrashManager::Create(grpc_channel_);
    }
  }

  ListPresets();

  if (!absl::GetFlag(FLAGS_enable_tracepoint_feature)) {
    return;
  }

  thread_pool_->Schedule([this] {
    std::unique_ptr<TracepointServiceClient> tracepoint_manager =
        TracepointServiceClient::Create(grpc_channel_);

    ErrorMessageOr<std::vector<TracepointInfo>> result = tracepoint_manager->GetTracepointList();

    if (result.has_error()) {
      ERROR("Error retrieving tracepoints: %s", result.error().message());
      SendErrorToUi("Error retrieving tracepoints", result.error().message());
      return;
    }

    main_thread_executor_->Schedule([result, this]() {
      tracepoints_data_view_->SetTracepoints(result.value());

      FireRefreshCallbacks(DataViewType::kTracepoints);
    });
  });
}

static std::vector<std::filesystem::path> ListRegularFilesWithExtension(
    const std::filesystem::path& directory, std::string_view extension) {
  std::vector<std::filesystem::path> files;

  std::error_code error;
  auto directory_iterator = std::filesystem::directory_iterator(directory, error);
  if (error) {
    ERROR("Unable to list files in directory \"%s\": %s", directory.string(), error.message());
    return {};
  }

  for (auto it = std::filesystem::begin(directory_iterator),
            end = std::filesystem::end(directory_iterator);
       it != end; it.increment(error)) {
    if (error) {
      ERROR("Iterating directory \"%s\": %s (increment failed, stopping)", directory.string(),
            error.message());
      break;
    }

    const auto& path = it->path();
    bool is_regular_file = std::filesystem::is_regular_file(path, error);
    if (error) {
      ERROR("Unable to stat \"%s\": %s (ignoring)", path.string(), error.message());
      continue;
    }

    if (!is_regular_file) {
      continue;
    }

    if (path.extension().string() != extension) {
      continue;
    }

    files.push_back(path);
  }

  return files;
}

void OrbitApp::ListPresets() {
  std::vector<std::filesystem::path> preset_filenames =
      ListRegularFilesWithExtension(Path::CreateOrGetPresetDir(), ".opr");
  std::vector<std::shared_ptr<PresetFile>> presets;
  for (const std::filesystem::path& filename : preset_filenames) {
    ErrorMessageOr<PresetInfo> preset_result = ReadPresetFromFile(filename);
    if (preset_result.has_error()) {
      ERROR("Loading preset from \"%s\" failed: %s", filename.string(),
            preset_result.error().message());
      continue;
    }

    auto preset = std::make_shared<PresetFile>();
    preset->set_file_name(filename.string());
    preset->mutable_preset_info()->CopyFrom(preset_result.value());
    presets.push_back(preset);
  }

  presets_data_view_->SetPresets(presets);
}

void OrbitApp::RefreshCaptureView() {
  ORBIT_SCOPE_FUNCTION;
  RequestUpdatePrimitives();
  FireRefreshCallbacks();
  DoZoom = true;  // TODO: remove global, review logic
}

void OrbitApp::RenderImGuiDebugUI() {
  CHECK(debug_canvas_ != nullptr);
  CHECK(capture_window_ != nullptr);
  ScopeImguiContext context(debug_canvas_->GetImGuiContext());
  Orbit_ImGui_NewFrame(debug_canvas_);

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(25, 25, 25, 255));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::Begin("OrbitDebug", nullptr, ImVec2(0, 0), 1.f, window_flags);

  if (ImGui::BeginTabBar("DebugTabBar", ImGuiTabBarFlags_None)) {
    if (ImGui::BeginTabItem("CaptureWindow")) {
      capture_window_->RenderImGuiDebugUI();
      ImGui::EndTabItem();
    }

    if (introspection_window_) {
      if (ImGui::BeginTabItem("Introspection")) {
        introspection_window_->RenderImGuiDebugUI();
        ImGui::EndTabItem();
      }
    }

    if (ImGui::BeginTabItem("Misc")) {
      static bool show_imgui_demo = false;
      ImGui::Checkbox("Show ImGui Demo", &show_imgui_demo);
      if (show_imgui_demo) {
        ImGui::ShowDemoWindow();
      }
      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }

  ImGui::PopStyleVar();
  ImGui::PopStyleColor();
  ImGui::End();

  ImGui::Render();
  debug_canvas_->RequestRedraw();
}

void OrbitApp::Disassemble(int32_t pid, const FunctionInfo& function) {
  CHECK(process_ != nullptr);
  const ModuleData* module = module_manager_->GetModuleByPath(function.loaded_module_path());
  CHECK(module != nullptr);
  const bool is_64_bit = process_->is_64_bit();
  const uint64_t absolute_address =
      function_utils::GetAbsoluteAddress(function, *process_, *module);
  thread_pool_->Schedule([this, absolute_address, is_64_bit, pid, function] {
    auto result = GetProcessManager()->LoadProcessMemory(pid, absolute_address, function.size());
    if (!result.has_value()) {
      SendErrorToUi("Error reading memory", absl::StrFormat("Could not read process memory: %s.",
                                                            result.error().message()));
      return;
    }

    const std::string& memory = result.value();
    Disassembler disasm;
    disasm.AddLine(absl::StrFormat("asm: /* %s */", function_utils::GetDisplayName(function)));
    disasm.Disassemble(memory.data(), memory.size(), absolute_address, is_64_bit);
    if (!HasCaptureData() || !GetCaptureData().has_post_processed_sampling_data()) {
      DisassemblyReport empty_report(disasm);
      SendDisassemblyToUi(disasm.GetResult(), std::move(empty_report));
      return;
    }
    const CaptureData& capture_data = GetCaptureData();
    const PostProcessedSamplingData& post_processed_sampling_data =
        capture_data.post_processed_sampling_data();

    DisassemblyReport report(disasm, absolute_address, post_processed_sampling_data,
                             capture_data.GetCallstackData()->GetCallstackEventsCount());
    SendDisassemblyToUi(disasm.GetResult(), std::move(report));
  });
}

void OrbitApp::ShowSourceCode(const orbit_client_protos::FunctionInfo& function) {
  const ModuleData* module = module_manager_->GetModuleByPath(function.loaded_module_path());

  auto loaded_module = RetrieveModuleWithDebugInfo(module);

  loaded_module
      .ThenIfSuccess(
          main_thread_executor_,
          [this, module,
           function](const std::filesystem::path& local_file_path) -> ErrorMessageOr<void> {
            const auto elf_file = orbit_elf_utils::ElfFile::Create(local_file_path);
            const auto first_line_info_or_error = elf_file.value()->GetLineInfo(function.address());

            if (first_line_info_or_error.has_error()) {
              return ErrorMessage{absl::StrFormat(
                  "Could not find source code line info for function \"%s\" in module \"%s\": %s",
                  function.pretty_name(), module->file_path(),
                  first_line_info_or_error.error().message())};
            }
            const auto& line_info = first_line_info_or_error.value();
            auto source_file_path =
                std::filesystem::path{line_info.source_file()}.lexically_normal();

            std::optional<std::unique_ptr<CodeReport>> code_report;

            if (HasCaptureData() && GetCaptureData().has_post_processed_sampling_data()) {
              const auto& sampling_data = GetCaptureData().post_processed_sampling_data();
              const auto absolute_address =
                  function_utils::GetAbsoluteAddress(function, *process_, *module);

              code_report = std::make_unique<orbit_gl::SourceCodeReport>(
                  line_info.source_file(), function, absolute_address, elf_file.value().get(),
                  sampling_data, GetCaptureData().GetCallstackData()->GetCallstackEventsCount());
            }

            main_window_->ShowSourceCode(source_file_path, line_info.source_line(),
                                         std::move(code_report));
            return outcome::success();
          })
      .Then(main_thread_executor_, [this](const ErrorMessageOr<void>& maybe_error) {
        if (maybe_error.has_error()) {
          SendErrorToUi("Error showing source code", maybe_error.error().message());
        }
      });
}

Timer GMainTimer;

void OrbitApp::MainTick() {
  ORBIT_SCOPE("OrbitApp::MainTick");
  GMainTimer.Restart();

  if (DoZoom && HasCaptureData()) {
    // TODO (b/176077097): TrackManager has to manage sorting by their own.
    GetMutableTimeGraph()->GetTrackManager()->SortTracks();
    capture_window_->ZoomAll();
    RequestUpdatePrimitives();
    DoZoom = false;
  }
}

void OrbitApp::SetCaptureWindow(CaptureWindow* capture) {
  CHECK(capture_window_ == nullptr);
  capture_window_ = capture;
  capture_window_->set_draw_help(false);
}

void OrbitApp::SetDebugCanvas(GlCanvas* debug_canvas) {
  CHECK(debug_canvas_ == nullptr);
  debug_canvas_ = debug_canvas;
  debug_canvas_->EnableImGui();
  debug_canvas_->AddRenderCallback([this]() { RenderImGuiDebugUI(); });
}

void OrbitApp::SetIntrospectionWindow(IntrospectionWindow* introspection_window) {
  CHECK(introspection_window_ == nullptr);
  introspection_window_ = introspection_window;
}

void OrbitApp::StopIntrospection() {
  if (introspection_window_) {
    introspection_window_->StopIntrospection();
  }
}

void OrbitApp::RequestUpdatePrimitives() {
  if (capture_window_ != nullptr) {
    capture_window_->RequestUpdatePrimitives();
  }
}

void OrbitApp::SetSamplingReport(
    PostProcessedSamplingData post_processed_sampling_data,
    absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> unique_callstacks) {
  ORBIT_SCOPE_FUNCTION;
  // clear old sampling report
  if (sampling_report_ != nullptr) {
    sampling_report_->ClearReport();
  }

  auto report = std::make_shared<SamplingReport>(this, std::move(post_processed_sampling_data),
                                                 std::move(unique_callstacks));
  CHECK(sampling_reports_callback_);
  DataView* callstack_data_view = GetOrCreateDataView(DataViewType::kCallstack);
  sampling_reports_callback_(callstack_data_view, report);

  sampling_report_ = report;
}

void OrbitApp::SetSelectionReport(
    PostProcessedSamplingData post_processed_sampling_data,
    absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> unique_callstacks,
    bool has_summary) {
  CHECK(selection_report_callback_);
  // clear old selection report
  if (selection_report_ != nullptr) {
    selection_report_->ClearReport();
  }

  auto report = std::make_shared<SamplingReport>(this, std::move(post_processed_sampling_data),
                                                 std::move(unique_callstacks), has_summary);
  DataView* callstack_data_view = GetOrCreateSelectionCallstackDataView();

  selection_report_ = report;
  selection_report_callback_(callstack_data_view, report);
  FireRefreshCallbacks();
}

void OrbitApp::SetTopDownView(const CaptureData& capture_data) {
  ORBIT_SCOPE_FUNCTION;
  CHECK(top_down_view_callback_);
  std::unique_ptr<CallTreeView> top_down_view = CallTreeView::CreateTopDownViewFromSamplingProfiler(
      capture_data.post_processed_sampling_data(), capture_data);
  top_down_view_callback_(std::move(top_down_view));
}

void OrbitApp::ClearTopDownView() {
  CHECK(top_down_view_callback_);
  top_down_view_callback_(std::make_unique<CallTreeView>());
}

void OrbitApp::SetSelectionTopDownView(
    const PostProcessedSamplingData& selection_post_processed_data,
    const CaptureData& capture_data) {
  CHECK(selection_top_down_view_callback_);
  std::unique_ptr<CallTreeView> selection_top_down_view =
      CallTreeView::CreateTopDownViewFromSamplingProfiler(selection_post_processed_data,
                                                          capture_data);
  selection_top_down_view_callback_(std::move(selection_top_down_view));
}

void OrbitApp::ClearSelectionTopDownView() {
  CHECK(selection_top_down_view_callback_);
  selection_top_down_view_callback_(std::make_unique<CallTreeView>());
}

void OrbitApp::SetBottomUpView(const CaptureData& capture_data) {
  ORBIT_SCOPE_FUNCTION;
  CHECK(bottom_up_view_callback_);
  std::unique_ptr<CallTreeView> bottom_up_view =
      CallTreeView::CreateBottomUpViewFromSamplingProfiler(
          capture_data.post_processed_sampling_data(), capture_data);
  bottom_up_view_callback_(std::move(bottom_up_view));
}

void OrbitApp::ClearBottomUpView() {
  CHECK(bottom_up_view_callback_);
  bottom_up_view_callback_(std::make_unique<CallTreeView>());
}

void OrbitApp::SetSelectionBottomUpView(
    const PostProcessedSamplingData& selection_post_processed_data,
    const CaptureData& capture_data) {
  CHECK(selection_bottom_up_view_callback_);
  std::unique_ptr<CallTreeView> selection_bottom_up_view =
      CallTreeView::CreateBottomUpViewFromSamplingProfiler(selection_post_processed_data,
                                                           capture_data);
  selection_bottom_up_view_callback_(std::move(selection_bottom_up_view));
}

void OrbitApp::ClearSelectionBottomUpView() {
  CHECK(selection_bottom_up_view_callback_);
  selection_bottom_up_view_callback_(std::make_unique<CallTreeView>());
}

std::string OrbitApp::GetCaptureTime() {
  const TimeGraph* time_graph = GetTimeGraph();
  double time = (time_graph == nullptr) ? 0 : time_graph->GetCaptureTimeSpanUs();
  return GetPrettyTime(absl::Microseconds(time));
}

std::string OrbitApp::GetSaveFile(const std::string& extension) {
  CHECK(save_file_callback_);
  return save_file_callback_(extension);
}

void OrbitApp::SetClipboard(const std::string& text) {
  CHECK(clipboard_callback_);
  clipboard_callback_(text);
}

ErrorMessageOr<void> OrbitApp::OnSavePreset(const std::string& filename) {
  ScopedMetric metric{metrics_uploader_,
                      orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_PRESET_SAVE};
  auto save_result = SavePreset(filename);
  if (save_result.has_error()) {
    metric.SetStatusCode(orbit_metrics_uploader::OrbitLogEvent_StatusCode_INTERNAL_ERROR);
    return save_result.error();
  }
  ListPresets();
  Refresh(DataViewType::kPresets);
  return outcome::success();
}

ErrorMessageOr<void> OrbitApp::SavePreset(const std::string& filename) {
  PresetInfo preset;

  for (const auto& function : data_manager_->GetSelectedFunctions()) {
    // GetSelectedFunctions should not contain orbit functions
    CHECK(!function_utils::IsOrbitFunc(function));

    uint64_t hash = function_utils::GetHash(function);
    (*preset.mutable_path_to_module())[function.loaded_module_path()].add_function_hashes(hash);
  }

  for (const auto& function : data_manager_->user_defined_capture_data().frame_track_functions()) {
    uint64_t hash = function_utils::GetHash(function);
    (*preset.mutable_path_to_module())[function.loaded_module_path()]
        .add_frame_track_function_hashes(hash);
  }

  std::string filename_with_ext = filename;
  if (!absl::EndsWith(filename, ".opr")) {
    filename_with_ext += ".opr";
  }

  auto fd_or_error = orbit_base::OpenFileForWriting(filename_with_ext);
  if (fd_or_error.has_error()) {
    std::string error_message = absl::StrFormat("Failed to open \"%s\": %s", filename_with_ext,
                                                fd_or_error.error().message());
    ERROR("%s", error_message);
    return ErrorMessage{error_message};
  }

  LOG("Saving preset to \"%s\"", filename_with_ext);
  if (!preset.SerializeToFileDescriptor(fd_or_error.value().get())) {
    std::string error_message =
        absl::StrFormat("Failed to save preset to \"%s\"", filename_with_ext);
    ERROR("%s", error_message);
    return ErrorMessage(error_message);
  }

  return outcome::success();
}

ErrorMessageOr<PresetInfo> OrbitApp::ReadPresetFromFile(const std::filesystem::path& filename) {
  std::filesystem::path file_path =
      filename.is_absolute() ? filename : Path::CreateOrGetPresetDir() / filename;

  auto fd_or_error = orbit_base::OpenFileForReading(file_path);
  if (fd_or_error.has_error()) {
    std::string error_message = absl::StrFormat("Failed to open \"%s\": %s", file_path.string(),
                                                fd_or_error.error().message());
    ERROR("%s", error_message);
    return ErrorMessage{error_message};
  }

  PresetInfo preset_info;
  if (!preset_info.ParseFromFileDescriptor(fd_or_error.value().get())) {
    std::string error_message =
        absl::StrFormat("Failed to load preset from \"%s\"", file_path.string());
    ERROR("%s", error_message);
    return ErrorMessage{error_message};
  }
  return preset_info;
}

ErrorMessageOr<void> OrbitApp::OnLoadPreset(const std::string& filename) {
  OUTCOME_TRY(preset_info, ReadPresetFromFile(filename));

  auto preset = std::make_shared<PresetFile>();
  preset->set_file_name(filename);
  preset->mutable_preset_info()->CopyFrom(preset_info);
  LoadPreset(preset);
  return outcome::success();
}

PresetLoadState OrbitApp::GetPresetLoadState(
    const std::shared_ptr<orbit_client_protos::PresetFile>& preset) const {
  return GetPresetLoadStateForProcess(preset, GetTargetProcess());
}

ErrorMessageOr<void> OrbitApp::OnSaveCapture(const std::filesystem::path& file_name) {
  ScopedMetric metric{metrics_uploader_,
                      orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_CAPTURE_SAVE};
  const auto& key_to_string_map = string_manager_.GetKeyToStringMap();

  std::vector<std::shared_ptr<TimerChain>> chains = GetTimeGraph()->GetAllSerializableTimerChains();

  TimerInfosIterator timers_it_begin(chains.begin(), chains.end());
  TimerInfosIterator timers_it_end(chains.end(), chains.end());
  const CaptureData& capture_data = GetCaptureData();

  auto save_result = capture_serializer::Save(file_name, capture_data, key_to_string_map,
                                              timers_it_begin, timers_it_end);
  if (save_result.has_error()) {
    metric.SetStatusCode(orbit_metrics_uploader::OrbitLogEvent_StatusCode_INTERNAL_ERROR);
  }

  return save_result;
}

Future<ErrorMessageOr<CaptureListener::CaptureOutcome>> OrbitApp::LoadCaptureFromFile(
    const std::string& file_name) {
  ScopedMetric metric{metrics_uploader_,
                      orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_CAPTURE_LOAD};
  if (capture_window_ != nullptr) {
    capture_window_->set_draw_help(false);
  }
  ClearCapture();
  auto load_future =
      thread_pool_->Schedule([this, file_name, metric = std::move(metric)]() mutable {
        capture_loading_cancellation_requested_ = false;

        ErrorMessageOr<CaptureListener::CaptureOutcome> load_result = capture_deserializer::Load(
            file_name, this, module_manager_.get(), &capture_loading_cancellation_requested_);

        if (load_result.has_error()) {
          metric.SetStatusCode(orbit_metrics_uploader::OrbitLogEvent_StatusCode_INTERNAL_ERROR);
          return load_result;
        }

        switch (load_result.value()) {
          case CaptureOutcome::kCancelled:
            metric.SetStatusCode(orbit_metrics_uploader::OrbitLogEvent_StatusCode_CANCELLED);
            break;
          case CaptureOutcome::kComplete:
            OnCaptureComplete();
            break;
        }

        return load_result;
      });

  DoZoom = true;  // TODO: remove global, review logic

  return load_future;
}

void OrbitApp::OnLoadCaptureCancelRequested() { capture_loading_cancellation_requested_ = true; }

void OrbitApp::FireRefreshCallbacks(DataViewType type) {
  for (DataView* panel : panels_) {
    if (type == DataViewType::kAll || type == panel->GetType()) {
      panel->OnDataChanged();
    }
  }

  CHECK(refresh_callback_);
  refresh_callback_(type);
}

void OrbitApp::StartCapture() {
  const ProcessData* process = GetTargetProcess();
  if (process == nullptr) {
    SendErrorToUi("Error starting capture",
                  "No process selected. Please select a target process for the capture.");
    return;
  }

  if (capture_window_ != nullptr) {
    capture_window_->set_draw_help(false);
  }

  std::vector<FunctionInfo> selected_functions = data_manager_->GetSelectedFunctions();
  std::vector<FunctionInfo> orbit_functions = module_manager_->GetOrbitFunctionsOfProcess(*process);
  selected_functions.insert(selected_functions.end(), orbit_functions.begin(),
                            orbit_functions.end());

  UserDefinedCaptureData user_defined_capture_data = data_manager_->user_defined_capture_data();

  absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions_map;
  absl::flat_hash_set<uint64_t> frame_track_function_ids;
  // non-zero since 0 is reserved for invalid ids.
  uint64_t function_id = 1;
  for (auto& function : selected_functions) {
    const ModuleData* module = module_manager_->GetModuleByPath(function.loaded_module_path());
    CHECK(module != nullptr);
    if (user_defined_capture_data.ContainsFrameTrack(function)) {
      frame_track_function_ids.insert(function_id);
    }
    selected_functions_map[function_id++] = std::move(function);
  }

  TracepointInfoSet selected_tracepoints = data_manager_->selected_tracepoints();
  bool collect_thread_states = data_manager_->collect_thread_states();
  bool enable_introspection = absl::GetFlag(FLAGS_devmode);
  uint64_t max_local_marker_depth_per_command_buffer =
      data_manager_->max_local_marker_depth_per_command_buffer();

  CHECK(capture_client_ != nullptr);
  Future<ErrorMessageOr<CaptureOutcome>> capture_result = capture_client_->Capture(
      thread_pool_.get(), *process, *module_manager_, std::move(selected_functions_map),
      std::move(selected_tracepoints), std::move(frame_track_function_ids), collect_thread_states,
      enable_introspection, max_local_marker_depth_per_command_buffer);

  capture_result.Then(main_thread_executor_, [this](ErrorMessageOr<CaptureOutcome> capture_result) {
    if (capture_result.has_error()) {
      OnCaptureFailed(capture_result.error());
      return;
    }
    switch (capture_result.value()) {
      case CaptureListener::CaptureOutcome::kCancelled:
        OnCaptureCancelled();
        return;
      case CaptureListener::CaptureOutcome::kComplete:
        OnCaptureComplete();
        return;
    }
  });
}

void OrbitApp::StopCapture() {
  if (!capture_client_->StopCapture()) {
    return;
  }

  if (metrics_uploader_ != nullptr) {
    auto capture_time_us =
        std::chrono::duration<double, std::micro>(GetTimeGraph()->GetCaptureTimeSpanUs());
    auto capture_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(capture_time_us);
    metrics_uploader_->SendLogEvent(
        orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_CAPTURE_DURATION, capture_time_ms);
  }

  CHECK(capture_stop_requested_callback_);
  capture_stop_requested_callback_();
}

void OrbitApp::AbortCapture() {
  if (capture_client_ == nullptr) return;

  static constexpr int64_t kMaxWaitForAbortCaptureMs = 2000;
  if (!capture_client_->AbortCaptureAndWait(kMaxWaitForAbortCaptureMs)) {
    return;
  }

  CHECK(capture_stop_requested_callback_);
  capture_stop_requested_callback_();
}

void OrbitApp::ClearCapture() {
  ORBIT_SCOPE_FUNCTION;
  if (capture_window_ != nullptr) {
    capture_window_->ClearTimeGraph();
  }
  capture_data_.reset();

  string_manager_.Clear();

  set_selected_thread_id(orbit_base::kAllProcessThreadsTid);
  SelectTextBox(nullptr);

  UpdateAfterCaptureCleared();

  CHECK(capture_cleared_callback_);
  capture_cleared_callback_();

  FireRefreshCallbacks();
}

void OrbitApp::ToggleDrawHelp() {
  if (capture_window_ != nullptr) {
    capture_window_->ToggleDrawHelp();
  }
}

void OrbitApp::ToggleCapture() {
  if (IsCapturing()) {
    StopCapture();
  } else {
    StartCapture();
  }
}

bool OrbitApp::IsCaptureConnected(const CaptureData& capture) const {
  // This function is used to determine if a capture is in a connected state. Lets imagine a user
  // selects a process and takes a capture. Then the process of the capture is the same as the
  // selected one and that means they are connected. If the user than selects a different process,
  // the capture is not connected anymore. Orbit can be in a similar "capture connected" state, when
  // the user connects to an instance, selects a process and then loads an instance from file that
  // was taken shortly before of the same process.
  // TODO(163303287): It might be the case in the future that captures loaded from file are always
  // opened in a new window (compare b/163303287). Then this function is probably not necessary
  // anymore. Otherwise, this function should probably be more sophisticated and also compare the
  // build-id of the selected process (main module) and the process of the capture.

  const ProcessData* selected_process = GetTargetProcess();
  if (selected_process == nullptr) return false;

  const ProcessData* capture_process = capture.process();
  CHECK(capture_process != nullptr);

  return selected_process->pid() == capture_process->pid() &&
         selected_process->full_path() == capture_process->full_path();
}

void OrbitApp::SendDisassemblyToUi(std::string disassembly, DisassemblyReport report) {
  main_thread_executor_->Schedule(
      [this, disassembly = std::move(disassembly), report = std::move(report)]() mutable {
        CHECK(disassembly_callback_);
        disassembly_callback_(std::move(disassembly), std::move(report));
      });
}

void OrbitApp::SendTooltipToUi(const std::string& tooltip) {
  main_thread_executor_->Schedule([this, tooltip] { main_window_->ShowTooltip(tooltip); });
}

void OrbitApp::SendInfoToUi(const std::string& title, const std::string& text) {
  main_thread_executor_->Schedule([this, title, text] {
    CHECK(info_message_callback_);
    info_message_callback_(title, text);
  });
}

void OrbitApp::SendWarningToUi(const std::string& title, const std::string& text) {
  main_thread_executor_->Schedule([this, title, text] {
    CHECK(warning_message_callback_);
    warning_message_callback_(title, text);
  });
}

void OrbitApp::SendErrorToUi(const std::string& title, const std::string& text) {
  main_thread_executor_->Schedule([this, title, text] {
    CHECK(error_message_callback_);
    error_message_callback_(title, text);
  });
}

orbit_base::Future<ErrorMessageOr<std::filesystem::path>> OrbitApp::RetrieveModuleFromRemote(
    const std::string& module_file_path) {
  ScopedStatus scoped_status = CreateScopedStatus(absl::StrFormat(
      "Searching for symbols on remote instance for module \"%s\"...", module_file_path));

  orbit_base::Future<ErrorMessageOr<std::string>> check_file_on_remote =
      thread_pool_->Schedule([process_manager = GetProcessManager(), module_file_path]() {
        return process_manager->FindDebugInfoFile(module_file_path);
      });

  auto download_file =
      [this, module_file_path, scoped_status = std::move(scoped_status)](
          ErrorMessageOr<std::string> result) mutable -> ErrorMessageOr<std::filesystem::path> {
    if (result.has_error()) return result.error();

    const std::string& debug_file_path = result.value();
    LOG("Found symbols file on the remote: \"%s\" - loading it using scp...", debug_file_path);

    const std::filesystem::path local_debug_file_path =
        symbol_helper_.GenerateCachedFileName(module_file_path);

    scoped_status.UpdateMessage(
        absl::StrFormat(R"(Copying debug info file for "%s" from remote: "%s"...)",
                        module_file_path, debug_file_path));
    SCOPED_TIMED_LOG("Copying \"%s\"", debug_file_path);
    auto scp_result = secure_copy_callback_(debug_file_path, local_debug_file_path.string());

    if (scp_result.has_error()) {
      return ErrorMessage{absl::StrFormat("Could not copy debug info file from the remote: %s",
                                          scp_result.error().message())};
    }

    return local_debug_file_path;
  };

  return check_file_on_remote.Then(main_thread_executor_, std::move(download_file));
}

orbit_base::Future<void> OrbitApp::RetrieveModulesAndLoadSymbols(
    absl::Span<const ModuleData* const> modules) {
  std::vector<orbit_base::Future<void>> futures;
  futures.reserve(modules.size());

  const auto handle_error = [this](const ErrorMessageOr<void>& result) {
    if (result.has_error()) {
      error_message_callback_("Error loading symbols", result.error().message());
      return;
    }
  };

  for (const auto& module : modules) {
    futures.emplace_back(
        RetrieveModuleAndLoadSymbols(module).Then(main_thread_executor_, std::move(handle_error)));
  }

  return orbit_base::JoinFutures(futures);
}

orbit_base::Future<ErrorMessageOr<void>> OrbitApp::RetrieveModuleAndLoadSymbols(
    const ModuleData* module) {
  return RetrieveModuleAndLoadSymbols(module->file_path(), module->build_id());
}

orbit_base::Future<ErrorMessageOr<void>> OrbitApp::RetrieveModuleAndLoadSymbols(
    const std::string& module_path, const std::string& build_id) {
  const ModuleData* const module_data = GetModuleByPath(module_path);
  if (module_data == nullptr) {
    return {ErrorMessage{absl::StrFormat("Module \"%s\" was not found", module_path)}};
  }
  if (module_data->is_loaded()) return {outcome::success()};

  return orbit_base::UnwrapFuture(
      RetrieveModule(module_path, build_id)
          .ThenIfSuccess(main_thread_executor_,
                         [this, module_path](const std::filesystem::path& local_file_path) {
                           return LoadSymbols(local_file_path, module_path);
                         }));
}

orbit_base::Future<ErrorMessageOr<std::filesystem::path>> OrbitApp::RetrieveModule(
    const ModuleData* module) {
  return RetrieveModule(module->file_path(), module->build_id());
}

orbit_base::Future<ErrorMessageOr<std::filesystem::path>> OrbitApp::RetrieveModule(
    const std::string& module_path, const std::string& build_id) {
  const ModuleData* module_data = GetModuleByPath(module_path);

  if (module_data == nullptr) {
    return {ErrorMessage{absl::StrFormat("Module \"%s\" was not found", module_path)}};
  }

  const auto it = modules_currently_loading_.find(module_path);
  if (it != modules_currently_loading_.end()) {
    return it->second;
  }

  auto local_symbols_path = FindModuleLocally(module_path, build_id);

  if (local_symbols_path.has_value()) {
    return local_symbols_path;
  }

  // TODO(177304549): [new UI] maybe come up with a better indicator whether orbit is
  // connected than process_manager != nullptr
  if (absl::GetFlag(FLAGS_local) || GetProcessManager() == nullptr) {
    return local_symbols_path;
  }

  auto final_result =
      RetrieveModuleFromRemote(module_path)
          .Then(main_thread_executor_,
                [this, module_path, local_error_message = local_symbols_path.error().message()](
                    const ErrorMessageOr<std::filesystem::path>& remote_result)
                    -> ErrorMessageOr<std::filesystem::path> {
                  modules_currently_loading_.erase(module_path);

                  // If remote loading fails as well, we combine the error messages.
                  if (remote_result.has_value()) return remote_result;
                  return {ErrorMessage{absl::StrFormat(
                      "Did not find symbols locally or on remote for module \"%s\": %s\n%s",
                      module_path, local_error_message, remote_result.error().message())}};
                });

  modules_currently_loading_.emplace(module_path, final_result);
  return final_result;
}

orbit_base::Future<ErrorMessageOr<std::filesystem::path>> OrbitApp::RetrieveModuleWithDebugInfo(
    const ModuleData* module_data) {
  return RetrieveModuleWithDebugInfo(module_data->file_path(), module_data->build_id());
}

orbit_base::Future<ErrorMessageOr<std::filesystem::path>> OrbitApp::RetrieveModuleWithDebugInfo(
    const std::string& module_path, const std::string& build_id) {
  auto loaded_module = RetrieveModule(module_path, build_id);
  return loaded_module.ThenIfSuccess(
      main_thread_executor_,
      [this, module_path](
          const std::filesystem::path& local_file_path) -> ErrorMessageOr<std::filesystem::path> {
        auto elf_file = orbit_elf_utils::ElfFile::Create(local_file_path);

        if (elf_file.has_error()) return elf_file.error();

        if (elf_file.value()->HasDebugInfo()) return local_file_path;

        if (!elf_file.value()->HasGnuDebuglink()) {
          return ErrorMessage{
              absl::StrFormat("Module \"%s\" neither includes debug info, nor does it contain "
                              "a .gnu_debuglink section which could refer to a separate debug "
                              "info file.",
                              module_path)};
        }

        const auto debuglink = elf_file.value()->GetGnuDebugLinkInfo().value();
        ErrorMessageOr<std::filesystem::path> local_debuginfo_path =
            symbol_helper_.FindDebugInfoFileLocally(debuglink.path.filename().string(),
                                                    debuglink.crc32_checksum);
        if (local_debuginfo_path.has_error()) {
          return ErrorMessage{absl::StrFormat(
              "Module \"%s\" doesn't include debug info, and a separate "
              "debuginfo file wasn't found on this machine, when searching "
              "the paths from your SymbolsPath.txt. Please make sure the "
              "debuginfo file can be found in one of the listed directories. According to "
              "the .gnu_debuglink section, the debuginfo file must be called \"%s\".",
              module_path, debuglink.path.string())};
        }

        elf_file = orbit_elf_utils::ElfFile::Create(local_debuginfo_path.value());
        if (elf_file.has_error()) return elf_file.error();
        return local_debuginfo_path;
      });
}

static ErrorMessageOr<std::filesystem::path> FindModuleLocallyImpl(
    const SymbolHelper& symbol_helper, const std::filesystem::path& module_path,
    const std::string& build_id) {
  if (build_id.empty()) {
    return ErrorMessage(absl::StrFormat(
        "Unable to find local symbols for module \"%s\", build id is empty", module_path.string()));
  }

  std::string error_message;
  {
    const auto symbols_path = symbol_helper.FindSymbolsWithSymbolsPathFile(module_path, build_id);
    if (symbols_path.has_value()) {
      LOG("Found symbols for module \"%s\" in user provided symbol folder. Symbols filename: "
          "\"%s\"",
          module_path.string(), symbols_path.value().string());
      return symbols_path.value();
    }
    error_message += "\n* " + symbols_path.error().message();
  }
  {
    const auto symbols_path = symbol_helper.FindSymbolsInCache(module_path, build_id);
    if (symbols_path.has_value()) {
      LOG("Found symbols for module \"%s\" in cache. Symbols filename: \"%s\"",
          module_path.string(), symbols_path.value().string());
      return symbols_path.value();
    }
    error_message += "\n* " + symbols_path.error().message();
  }
  if (absl::GetFlag(FLAGS_local)) {
    const auto symbols_included_in_module = SymbolHelper::VerifySymbolsFile(module_path, build_id);
    if (symbols_included_in_module.has_value()) {
      LOG("Found symbols included in module: \"%s\"", module_path.string());
      return module_path;
    }
    error_message += "\n* Symbols are not included in module file: " +
                     symbols_included_in_module.error().message();
  }

  error_message = absl::StrFormat("Did not find local symbols for module \"%s\": %s",
                                  module_path.string(), error_message);
  LOG("%s", error_message);
  return ErrorMessage(error_message);
}

ErrorMessageOr<std::filesystem::path> OrbitApp::FindModuleLocally(
    const std::filesystem::path& module_path, const std::string& build_id) {
  const auto scoped_status = CreateScopedStatus(absl::StrFormat(
      "Searching for symbols on local machine for module: \"%s\"...", module_path.string()));
  return FindModuleLocallyImpl(symbol_helper_, module_path, build_id);
}

void OrbitApp::AddSymbols(const std::filesystem::path& module_file_path,
                          const orbit_grpc_protos::ModuleSymbols& module_symbols) {
  ModuleData* module_data = GetMutableModuleByPath(module_file_path.string());
  module_data->AddSymbols(module_symbols);

  const ProcessData* selected_process = GetTargetProcess();
  if (selected_process != nullptr && selected_process->IsModuleLoaded(module_data->file_path())) {
    functions_data_view_->AddFunctions(module_data->GetFunctions());
    LOG("Added loaded function symbols for module \"%s\" to the functions tab",
        module_data->file_path());
  }

  UpdateAfterSymbolLoading();
  FireRefreshCallbacks();
}

orbit_base::Future<ErrorMessageOr<void>> OrbitApp::LoadSymbols(
    const std::filesystem::path& symbols_path, const std::string& module_file_path) {
  const auto it = symbols_currently_loading_.find(module_file_path);
  if (it != symbols_currently_loading_.end()) {
    return it->second;
  }

  auto scoped_status = CreateScopedStatus(absl::StrFormat(
      R"(Loading symbols for "%s" from file "%s"...)", module_file_path, symbols_path.string()));

  auto load_symbols_from_file = thread_pool_->Schedule(
      [symbols_path]() { return SymbolHelper::LoadSymbolsFromFile(symbols_path); });

  auto add_symbols =
      [this, module_file_path, scoped_status = std::move(scoped_status)](
          const ErrorMessageOr<orbit_grpc_protos::ModuleSymbols>& symbols_result) mutable
      -> ErrorMessageOr<void> {
    symbols_currently_loading_.erase(module_file_path);

    if (symbols_result.has_error()) return symbols_result.error();

    AddSymbols(module_file_path, symbols_result.value());

    std::string message =
        absl::StrFormat(R"(Successfully loaded %d symbols for "%s")",
                        symbols_result.value().symbol_infos_size(), module_file_path);
    scoped_status.UpdateMessage(message);
    LOG("%s", message);
    return outcome::success();
  };

  auto result_future = load_symbols_from_file.Then(main_thread_executor_, std::move(add_symbols));
  symbols_currently_loading_.emplace(module_file_path, result_future);
  return result_future;
}

void OrbitApp::LoadSymbols(const std::filesystem::path& symbols_path, ModuleData* module_data,
                           std::vector<uint64_t> function_hashes_to_hook,
                           std::vector<uint64_t> frame_track_function_hashes) {
  // This overload will be removed in a subsequent commit.

  const auto handle_hooks_and_frame_tracks =
      [function_hashes_to_hook = std::move(function_hashes_to_hook),
       frame_track_function_hashes = std::move(frame_track_function_hashes), module_data,
       symbols_path, this](const ErrorMessageOr<void>& result) {
        if (result.has_error()) {
          std::string error_message{absl::StrFormat(
              "Unable to load symbols for %s from file %s: %s", module_data->file_path(),
              symbols_path.string(), result.error().message())};
          ERROR("%s", error_message);
          crash_handler_->DumpWithoutCrash();
          SendErrorToUi("Unexpected error while loading symbols", error_message);
          return;
        }

        if (!function_hashes_to_hook.empty()) {
          SelectFunctionsFromHashes(module_data, function_hashes_to_hook);
          LOG("Auto hooked functions in module \"%s\"", module_data->file_path());
        }

        if (!frame_track_function_hashes.empty()) {
          EnableFrameTracksFromHashes(module_data, frame_track_function_hashes);
          LOG("Added frame tracks in module \"%s\"", module_data->file_path());
        }

        UpdateAfterSymbolLoading();
        FireRefreshCallbacks();
      };

  const auto future = LoadSymbols(symbols_path, module_data->file_path())
                          .Then(main_thread_executor_, std::move(handle_hooks_and_frame_tracks));

  // We discard this future and don't wait for it to complete, as this was the original
  // behaviour of this function which needs to be preserved.
  (void)future;
}

orbit_base::Future<ErrorMessageOr<void>> OrbitApp::LoadPresetModule(
    const std::string& module_path, const orbit_client_protos::PresetModule& preset_module) {
  if (!GetTargetProcess()->IsModuleLoaded(module_path)) {
    return ErrorMessage{"Module not loaded by process."};
  }
  ModuleData* module_data = module_manager_->GetMutableModuleByPath(module_path);
  if (module_data == nullptr) {
    ERROR("module \"%s\" was loaded by the process, but is not part of module manager",
          module_path);
    crash_handler_->DumpWithoutCrash();
    return ErrorMessage{"Unexpected error while loading preset."};
  }

  auto handle_hooks_and_frame_tracks =
      [this, module_data,
       preset_module](const ErrorMessageOr<void>& result) -> ErrorMessageOr<void> {
    if (result.has_error()) return result.error();
    SelectFunctionsFromHashes(module_data, preset_module.function_hashes());
    EnableFrameTracksFromHashes(module_data, preset_module.frame_track_function_hashes());
    return outcome::success();
  };

  return RetrieveModuleAndLoadSymbols(module_data)
      .Then(main_thread_executor_, std::move(handle_hooks_and_frame_tracks));
}

void OrbitApp::SelectFunctionsFromHashes(const ModuleData* module,
                                         absl::Span<const uint64_t> function_hashes) {
  for (const auto function_hash : function_hashes) {
    const orbit_client_protos::FunctionInfo* const function_info =
        module->FindFunctionFromHash(function_hash);
    if (function_info == nullptr) {
      ERROR("Could not find function hash %#x in module \"%s\"", function_hash,
            module->file_path());
      continue;
    }
    SelectFunction(*function_info);
  }
}

void OrbitApp::EnableFrameTracksFromHashes(const ModuleData* module,
                                           absl::Span<const uint64_t> function_hashes) {
  for (const auto function_hash : function_hashes) {
    const orbit_client_protos::FunctionInfo* const function_info =
        module->FindFunctionFromHash(function_hash);
    if (function_info == nullptr) {
      ERROR("Could not find function hash %#x in module \"%s\"", function_hash,
            module->file_path());
      continue;
    }
    EnableFrameTrack(*function_info);
  }
}

void OrbitApp::LoadPreset(const std::shared_ptr<PresetFile>& preset_file) {
  ScopedMetric metric{metrics_uploader_,
                      orbit_metrics_uploader::OrbitLogEvent_LogEventType_ORBIT_PRESET_LOAD};
  std::vector<orbit_base::Future<std::string>> load_module_results{};
  load_module_results.reserve(preset_file->preset_info().path_to_module().size());

  // First we try to load all preset modules in parallel
  for (const auto& [module_path, preset_module] : preset_file->preset_info().path_to_module()) {
    auto load_preset_result = LoadPresetModule(module_path, preset_module);

    orbit_base::ImmediateExecutor immediate_executor{};
    auto future = load_preset_result.Then(
        &immediate_executor,
        [module_path = module_path](const ErrorMessageOr<void>& result) -> std::string {
          if (!result.has_error()) return {};
          // We will return the module_path plus error message in case loading fails.
          return absl::StrFormat("%s, error: \"%s\"", module_path, result.error().message());
        });

    load_module_results.emplace_back(std::move(future));
  }

  // Then - when all modules are loaded or failed to load - we update the UI and potentially show an
  // error message.
  auto results = orbit_base::JoinFutures(absl::MakeConstSpan(load_module_results));
  results.Then(main_thread_executor_, [this, metric = std::move(metric)](
                                          std::vector<std::string> module_paths_not_found) mutable {
    size_t tried_to_load_amount = module_paths_not_found.size();
    module_paths_not_found.erase(
        std::remove_if(module_paths_not_found.begin(), module_paths_not_found.end(),
                       [](const std::string& path) { return path.empty(); }),
        module_paths_not_found.end());

    if (tried_to_load_amount == module_paths_not_found.size()) {
      metric.SetStatusCode(orbit_metrics_uploader::OrbitLogEvent_StatusCode_INTERNAL_ERROR);
      SendErrorToUi("Preset loading failed",
                    absl::StrFormat("None of the modules of the preset were loaded:\n* %s",
                                    absl::StrJoin(module_paths_not_found, "\n* ")));
      return;
    }

    if (!module_paths_not_found.empty()) {
      SendWarningToUi("Preset only partially loaded",
                      absl::StrFormat("The following modules were not loaded:\n* %s",
                                      absl::StrJoin(module_paths_not_found, "\n* ")));
    }

    FireRefreshCallbacks();
  });
}

void OrbitApp::UpdateProcessAndModuleList() {
  auto module_infos = thread_pool_->Schedule(
      [this] { return GetProcessManager()->LoadModuleList(GetTargetProcess()->pid()); });

  auto all_reloaded_modules = module_infos.ThenIfSuccess(
      main_thread_executor_,
      [this](const std::vector<orbit_grpc_protos::ModuleInfo>& module_infos) {
        return ReloadModules(module_infos);
      });

  // `all_modules_reloaded` is a future in a future. So we have to unwrap here.
  orbit_base::UnwrapFuture(all_reloaded_modules)
      .ThenIfSuccess(main_thread_executor_,
                     [this](const std::vector<ErrorMessageOr<void>>& reload_results) {
                       // We ignore whether reloading a particular module failed to preserve the
                       // behaviour from before refactoring this. This can be changed in a
                       // subsequent PR.
                       (void)reload_results;

                       RefreshUIAfterModuleReload();
                     })
      .Then(main_thread_executor_, [this](const ErrorMessageOr<void>& result) {
        if (result.has_error()) {
          std::string error_message =
              absl::StrFormat("Error retrieving modules: %s", result.error().message());
          ERROR("%s", error_message);
          SendErrorToUi("%s", error_message);
        }
      });
}

void OrbitApp::RefreshUIAfterModuleReload() {
  modules_data_view_->UpdateModules(GetMutableTargetProcess());

  functions_data_view_->ClearFunctions();
  for (const auto& [module_path, _] : GetTargetProcess()->GetMemoryMap()) {
    ModuleData* module = module_manager_->GetMutableModuleByPath(module_path);
    if (module->is_loaded()) {
      functions_data_view_->AddFunctions(module->GetFunctions());
    }
  }

  FireRefreshCallbacks();
}

orbit_base::Future<std::vector<ErrorMessageOr<void>>> OrbitApp::ReloadModules(
    absl::Span<const ModuleInfo> module_infos) {
  ProcessData* process = GetMutableTargetProcess();

  CHECK(process != nullptr);
  process->UpdateModuleInfos(module_infos);

  // Updating the list of loaded modules (in memory) of a process, can mean that a process has
  // now less loaded modules than before. If the user hooked (selected) functions of a module
  // that is now not used anymore by the process, these functions need to be deselected (A)

  // Updating a module can result in not having symbols(functions) anymore. In that case all
  // functions from this module need to be deselected (B), because they are not valid
  // anymore. These functions are saved (C), so the module can be loaded again and the functions
  // are then selected (hooked) again (D).

  // This all applies similarly to frame tracks that are based on selected functions.

  // Update modules and get the ones to reload.
  std::vector<ModuleData*> modules_to_reload = module_manager_->AddOrUpdateModules(module_infos);

  absl::flat_hash_map<std::string, std::vector<uint64_t>> function_hashes_to_hook_map;
  for (const FunctionInfo& func : data_manager_->GetSelectedFunctions()) {
    const ModuleData* module = module_manager_->GetModuleByPath(func.loaded_module_path());
    // (A) deselect functions when the module is not loaded by the process anymore
    if (!process->IsModuleLoaded(module->file_path())) {
      data_manager_->DeselectFunction(func);
    } else if (!module->is_loaded()) {
      // (B) deselect when module does not have functions anymore (!is_loaded())
      data_manager_->DeselectFunction(func);
      // (C) Save function hashes, so they can be hooked again after reload
      function_hashes_to_hook_map[module->file_path()].push_back(function_utils::GetHash(func));
    }
  }
  absl::flat_hash_map<std::string, std::vector<uint64_t>> frame_track_function_hashes_map;
  for (const FunctionInfo& func :
       data_manager_->user_defined_capture_data().frame_track_functions()) {
    const ModuleData* module = module_manager_->GetModuleByPath(func.loaded_module_path());
    // Frame tracks are only meaningful if the module for the underlying function is actually
    // loaded by the process.
    if (!process->IsModuleLoaded(module->file_path())) {
      RemoveFrameTrack(func);
    } else if (!module->is_loaded()) {
      RemoveFrameTrack(func);
      frame_track_function_hashes_map[module->file_path()].push_back(function_utils::GetHash(func));
    }
  }

  std::vector<orbit_base::Future<ErrorMessageOr<void>>> reloaded_modules;
  reloaded_modules.reserve(modules_to_reload.size());

  for (const auto& module_to_reload : modules_to_reload) {
    std::vector<uint64_t> hooked_functions =
        std::move(function_hashes_to_hook_map[module_to_reload->file_path()]);
    std::vector<uint64_t> frame_tracks =
        std::move(frame_track_function_hashes_map[module_to_reload->file_path()]);

    auto reloaded_module =
        RetrieveModuleAndLoadSymbols(module_to_reload)
            .ThenIfSuccess(main_thread_executor_, [this, module_to_reload,
                                                   hooked_functions = std::move(hooked_functions),
                                                   frame_tracks = std::move(frame_tracks)]() {
              // (D) Re-hook functions which had been hooked before.
              SelectFunctionsFromHashes(module_to_reload, hooked_functions);
              LOG("Auto hooked functions in module \"%s\"", module_to_reload->file_path());

              EnableFrameTracksFromHashes(module_to_reload, frame_tracks);
              LOG("Added frame tracks in module \"%s\"", module_to_reload->file_path());
            });
    reloaded_modules.emplace_back(std::move(reloaded_module));
  }

  return orbit_base::JoinFutures(absl::MakeConstSpan(reloaded_modules));
}

void OrbitApp::SetCollectThreadStates(bool collect_thread_states) {
  data_manager_->set_collect_thread_states(collect_thread_states);
}

void OrbitApp::SetMaxLocalMarkerDepthPerCommandBuffer(
    uint64_t max_local_marker_depth_per_command_buffer) {
  data_manager_->set_max_local_marker_depth_per_command_buffer(
      max_local_marker_depth_per_command_buffer);
}

void OrbitApp::SelectFunction(const orbit_client_protos::FunctionInfo& func) {
  LOG("Selected %s (address_=0x%" PRIx64 ", loaded_module_path_=%s)", func.pretty_name(),
      func.address(), func.loaded_module_path());
  data_manager_->SelectFunction(func);
}

void OrbitApp::DeselectFunction(const orbit_client_protos::FunctionInfo& func) {
  data_manager_->DeselectFunction(func);
}

[[nodiscard]] bool OrbitApp::IsFunctionSelected(
    const orbit_client_protos::FunctionInfo& func) const {
  return data_manager_->IsFunctionSelected(func);
}

[[nodiscard]] bool OrbitApp::IsFunctionSelected(const SampledFunction& func) const {
  return IsFunctionSelected(func.absolute_address);
}

[[nodiscard]] bool OrbitApp::IsFunctionSelected(uint64_t absolute_address) const {
  const ProcessData* process = GetTargetProcess();
  if (process == nullptr) return false;

  const auto result = process->FindModuleByAddress(absolute_address);
  if (result.has_error()) return false;

  const std::string& module_path = result.value().first;
  const uint64_t module_base_address = result.value().second;

  const ModuleData* module = GetModuleByPath(module_path);
  if (module == nullptr) return false;

  const uint64_t relative_address = absolute_address - module_base_address;
  const FunctionInfo* function = module->FindFunctionByRelativeAddress(relative_address, false);
  if (function == nullptr) return false;

  return data_manager_->IsFunctionSelected(*function);
}

const FunctionInfo* OrbitApp::GetInstrumentedFunction(uint64_t function_id) const {
  return HasCaptureData() ? GetCaptureData().GetInstrumentedFunctionById(function_id) : nullptr;
}

void OrbitApp::SetVisibleFunctionIds(absl::flat_hash_set<uint64_t> visible_function_ids) {
  data_manager_->set_visible_function_ids(std::move(visible_function_ids));
  RequestUpdatePrimitives();
}

bool OrbitApp::IsFunctionVisible(uint64_t function_address) {
  if (data_manager_->IsFunctionVisible(function_address)) {
    return true;
  }

  // TODO(b/179225487): Filtering for manually instrumented scopes is not yet supported.
  // All "Orbit" functions are considered visible. Note: this code will change shortly as
  // we will introduce a TimerInfo type for Orbit API events which will allow faster filtering.
  const auto& instrumented_functions = GetCaptureData().instrumented_functions();
  auto it = instrumented_functions.find(function_address);
  if (it != instrumented_functions.end()) {
    if (function_utils::IsOrbitFunc(it->second)) {
      return true;
    }
  }

  return false;
}

uint64_t OrbitApp::highlighted_function_id() const {
  return data_manager_->highlighted_function_id();
}

void OrbitApp::set_highlighted_function_id(uint64_t highlighted_function_id) {
  data_manager_->set_highlighted_function_id(highlighted_function_id);
  RequestUpdatePrimitives();
}

ThreadID OrbitApp::selected_thread_id() const { return data_manager_->selected_thread_id(); }

void OrbitApp::set_selected_thread_id(ThreadID thread_id) {
  return data_manager_->set_selected_thread_id(thread_id);
}

const TextBox* OrbitApp::selected_text_box() const { return data_manager_->selected_text_box(); }

void OrbitApp::SelectTextBox(const TextBox* text_box) {
  data_manager_->set_selected_text_box(text_box);
  const TimerInfo* timer_info = text_box ? &text_box->GetTimerInfo() : nullptr;
  uint64_t function_id =
      timer_info ? timer_info->function_id() : orbit_grpc_protos::kInvalidFunctionId;
  data_manager_->set_highlighted_function_id(function_id);
  CHECK(timer_selected_callback_);
  timer_selected_callback_(timer_info);
}

void OrbitApp::DeselectTextBox() { data_manager_->set_selected_text_box(nullptr); }

uint64_t OrbitApp::GetFunctionIdToHighlight() const {
  const TextBox* selected_textbox = selected_text_box();
  const TimerInfo* selected_timer_info =
      selected_textbox ? &selected_textbox->GetTimerInfo() : nullptr;
  uint64_t selected_function_id =
      selected_timer_info ? selected_timer_info->function_id() : highlighted_function_id();

  // Highlighting of manually instrumented scopes is not yet supported.
  const FunctionInfo* function_info = GetInstrumentedFunction(selected_function_id);
  if (function_info == nullptr || function_utils::IsOrbitFunc(*function_info)) {
    return orbit_grpc_protos::kInvalidFunctionId;
  }

  return selected_function_id;
}

void OrbitApp::SelectCallstackEvents(const std::vector<CallstackEvent>& selected_callstack_events,
                                     int32_t thread_id) {
  const CallstackData* callstack_data = GetCaptureData().GetCallstackData();
  std::unique_ptr<CallstackData> selection_callstack_data = std::make_unique<CallstackData>();
  for (const CallstackEvent& event : selected_callstack_events) {
    selection_callstack_data->AddCallStackFromKnownCallstackData(event, callstack_data);
  }
  // TODO: this might live on the data_manager
  GetMutableCaptureData().set_selection_callstack_data(std::move(selection_callstack_data));

  // Generate selection report.
  bool generate_summary = thread_id == orbit_base::kAllProcessThreadsTid;
  PostProcessedSamplingData processed_sampling_data =
      orbit_client_model::CreatePostProcessedSamplingData(
          *GetCaptureData().GetSelectionCallstackData(), GetCaptureData(), generate_summary);

  SetSelectionTopDownView(processed_sampling_data, GetCaptureData());
  SetSelectionBottomUpView(processed_sampling_data, GetCaptureData());

  SetSelectionReport(std::move(processed_sampling_data),
                     GetCaptureData().GetSelectionCallstackData()->GetUniqueCallstacksCopy(),
                     generate_summary);
}

void OrbitApp::UpdateAfterSymbolLoading() {
  if (!HasCaptureData()) {
    return;
  }
  const CaptureData& capture_data = GetCaptureData();

  if (sampling_report_ != nullptr) {
    PostProcessedSamplingData post_processed_sampling_data =
        orbit_client_model::CreatePostProcessedSamplingData(*capture_data.GetCallstackData(),
                                                            capture_data);
    sampling_report_->UpdateReport(post_processed_sampling_data,
                                   capture_data.GetCallstackData()->GetUniqueCallstacksCopy());
    GetMutableCaptureData().set_post_processed_sampling_data(post_processed_sampling_data);
    SetTopDownView(capture_data);
    SetBottomUpView(capture_data);
  }

  if (selection_report_ == nullptr) {
    return;
  }

  PostProcessedSamplingData selection_post_processed_sampling_data =
      orbit_client_model::CreatePostProcessedSamplingData(*capture_data.GetSelectionCallstackData(),
                                                          capture_data,
                                                          selection_report_->has_summary());

  SetSelectionTopDownView(selection_post_processed_sampling_data, capture_data);
  SetSelectionBottomUpView(selection_post_processed_sampling_data, capture_data);
  selection_report_->UpdateReport(
      std::move(selection_post_processed_sampling_data),
      capture_data.GetSelectionCallstackData()->GetUniqueCallstacksCopy());
}

void OrbitApp::UpdateAfterCaptureCleared() {
  PostProcessedSamplingData empty_post_processed_sampling_data;
  absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> empty_unique_callstacks;

  if (sampling_report_ != nullptr) {
    SetSamplingReport(empty_post_processed_sampling_data, empty_unique_callstacks);
  }
  ClearTopDownView();
  ClearSelectionTopDownView();
  ClearBottomUpView();
  ClearSelectionBottomUpView();
  if (selection_report_ != nullptr) {
    SetSelectionReport(std::move(empty_post_processed_sampling_data), empty_unique_callstacks,
                       false);
  }
}

DataView* OrbitApp::GetOrCreateDataView(DataViewType type) {
  switch (type) {
    case DataViewType::kFunctions:
      if (!functions_data_view_) {
        functions_data_view_ = std::make_unique<FunctionsDataView>(this);
        panels_.push_back(functions_data_view_.get());
      }
      return functions_data_view_.get();

    case DataViewType::kCallstack:
      if (!callstack_data_view_) {
        callstack_data_view_ = std::make_unique<CallStackDataView>(this);
        panels_.push_back(callstack_data_view_.get());
      }
      return callstack_data_view_.get();

    case DataViewType::kModules:
      if (!modules_data_view_) {
        modules_data_view_ = std::make_unique<ModulesDataView>(this);
        panels_.push_back(modules_data_view_.get());
      }
      return modules_data_view_.get();

    case DataViewType::kPresets:
      if (!presets_data_view_) {
        presets_data_view_ = std::make_unique<PresetsDataView>(this, metrics_uploader_);
        panels_.push_back(presets_data_view_.get());
      }
      return presets_data_view_.get();

    case DataViewType::kSampling:
      FATAL(
          "DataViewType::kSampling Data View construction is not supported by"
          "the factory.");
    case DataViewType::kLiveFunctions:
      FATAL("DataViewType::kLiveFunctions should not be used with the factory.");

    case DataViewType::kAll:
      FATAL("DataViewType::kAll should not be used with the factory.");

    case DataViewType::kTracepoints:
      if (!tracepoints_data_view_) {
        tracepoints_data_view_ = std::make_unique<TracepointsDataView>(this);
        panels_.push_back(tracepoints_data_view_.get());
      }
      return tracepoints_data_view_.get();

    case DataViewType::kInvalid:
      FATAL("DataViewType::kInvalid should not be used with the factory.");
  }
  FATAL("Unreachable");
}

DataView* OrbitApp::GetOrCreateSelectionCallstackDataView() {
  if (selection_callstack_data_view_ == nullptr) {
    selection_callstack_data_view_ = std::make_unique<CallStackDataView>(this);
    panels_.push_back(selection_callstack_data_view_.get());
  }
  return selection_callstack_data_view_.get();
}

void OrbitApp::FilterTracks(const std::string& filter) {
  GetMutableTimeGraph()->SetThreadFilter(filter);
}

void OrbitApp::CrashOrbitService(CrashOrbitServiceRequest_CrashType crash_type) {
  if (absl::GetFlag(FLAGS_devmode)) {
    thread_pool_->Schedule([crash_type, this] { crash_manager_->CrashOrbitService(crash_type); });
  }
}

CaptureClient::State OrbitApp::GetCaptureState() const {
  return capture_client_ ? capture_client_->state() : CaptureClient::State::kStopped;
}

bool OrbitApp::IsCapturing() const {
  return capture_client_ ? capture_client_->IsCapturing() : false;
}

ScopedStatus OrbitApp::CreateScopedStatus(const std::string& initial_message) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  CHECK(status_listener_ != nullptr);
  return ScopedStatus{GetMainThreadExecutor()->weak_from_this(), status_listener_, initial_message};
}

void OrbitApp::SelectTracepoint(const TracepointInfo& tracepoint) {
  data_manager_->SelectTracepoint(tracepoint);
}

void OrbitApp::DeselectTracepoint(const TracepointInfo& tracepoint) {
  data_manager_->DeselectTracepoint(tracepoint);
}

[[nodiscard]] bool OrbitApp::IsTracepointSelected(const TracepointInfo& info) const {
  return data_manager_->IsTracepointSelected(info);
}

void OrbitApp::EnableFrameTrack(const FunctionInfo& function) {
  data_manager_->EnableFrameTrack(function);
}

void OrbitApp::DisableFrameTrack(const FunctionInfo& function) {
  data_manager_->DisableFrameTrack(function);
}

void OrbitApp::AddFrameTrack(const FunctionInfo& function) {
  if (!HasCaptureData()) {
    return;
  }

  std::optional<uint64_t> instrumented_function_id =
      GetCaptureData().FindInstrumentedFunctionIdSlow(function);
  // If the function is not instrumented - ignore it. This happens when user
  // enables frame tracks for a not instrumented function from the function list.
  if (!instrumented_function_id) {
    return;
  }

  AddFrameTrack(instrumented_function_id.value());
}

void OrbitApp::AddFrameTrack(uint64_t instrumented_function_id) {
  CHECK(instrumented_function_id != 0);
  if (!HasCaptureData()) {
    return;
  }

  const FunctionInfo* function =
      GetCaptureData().GetInstrumentedFunctionById(instrumented_function_id);
  CHECK(function != nullptr);

  // We only add a frame track to the actual capture data if the function for the frame
  // track actually has hits in the capture data. Otherwise we can end up in inconsistent
  // states where "empty" frame tracks exist in the capture data (which would also be
  // serialized).
  const FunctionStats& stats = GetCaptureData().GetFunctionStatsOrDefault(*function);
  if (stats.count() > 1) {
    frame_track_online_processor_.AddFrameTrack(instrumented_function_id);
    GetMutableCaptureData().EnableFrameTrack(instrumented_function_id);
    if (!IsCapturing()) {
      AddFrameTrackTimers(instrumented_function_id);
    }
  } else {
    CHECK(empty_frame_track_warning_callback_);
    empty_frame_track_warning_callback_(function->pretty_name());
  }
}

void OrbitApp::RemoveFrameTrack(const FunctionInfo& function) {
  // Ignore this call if there is no capture data
  if (!HasCaptureData()) {
    return;
  }

  std::optional<uint64_t> instrumented_function_id =
      GetCaptureData().FindInstrumentedFunctionIdSlow(function);
  // If the function is not instrumented - ignore it. This happens when user
  // enables frame tracks for a not instrumented function from the function list.
  if (!instrumented_function_id) {
    return;
  }

  RemoveFrameTrack(instrumented_function_id.value());
}

void OrbitApp::RemoveFrameTrack(uint64_t instrumented_function_id) {
  const FunctionInfo* function =
      GetCaptureData().GetInstrumentedFunctionById(instrumented_function_id);
  // Removing a frame track requires that the frame track is disabled in the settings
  // (what is stored in DataManager).
  CHECK(!IsFrameTrackEnabled(*function));
  CHECK(std::this_thread::get_id() == main_thread_id_);

  // We can only remove the frame track from the capture data if we have capture data and
  // the frame track is actually enabled in the capture data.
  if (HasCaptureData() && GetCaptureData().IsFrameTrackEnabled(instrumented_function_id)) {
    frame_track_online_processor_.RemoveFrameTrack(instrumented_function_id);
    GetMutableCaptureData().DisableFrameTrack(instrumented_function_id);
    GetMutableTimeGraph()->RemoveFrameTrack(instrumented_function_id);
  }
}

bool OrbitApp::IsFrameTrackEnabled(const FunctionInfo& function) const {
  return data_manager_->IsFrameTrackEnabled(function);
}

bool OrbitApp::HasFrameTrackInCaptureData(uint64_t instrumented_function_id) const {
  return GetTimeGraph()->HasFrameTrack(instrumented_function_id);
}

void OrbitApp::RefreshFrameTracks() {
  CHECK(HasCaptureData());
  CHECK(std::this_thread::get_id() == main_thread_id_);
  for (const auto& function_id : GetCaptureData().frame_track_function_ids()) {
    GetMutableTimeGraph()->RemoveFrameTrack(function_id);
    AddFrameTrackTimers(function_id);
  }
}

void OrbitApp::AddFrameTrackTimers(uint64_t instrumented_function_id) {
  CHECK(HasCaptureData());
  const FunctionInfo* function =
      GetCaptureData().GetInstrumentedFunctionById(instrumented_function_id);
  CHECK(function != nullptr);
  const FunctionStats& stats = GetCaptureData().GetFunctionStatsOrDefault(*function);
  if (stats.count() == 0) {
    return;
  }

  std::vector<std::shared_ptr<TimerChain>> chains = GetTimeGraph()->GetAllThreadTrackTimerChains();

  std::vector<uint64_t> all_start_times;

  for (const auto& chain : chains) {
    if (!chain) continue;
    for (const TimerBlock& block : *chain) {
      for (uint64_t i = 0; i < block.size(); ++i) {
        const TextBox& box = block[i];
        if (box.GetTimerInfo().function_id() == instrumented_function_id) {
          all_start_times.push_back(box.GetTimerInfo().start());
        }
      }
    }
  }
  std::sort(all_start_times.begin(), all_start_times.end());

  for (size_t k = 0; k < all_start_times.size() - 1; ++k) {
    TimerInfo frame_timer;
    orbit_gl::CreateFrameTrackTimer(instrumented_function_id, all_start_times[k],
                                    all_start_times[k + 1], k, &frame_timer);
    GetMutableTimeGraph()->ProcessTimer(frame_timer, function);
  }
}

void OrbitApp::SetTargetProcess(ProcessData* process) {
  CHECK(process != nullptr);

  if (process != process_) {
    data_manager_->ClearSelectedFunctions();
    data_manager_->ClearUserDefinedCaptureData();
    process_ = process;
  }
}
