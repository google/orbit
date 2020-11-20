// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "App.h"

#include <absl/container/flat_hash_map.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <thread>
#include <utility>

#include "CallStackDataView.h"
#include "Callstack.h"
#include "CaptureWindow.h"
#include "CoreUtils.h"
#include "Disassembler.h"
#include "DisassemblyReport.h"
#include "FrameTrackOnlineProcessor.h"
#include "FunctionsDataView.h"
#include "GlCanvas.h"
#include "ImGuiOrbit.h"
#include "ModulesDataView.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Tracing.h"
#include "OrbitClientData/FunctionUtils.h"
#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ModuleManager.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientModel/CaptureDeserializer.h"
#include "OrbitClientModel/CaptureSerializer.h"
#include "Path.h"
#include "PresetsDataView.h"
#include "ProcessesDataView.h"
#include "SamplingProfiler.h"
#include "SamplingReport.h"
#include "StringManager.h"
#include "SymbolHelper.h"
#include "Timer.h"
#include "TimerInfosIterator.h"
#include "capture_data.pb.h"
#include "preset.pb.h"
#include "symbol.pb.h"

ABSL_DECLARE_FLAG(bool, devmode);
ABSL_DECLARE_FLAG(bool, local);
ABSL_DECLARE_FLAG(bool, enable_tracepoint_feature);
ABSL_DECLARE_FLAG(bool, track_ordering_feature);

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::PresetFile;
using orbit_client_protos::PresetInfo;
using orbit_client_protos::PresetModule;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::CrashOrbitServiceRequest_CrashType;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::ProcessInfo;
using orbit_grpc_protos::TracepointInfo;

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

std::unique_ptr<OrbitApp> GOrbitApp;
bool DoZoom = false;

OrbitApp::OrbitApp(ApplicationOptions&& options,
                   std::unique_ptr<MainThreadExecutor> main_thread_executor)
    : options_(std::move(options)), main_thread_executor_(std::move(main_thread_executor)) {
  thread_pool_ = ThreadPool::Create(4 /*min_size*/, 256 /*max_size*/, absl::Seconds(1));
  main_thread_id_ = std::this_thread::get_id();
  data_manager_ = std::make_unique<DataManager>(main_thread_id_);
  module_manager_ = std::make_unique<OrbitClientData::ModuleManager>();
  manual_instrumentation_manager_ = std::make_unique<ManualInstrumentationManager>();
}

OrbitApp::~OrbitApp() {
#ifdef _WIN32
  oqpi_tk::stop_scheduler();
#endif
}

void OrbitApp::OnCaptureStarted(ProcessData&& process,
                                absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions,
                                TracepointInfoSet selected_tracepoints,
                                UserDefinedCaptureData user_defined_capture_data) {
  // We need to block until initialization is complete to
  // avoid races when capture thread start processing data.
  absl::Mutex mutex;
  absl::MutexLock mutex_lock(&mutex);
  bool initialization_complete = false;

  main_thread_executor_->Schedule(
      [this, &initialization_complete, &mutex, process = std::move(process),
       selected_functions = std::move(selected_functions),
       selected_tracepoints = std::move(selected_tracepoints),
       user_defined_capture_data = std::move(user_defined_capture_data)]() mutable {
        const bool has_selected_functions = !selected_functions.empty();

        ClearCapture();

        // It is safe to do this write on the main thread, as the capture thread is suspended until
        // this task is completely executed.
        capture_data_ =
            CaptureData(std::move(process), module_manager_.get(), std::move(selected_functions),
                        std::move(selected_tracepoints), std::move(user_defined_capture_data));

        frame_track_online_processor_ =
            FrameTrackOnlineProcessor(GetCaptureData(), GCurrentTimeGraph);

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
  SamplingProfiler sampling_profiler(*GetCaptureData().GetCallstackData(), GetCaptureData());
  RefreshFrameTracks();

  main_thread_executor_->Schedule(
      [this, sampling_profiler = std::move(sampling_profiler)]() mutable {
        ORBIT_SCOPE("OnCaptureComplete");
        GetMutableCaptureData().set_sampling_profiler(sampling_profiler);
        RefreshCaptureView();

        SetSamplingReport(std::move(sampling_profiler),
                          GetCaptureData().GetCallstackData()->GetUniqueCallstacksCopy());
        SetTopDownView(GetCaptureData());
        SetBottomUpView(GetCaptureData());

        CHECK(capture_stopped_callback_);
        capture_stopped_callback_();

        CHECK(open_capture_finished_callback_);
        open_capture_finished_callback_();

        FireRefreshCallbacks();
      });
}

void OrbitApp::OnCaptureCancelled() {
  main_thread_executor_->Schedule([this]() mutable {
    ORBIT_SCOPE("OnCaptureCancelled");
    CHECK(capture_failed_callback_);
    capture_failed_callback_();

    CHECK(open_capture_failed_callback_);
    open_capture_failed_callback_();

    ClearCapture();
  });
}

void OrbitApp::OnCaptureFailed(ErrorMessage error_message) {
  main_thread_executor_->Schedule([this, error_message = std::move(error_message)]() mutable {
    ORBIT_SCOPE("OnCaptureFailed");
    CHECK(capture_failed_callback_);
    capture_failed_callback_();

    CHECK(open_capture_failed_callback_);
    open_capture_failed_callback_();

    ClearCapture();
    SendErrorToUi("Error in capture", error_message.message());
  });
}

void OrbitApp::OnTimer(const TimerInfo& timer_info) {
  if (timer_info.function_address() > 0) {
    CaptureData& capture_data = GetMutableCaptureData();
    const FunctionInfo& func = capture_data.selected_functions().at(timer_info.function_address());
    uint64_t elapsed_nanos = timer_info.end() - timer_info.start();
    capture_data.UpdateFunctionStats(func, elapsed_nanos);
    GCurrentTimeGraph->ProcessTimer(timer_info, &func);
    frame_track_online_processor_.ProcessTimer(timer_info, func);
  } else {
    GCurrentTimeGraph->ProcessTimer(timer_info, nullptr);
  }
}

void OrbitApp::OnKeyAndString(uint64_t key, std::string str) {
  string_manager_->AddIfNotPresent(key, std::move(str));
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
    ApplicationOptions&& options, std::unique_ptr<MainThreadExecutor> main_thread_executor) {
  auto app = std::make_unique<OrbitApp>(std::move(options), std::move(main_thread_executor));

#ifdef _WIN32
  oqpi_tk::start_default_scheduler();
#endif

  app->LoadFileMapping();

  return app;
}

void OrbitApp::PostInit() {
  if (!options_.grpc_server_address.empty()) {
    grpc_channel_ = grpc::CreateCustomChannel(
        options_.grpc_server_address, grpc::InsecureChannelCredentials(), grpc::ChannelArguments());
    if (!grpc_channel_) {
      ERROR("Unable to create GRPC channel to %s", options_.grpc_server_address);
    }

    capture_client_ = std::make_unique<CaptureClient>(grpc_channel_, this);

    // TODO: Replace refresh_timeout with config option. Let users to modify it.
    process_manager_ = ProcessManager::Create(grpc_channel_, absl::Milliseconds(1000));

    auto callback = [this](ProcessManager* process_manager) {
      main_thread_executor_->Schedule([this, process_manager]() {
        const std::vector<ProcessInfo>& process_infos = process_manager->GetProcessList();
        data_manager_->UpdateProcessInfos(process_infos);
        processes_data_view_->SetProcessList(process_infos);

        if (GetSelectedProcess() == nullptr && processes_data_view_->GetFirstProcessId() != -1) {
          processes_data_view_->SelectProcess(processes_data_view_->GetFirstProcessId());
        }
        FireRefreshCallbacks(DataViewType::kProcesses);
      });
    };

    process_manager_->SetProcessListUpdateListener(callback);

    frame_pointer_validator_client_ =
        std::make_unique<FramePointerValidatorClient>(this, grpc_channel_);

    if (absl::GetFlag(FLAGS_devmode)) {
      crash_manager_ = CrashManager::Create(grpc_channel_);
    }
  }

  ListPresets();

  string_manager_ = std::make_shared<StringManager>();

  GCurrentTimeGraph->SetStringManager(string_manager_);

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

void OrbitApp::LoadFileMapping() {
  file_mapping_.clear();
  std::string file_name = Path::GetFileMappingFileName();
  if (!std::filesystem::exists(file_name)) {
    std::ofstream outfile(file_name);
    outfile << "//-------------------" << std::endl
            << "// Orbit File Mapping" << std::endl
            << "//-------------------" << std::endl
            << R"(// If the file path in the pdb is "D:\NoAccess\File.cpp")" << std::endl
            << R"(// and File.cpp is locally available in "C:\Available\")" << std::endl
            << "// then enter a file mapping on its own line like so:" << std::endl
            << R"(// "D:\NoAccess\File.cpp" "C:\Available\")" << std::endl
            << std::endl
            << R"("D:\NoAccess" "C:\Available")" << std::endl;

    outfile.close();
  }

  std::fstream infile(file_name);
  if (!infile.fail()) {
    std::string line;
    while (std::getline(infile, line)) {
      if (absl::StartsWith(line, "//")) continue;

      bool contains_quotes = absl::StrContains(line, "\"");

      std::vector<std::string> tokens = absl::StrSplit(line, ' ');

      if (tokens.size() == 2 && !contains_quotes) {
        file_mapping_[ToLower(tokens[0])] = ToLower(tokens[1]);
      } else {
        std::vector<std::string> valid_tokens;
        std::vector<std::string> subtokens = absl::StrSplit(line, '"');
        for (const std::string& subtoken : subtokens) {
          if (!IsBlank(subtoken)) {
            valid_tokens.push_back(subtoken);
          }
        }

        if (valid_tokens.size() > 1) {
          file_mapping_[ToLower(valid_tokens[0])] = ToLower(valid_tokens[1]);
        }
      }
    }
  }
}

void OrbitApp::ListPresets() {
  std::vector<std::string> preset_filenames = Path::ListFiles(Path::CreateOrGetPresetDir(), ".opr");
  std::vector<std::shared_ptr<PresetFile>> presets;
  for (std::string& filename : preset_filenames) {
    ErrorMessageOr<PresetInfo> preset_result = ReadPresetFromFile(filename);
    if (preset_result.has_error()) {
      ERROR("Loading preset from \"%s\" failed: %s", filename, preset_result.error().message());
      continue;
    }

    auto preset = std::make_shared<PresetFile>();
    preset->set_file_name(filename);
    preset->mutable_preset_info()->CopyFrom(preset_result.value());
    presets.push_back(preset);
  }

  presets_data_view_->SetPresets(presets);
}

void OrbitApp::RefreshCaptureView() {
  ORBIT_SCOPE_FUNCTION;
  NeedsRedraw();
  GOrbitApp->FireRefreshCallbacks();
  DoZoom = true;  // TODO: remove global, review logic
}

void OrbitApp::RenderImGui() {
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
  capture_window_->RenderImGui();
  if (introspection_window_) {
    introspection_window_->RenderImGui();
  }
  ImGui::PopStyleVar();
  ImGui::PopStyleColor();
  ImGui::End();

  ImGui::Render();
  debug_canvas_->NeedsRedraw();
}

void OrbitApp::Disassemble(int32_t pid, const FunctionInfo& function) {
  const ProcessData* process = data_manager_->GetProcessByPid(pid);
  CHECK(process != nullptr);
  const ModuleData* module = module_manager_->GetModuleByPath(function.loaded_module_path());
  CHECK(module != nullptr);
  const bool is_64_bit = process->is_64_bit();
  const uint64_t absolute_address = FunctionUtils::GetAbsoluteAddress(function, *process, *module);
  thread_pool_->Schedule([this, absolute_address, is_64_bit, pid, function] {
    auto result = process_manager_->LoadProcessMemory(pid, absolute_address, function.size());
    if (!result.has_value()) {
      SendErrorToUi("Error reading memory", absl::StrFormat("Could not read process memory: %s.",
                                                            result.error().message()));
      return;
    }

    const std::string& memory = result.value();
    Disassembler disasm;
    disasm.AddLine(absl::StrFormat("asm: /* %s */", FunctionUtils::GetDisplayName(function)));
    disasm.Disassemble(memory.data(), memory.size(), absolute_address, is_64_bit);
    if (!sampling_report_) {
      DisassemblyReport empty_report(disasm);
      SendDisassemblyToUi(disasm.GetResult(), std::move(empty_report));
      return;
    }
    const CaptureData& capture_data = GetCaptureData();
    const SamplingProfiler& profiler = capture_data.sampling_profiler();

    DisassemblyReport report(disasm, absolute_address, profiler,
                             capture_data.GetCallstackData()->GetCallstackEventsCount());
    SendDisassemblyToUi(disasm.GetResult(), std::move(report));
  });
}

void OrbitApp::OnExit() {
  AbortCapture();

  process_manager_->Shutdown();
  thread_pool_->ShutdownAndWait();

  GOrbitApp = nullptr;
}

Timer GMainTimer;

// TODO: make it non-static
void OrbitApp::MainTick() {
  ORBIT_SCOPE("OrbitApp::MainTick");
  GMainTimer.Restart();

  if (DoZoom && GOrbitApp->HasCaptureData()) {
    GCurrentTimeGraph->SortTracks();
    GOrbitApp->capture_window_->ZoomAll();
    GOrbitApp->NeedsRedraw();
    DoZoom = false;
  }
}

void OrbitApp::RegisterCaptureWindow(CaptureWindow* capture) {
  CHECK(capture_window_ == nullptr);
  GCurrentTimeGraph = capture->GetTimeGraph();
  capture_window_ = capture;
}

void OrbitApp::RegisterDebugCanvas(GlCanvas* debug_canvas) {
  CHECK(debug_canvas_ == nullptr);
  debug_canvas_ = debug_canvas;
  debug_canvas_->EnableImGui();
  Orbit_ImGui_Init(debug_canvas_->GetInitialFontSize());
  debug_canvas_->AddRenderCallback([this]() { RenderImGui(); });
}

void OrbitApp::RegisterIntrospectionWindow(IntrospectionWindow* introspection_window) {
  CHECK(introspection_window_ == nullptr);
  introspection_window_ = introspection_window;
}

void OrbitApp::StopIntrospection() {
  if (introspection_window_) {
    introspection_window_->StopIntrospection();
  }
}

void OrbitApp::NeedsRedraw() {
  if (capture_window_ != nullptr) {
    capture_window_->NeedsUpdate();
  }
}

void OrbitApp::SetSamplingReport(
    SamplingProfiler sampling_profiler,
    absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> unique_callstacks) {
  ORBIT_SCOPE_FUNCTION;
  // clear old sampling report
  if (sampling_report_ != nullptr) {
    sampling_report_->ClearReport();
  }

  auto report =
      std::make_shared<SamplingReport>(std::move(sampling_profiler), std::move(unique_callstacks));
  CHECK(sampling_reports_callback_);
  DataView* callstack_data_view = GetOrCreateDataView(DataViewType::kCallstack);
  sampling_reports_callback_(callstack_data_view, report);

  sampling_report_ = report;
}

void OrbitApp::SetSelectionReport(
    SamplingProfiler sampling_profiler,
    absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> unique_callstacks,
    bool has_summary) {
  CHECK(selection_report_callback_);
  // clear old selection report
  if (selection_report_ != nullptr) {
    selection_report_->ClearReport();
  }

  auto report = std::make_shared<SamplingReport>(std::move(sampling_profiler),
                                                 std::move(unique_callstacks), has_summary);
  DataView* callstack_data_view = GetOrCreateSelectionCallstackDataView();
  selection_report_callback_(callstack_data_view, report);

  selection_report_ = report;
  FireRefreshCallbacks();
}

void OrbitApp::SetTopDownView(const CaptureData& capture_data) {
  ORBIT_SCOPE_FUNCTION;
  CHECK(top_down_view_callback_);
  std::unique_ptr<CallTreeView> top_down_view = CallTreeView::CreateTopDownViewFromSamplingProfiler(
      capture_data.sampling_profiler(), capture_data);
  top_down_view_callback_(std::move(top_down_view));
}

void OrbitApp::ClearTopDownView() {
  CHECK(top_down_view_callback_);
  top_down_view_callback_(std::make_unique<CallTreeView>());
}

void OrbitApp::SetSelectionTopDownView(const SamplingProfiler& selection_sampling_profiler,
                                       const CaptureData& capture_data) {
  CHECK(selection_top_down_view_callback_);
  std::unique_ptr<CallTreeView> selection_top_down_view =
      CallTreeView::CreateTopDownViewFromSamplingProfiler(selection_sampling_profiler,
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
      CallTreeView::CreateBottomUpViewFromSamplingProfiler(capture_data.sampling_profiler(),
                                                           capture_data);
  bottom_up_view_callback_(std::move(bottom_up_view));
}

void OrbitApp::ClearBottomUpView() {
  CHECK(bottom_up_view_callback_);
  bottom_up_view_callback_(std::make_unique<CallTreeView>());
}

void OrbitApp::SetSelectionBottomUpView(const SamplingProfiler& selection_sampling_profiler,
                                        const CaptureData& capture_data) {
  CHECK(selection_bottom_up_view_callback_);
  std::unique_ptr<CallTreeView> selection_bottom_up_view =
      CallTreeView::CreateBottomUpViewFromSamplingProfiler(selection_sampling_profiler,
                                                           capture_data);
  selection_bottom_up_view_callback_(std::move(selection_bottom_up_view));
}

void OrbitApp::ClearSelectionBottomUpView() {
  CHECK(selection_bottom_up_view_callback_);
  selection_bottom_up_view_callback_(std::make_unique<CallTreeView>());
}

std::string OrbitApp::GetCaptureTime() {
  double time = GCurrentTimeGraph != nullptr ? GCurrentTimeGraph->GetCaptureTimeSpanUs() : 0.0;
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
  OUTCOME_TRY(SavePreset(filename));
  ListPresets();
  Refresh(DataViewType::kPresets);
  return outcome::success();
}

ErrorMessageOr<void> OrbitApp::SavePreset(const std::string& filename) {
  PresetInfo preset;

  for (const auto& function : data_manager_->GetSelectedFunctions()) {
    // GetSelectedFunctions should not contain orbit functions
    CHECK(!FunctionUtils::IsOrbitFunc(function));

    uint64_t hash = FunctionUtils::GetHash(function);
    (*preset.mutable_path_to_module())[function.loaded_module_path()].add_function_hashes(hash);
  }

  for (const auto& function : data_manager_->user_defined_capture_data().frame_track_functions()) {
    uint64_t hash = FunctionUtils::GetHash(function);
    (*preset.mutable_path_to_module())[function.loaded_module_path()]
        .add_frame_track_function_hashes(hash);
  }

  std::string filename_with_ext = filename;
  if (!absl::EndsWith(filename, ".opr")) {
    filename_with_ext += ".opr";
  }

  std::ofstream file(filename_with_ext, std::ios::binary);
  if (file.fail()) {
    ERROR("Saving preset in \"%s\": %s", filename_with_ext, "file.fail()");
    return ErrorMessage(
        absl::StrFormat("Error opening the file \"%s\" for writing", filename_with_ext));
  }

  LOG("Saving preset in \"%s\"", filename_with_ext);
  preset.SerializeToOstream(&file);

  return outcome::success();
}

ErrorMessageOr<PresetInfo> OrbitApp::ReadPresetFromFile(const std::string& filename) {
  std::string file_path = filename;

  if (Path::GetDirectory(filename).empty()) {
    file_path = Path::JoinPath({Path::CreateOrGetPresetDir(), filename});
  }

  std::ifstream file(file_path, std::ios::binary);
  if (file.fail()) {
    ERROR("Loading preset from \"%s\": file.fail()", file_path);
    return ErrorMessage("Error opening the file for reading");
  }

  PresetInfo preset_info;
  if (!preset_info.ParseFromIstream(&file)) {
    ERROR("Loading preset from \"%s\" failed", file_path);
    return ErrorMessage(absl::StrFormat("Error reading the preset"));
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
  return GetPresetLoadStateForProcess(preset, GetSelectedProcess());
}

ErrorMessageOr<void> OrbitApp::OnSaveCapture(const std::string& file_name) {
  const auto& key_to_string_map = GCurrentTimeGraph->GetStringManager()->GetKeyToStringMap();

  std::vector<std::shared_ptr<TimerChain>> chains =
      GCurrentTimeGraph->GetAllSerializableTimerChains();

  TimerInfosIterator timers_it_begin(chains.begin(), chains.end());
  TimerInfosIterator timers_it_end(chains.end(), chains.end());
  const CaptureData& capture_data = GetCaptureData();

  return capture_serializer::Save(file_name, capture_data, key_to_string_map, timers_it_begin,
                                  timers_it_end);
}

void OrbitApp::OnLoadCapture(const std::string& file_name) {
  CHECK(open_capture_callback_);
  open_capture_callback_();
  if (capture_window_ != nullptr) {
    capture_window_->set_draw_help(false);
  }
  ClearCapture();
  string_manager_->Clear();
  thread_pool_->Schedule([this, file_name]() mutable {
    capture_loading_cancellation_requested_ = false;
    capture_deserializer::Load(file_name, this, module_manager_.get(),
                               &capture_loading_cancellation_requested_);
  });

  DoZoom = true;  // TODO: remove global, review logic
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

bool OrbitApp::StartCapture() {
  const ProcessData* process = data_manager_->selected_process();
  if (process == nullptr) {
    SendErrorToUi("Error starting capture",
                  "No process selected. Please select a target process for the capture.");
    return false;
  }

  std::vector<FunctionInfo> selected_functions = data_manager_->GetSelectedFunctions();
  std::vector<FunctionInfo> orbit_functions = module_manager_->GetOrbitFunctionsOfProcess(*process);
  selected_functions.insert(selected_functions.end(), orbit_functions.begin(),
                            orbit_functions.end());

  absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions_map;
  for (auto& function : selected_functions) {
    const ModuleData* module = module_manager_->GetModuleByPath(function.loaded_module_path());
    CHECK(module != nullptr);
    uint64_t absolute_address = FunctionUtils::GetAbsoluteAddress(function, *process, *module);
    selected_functions_map[absolute_address] = std::move(function);
  }

  TracepointInfoSet selected_tracepoints = data_manager_->selected_tracepoints();
  bool enable_introspection = absl::GetFlag(FLAGS_devmode);
  UserDefinedCaptureData user_defined_capture_data =
      data_manager_->mutable_user_defined_capture_data();

  ErrorMessageOr<void> result = capture_client_->StartCapture(
      thread_pool_.get(), *process, *module_manager_, std::move(selected_functions_map),
      std::move(selected_tracepoints), std::move(user_defined_capture_data), enable_introspection);

  if (result.has_error()) {
    SendErrorToUi("Error starting capture", result.error().message());
    return false;
  }

  return true;
}

void OrbitApp::StopCapture() {
  if (!capture_client_->StopCapture()) {
    return;
  }

  CHECK(capture_stop_requested_callback_);
  capture_stop_requested_callback_();
}

void OrbitApp::AbortCapture() {
  if (!capture_client_->TryAbortCapture()) {
    return;
  }

  CHECK(capture_stop_requested_callback_);
  capture_stop_requested_callback_();
}

void OrbitApp::ClearCapture() {
  ORBIT_SCOPE_FUNCTION;
  capture_data_.reset();
  set_selected_thread_id(SamplingProfiler::kAllThreadsFakeTid);
  SelectTextBox(nullptr);

  UpdateAfterCaptureCleared();

  if (GCurrentTimeGraph != nullptr) {
    GCurrentTimeGraph->Clear();
  }

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

bool OrbitApp::SelectProcess(const std::string& process) {
  if (processes_data_view_) {
    return processes_data_view_->SelectProcess(process);
  }

  return false;
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

  const ProcessData* selected_process = GetSelectedProcess();
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
  main_thread_executor_->Schedule([this, tooltip] {
    CHECK(tooltip_callback_);
    tooltip_callback_(tooltip);
  });
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

void OrbitApp::LoadModuleOnRemote(ModuleData* module_data,
                                  std::vector<uint64_t> function_hashes_to_hook,
                                  std::vector<uint64_t> frame_track_function_hashes) {
  ScopedStatus scoped_status = CreateScopedStatus(absl::StrFormat(
      "Searching for symbols on remote instance (module \"%s\")...", module_data->file_path()));
  thread_pool_->Schedule([this, module_data,
                          function_hashes_to_hook = std::move(function_hashes_to_hook),
                          frame_track_function_hashes = std::move(frame_track_function_hashes),
                          scoped_status = std::move(scoped_status)]() mutable {
    const auto result = process_manager_->FindDebugInfoFile(module_data->file_path());

    if (!result) {
      SendErrorToUi("Error loading symbols",
                    absl::StrFormat("Did not find symbols on remote for module \"%s\": %s",
                                    module_data->file_path(), result.error().message()));
      main_thread_executor_->Schedule(
          [this, module_data]() { modules_currently_loading_.erase(module_data->file_path()); });
      return;
    }

    const std::string& debug_file_path = result.value();

    LOG("Found symbols file on the remote: \"%s\" - loading it using scp...", debug_file_path);

    main_thread_executor_->Schedule(
        [this, module_data, function_hashes_to_hook = std::move(function_hashes_to_hook),
         frame_track_function_hashes = std::move(frame_track_function_hashes), debug_file_path,
         scoped_status = std::move(scoped_status)]() mutable {
          const std::filesystem::path local_debug_file_path =
              symbol_helper_.GenerateCachedFileName(module_data->file_path());

          {
            scoped_status.UpdateMessage(
                absl::StrFormat(R"(Copying debug info file for "%s" from remote: "%s"...)",
                                module_data->file_path(), debug_file_path));
            SCOPED_TIMED_LOG("Copying \"%s\"", debug_file_path);
            auto scp_result =
                secure_copy_callback_(debug_file_path, local_debug_file_path.string());
            if (!scp_result) {
              SendErrorToUi("Error loading symbols",
                            absl::StrFormat("Could not copy debug info file from the remote: %s",
                                            scp_result.error().message()));
              modules_currently_loading_.erase(module_data->file_path());
              return;
            }
          }

          LoadSymbols(local_debug_file_path, module_data, std::move(function_hashes_to_hook),
                      std::move(frame_track_function_hashes));
        });
  });
}

void OrbitApp::LoadModules(
    const std::vector<ModuleData*>& modules,
    absl::flat_hash_map<std::string, std::vector<uint64_t>> function_hashes_to_hook_map,
    absl::flat_hash_map<std::string, std::vector<uint64_t>> frame_track_function_hashes_map) {
  for (const auto& module : modules) {
    if (modules_currently_loading_.contains(module->file_path())) {
      continue;
    }
    modules_currently_loading_.insert(module->file_path());

    std::vector<uint64_t> function_hashes_to_hook;
    if (function_hashes_to_hook_map.contains(module->file_path())) {
      function_hashes_to_hook = function_hashes_to_hook_map.at(module->file_path());
    }

    std::vector<uint64_t> frame_track_function_hashes;
    if (frame_track_function_hashes_map.contains(module->file_path())) {
      frame_track_function_hashes = frame_track_function_hashes_map.at(module->file_path());
    }

    const auto& symbols_path = FindSymbolsLocally(module->file_path(), module->build_id());
    if (symbols_path) {
      LoadSymbols(symbols_path.value(), module, std::move(function_hashes_to_hook),
                  std::move(frame_track_function_hashes));
      continue;
    }

    if (!absl::GetFlag(FLAGS_local)) {
      LoadModuleOnRemote(module, std::move(function_hashes_to_hook),
                         std::move(frame_track_function_hashes));
      continue;
    }

    // If no symbols are found and remote loading is not attempted.
    SendErrorToUi("Error loading symbols",
                  absl::StrFormat("Did not find symbols for module \"%s\": %s", module->file_path(),
                                  symbols_path.error().message()));
    modules_currently_loading_.erase(module->file_path());
  }
}

ErrorMessageOr<std::filesystem::path> OrbitApp::FindSymbolsLocally(
    const std::filesystem::path& module_path, const std::string& build_id) {
  const auto scoped_status = CreateScopedStatus(absl::StrFormat(
      "Searching for symbols on local machine (module: \"%s\")...", module_path.string()));

  if (build_id.empty()) {
    return ErrorMessage(absl::StrFormat(
        "Unable to find local symbols for module \"%s\", build id is empty", module_path.string()));
  }

  std::string error_message;
  {
    const auto symbols_path = symbol_helper_.FindSymbolsWithSymbolsPathFile(module_path, build_id);
    if (symbols_path) {
      LOG("Found symbols for module \"%s\" in user provided symbol folder. Symbols filename: "
          "\"%s\"",
          module_path.string(), symbols_path.value().string());
      return symbols_path.value();
    }
    error_message += "\n* " + symbols_path.error().message();
  }
  {
    const auto symbols_path = symbol_helper_.FindSymbolsInCache(module_path, build_id);
    if (symbols_path) {
      LOG("Found symbols for module \"%s\" in cache. Symbols filename: \"%s\"",
          module_path.string(), symbols_path.value().string());
      return symbols_path.value();
    }
    error_message += "\n* " + symbols_path.error().message();
  }
  if (absl::GetFlag(FLAGS_local)) {
    const auto symbols_included_in_module = SymbolHelper::VerifySymbolsFile(module_path, build_id);
    if (symbols_included_in_module) {
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

void OrbitApp::LoadSymbols(const std::filesystem::path& symbols_path, ModuleData* module_data,
                           std::vector<uint64_t> function_hashes_to_hook,
                           std::vector<uint64_t> frame_track_function_hashes) {
  auto scoped_status =
      CreateScopedStatus(absl::StrFormat(R"(Loading symbols for "%s" from file "%s"...)",
                                         module_data->file_path(), symbols_path.string()));
  thread_pool_->Schedule([this, scoped_status = std::move(scoped_status), symbols_path, module_data,
                          function_hashes_to_hook = std::move(function_hashes_to_hook),
                          frame_track_function_hashes =
                              std::move(frame_track_function_hashes)]() mutable {
    auto symbols_result = SymbolHelper::LoadSymbolsFromFile(symbols_path);
    CHECK(symbols_result);
    module_data->AddSymbols(symbols_result.value());

    std::string message =
        absl::StrFormat(R"(Successfully loaded %d symbols for "%s")",
                        symbols_result.value().symbol_infos_size(), module_data->file_path());
    scoped_status.UpdateMessage(message);
    LOG("%s", message);

    main_thread_executor_->Schedule([this, scoped_status = std::move(scoped_status), module_data,
                                     function_hashes_to_hook = std::move(function_hashes_to_hook),
                                     frame_track_function_hashes =
                                         std::move(frame_track_function_hashes)] {
      modules_currently_loading_.erase(module_data->file_path());

      const ProcessData* selected_process = GetSelectedProcess();
      if (selected_process != nullptr &&
          selected_process->IsModuleLoaded(module_data->file_path())) {
        functions_data_view_->AddFunctions(module_data->GetFunctions());
        LOG("Added loaded function symbols for module \"%s\" to the functions tab",
            module_data->file_path());
      }

      if (!function_hashes_to_hook.empty()) {
        const auto selection_result =
            SelectFunctionsFromHashes(module_data, function_hashes_to_hook);
        if (!selection_result) {
          LOG("Warning, automated hooked incomplete: %s", selection_result.error().message());
        }
        LOG("Auto hooked functions in module \"%s\"", module_data->file_path());
      }

      if (!frame_track_function_hashes.empty()) {
        const auto frame_track_result =
            InsertFrameTracksFromHashes(module_data, frame_track_function_hashes);
        if (!frame_track_result) {
          LOG("Warning, could not insert frame tracks: %s", frame_track_result.error().message());
        }
        LOG("Added frame tracks in module \"%s\"", module_data->file_path());
      }

      UpdateAfterSymbolLoading();
      GOrbitApp->FireRefreshCallbacks();
    });
  });
}

ErrorMessageOr<void> OrbitApp::GetFunctionInfosFromHashes(
    const ModuleData* module, const std::vector<uint64_t>& function_hashes,
    std::vector<const FunctionInfo*>* function_infos) {
  const ProcessData* process = GetSelectedProcess();
  if (process == nullptr) {
    return ErrorMessage(absl::StrFormat(
        "Unable to get function infos for module \"%s\", because no process is selected",
        module->file_path()));
  }
  if (!process->IsModuleLoaded(module->file_path())) {
    return ErrorMessage(absl::StrFormat(
        R"(Unable to get function infos for module "%s", because the module is not loaded by process "%s")",
        module->file_path(), process->name()));
  }

  size_t count_missing = 0;
  for (const uint64_t function_hash : function_hashes) {
    const FunctionInfo* function = module->FindFunctionFromHash(function_hash);
    if (function == nullptr) {
      count_missing++;
      continue;
    }
    function_infos->push_back(function);
  }
  if (count_missing != 0) {
    return ErrorMessage(absl::StrFormat("* %d function infos missing from module \"%s\"\n",
                                        count_missing, module->file_path()));
  }
  return outcome::success();
}

ErrorMessageOr<void> OrbitApp::SelectFunctionsFromHashes(
    const ModuleData* module, const std::vector<uint64_t>& function_hashes) {
  std::vector<const FunctionInfo*> function_infos;
  const auto& error = GetFunctionInfosFromHashes(module, function_hashes, &function_infos);
  for (const auto* function : function_infos) {
    SelectFunction(*function);
  }
  return error;
}

ErrorMessageOr<void> OrbitApp::InsertFrameTracksFromHashes(
    const ModuleData* module, const std::vector<uint64_t> function_hashes) {
  std::vector<const FunctionInfo*> function_infos;
  const auto& error = GetFunctionInfosFromHashes(module, function_hashes, &function_infos);
  for (const auto* function : function_infos) {
    data_manager_->user_defined_capture_data().InsertFrameTrack(*function);
  }
  return error;
}

void OrbitApp::LoadPreset(const std::shared_ptr<PresetFile>& preset_file) {
  std::vector<ModuleData*> modules_to_load;
  std::vector<std::string> module_paths_not_found;  // file path of module
  for (const auto& [module_path, preset_module] : preset_file->preset_info().path_to_module()) {
    ModuleData* module_data = module_manager_->GetMutableModuleByPath(module_path);

    if (module_data == nullptr) {
      module_paths_not_found.push_back(module_path);
      continue;
    }
    if (module_data->is_loaded()) {
      std::vector<uint64_t> function_hashes{preset_module.function_hashes().begin(),
                                            preset_module.function_hashes().end()};
      const auto selecting_result = SelectFunctionsFromHashes(module_data, function_hashes);
      if (!selecting_result) {
        LOG("Warning: %s", selecting_result.error().message());
      }
      std::vector<uint64_t> frame_track_hashes{preset_module.frame_track_function_hashes().begin(),
                                               preset_module.frame_track_function_hashes().end()};
      const auto frame_track_result = InsertFrameTracksFromHashes(module_data, frame_track_hashes);
      if (!frame_track_result) {
        LOG("Warning: %s", frame_track_result.error().message());
      }
      continue;
    }
    modules_to_load.push_back(module_data);
  }
  if (!module_paths_not_found.empty()) {
    if (static_cast<int>(module_paths_not_found.size()) ==
        preset_file->preset_info().path_to_module_size()) {
      // no modules were loaded
      SendErrorToUi("Preset loading failed",
                    absl::StrFormat("None of the modules in the preset are loaded."));
    } else {
      SendWarningToUi("Preset only partially loaded",
                      absl::StrFormat("The following modules are not loaded:\n\"%s\"",
                                      absl::StrJoin(module_paths_not_found, "\"\n\"")));
    }
  }
  if (!modules_to_load.empty()) {
    absl::flat_hash_map<std::string, std::vector<uint64_t>> function_hashes_to_hook_map;
    absl::flat_hash_map<std::string, std::vector<uint64_t>> frame_track_function_hashes_map;
    for (const auto& [module_path, preset_module] : preset_file->preset_info().path_to_module()) {
      function_hashes_to_hook_map[module_path] = std::vector<uint64_t>{};
      for (const uint64_t function_hash : preset_module.function_hashes()) {
        function_hashes_to_hook_map.at(module_path).push_back(function_hash);
      }
      frame_track_function_hashes_map[module_path] = std::vector<uint64_t>{};
      for (const uint64_t function_hash : preset_module.frame_track_function_hashes()) {
        frame_track_function_hashes_map.at(module_path).push_back(function_hash);
      }
    }
    LoadModules(modules_to_load, std::move(function_hashes_to_hook_map),
                std::move(frame_track_function_hashes_map));
  }
  FireRefreshCallbacks();
}

void OrbitApp::UpdateProcessAndModuleList(int32_t pid) {
  CHECK(processes_data_view_->GetSelectedProcessId() == pid);
  thread_pool_->Schedule([pid, this] {
    ErrorMessageOr<std::vector<ModuleInfo>> result = process_manager_->LoadModuleList(pid);

    if (result.has_error()) {
      ERROR("Error retrieving modules: %s", result.error().message());
      SendErrorToUi("Error retrieving modules", result.error().message());
      return;
    }

    main_thread_executor_->Schedule([pid, module_infos = std::move(result.value()), this] {
      // Make sure that pid is actually what user has selected at
      // the moment we arrive here. If not - ignore the result.
      if (pid != processes_data_view_->GetSelectedProcessId()) {
        return;
      }

      ProcessData* process = data_manager_->GetMutableProcessByPid(pid);
      CHECK(process != nullptr);
      process->UpdateModuleInfos(module_infos);

      // If no process was selected before, or the process changed
      if (GetSelectedProcess() == nullptr || pid != GetSelectedProcess()->pid()) {
        data_manager_->ClearSelectedFunctions();
        data_manager_->set_selected_process(pid);
      }

      // Updating the list of loaded modules (in memory) of a process, can mean that a process has
      // now less loaded modules than before. If the user hooked (selected) functions of a module
      // that is now not used anymore by the process, these functions need to be deselected (A)

      // Updating a module can result in not having symbols(functions) anymore. In that case all
      // functions from this module need to be deselected (B), because they are not valid
      // anymore. These functions are saved (C), so the module can be loaded again and the functions
      // are then selected (hooked) again (D).

      // This all applies similarly to frame tracks that are based on selected functions.

      // Update modules and get the ones to reload.
      std::vector<ModuleData*> modules_to_reload =
          module_manager_->AddOrUpdateModules(module_infos);

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
          function_hashes_to_hook_map[module->file_path()].push_back(FunctionUtils::GetHash(func));
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
          frame_track_function_hashes_map[module->file_path()].push_back(
              FunctionUtils::GetHash(func));
        }
      }
      // (D) Load Modules again (and pass on functions to hook after loading)
      LoadModules(modules_to_reload, std::move(function_hashes_to_hook_map),
                  std::move(frame_track_function_hashes_map));

      // Refresh UI
      modules_data_view_->UpdateModules(process);

      functions_data_view_->ClearFunctions();
      for (const auto& [module_path, _] : GetSelectedProcess()->GetMemoryMap()) {
        ModuleData* module = module_manager_->GetMutableModuleByPath(module_path);
        if (module->is_loaded()) {
          functions_data_view_->AddFunctions(module->GetFunctions());
        }
      }

      FireRefreshCallbacks();
    });
  });
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
  const ProcessData* process = data_manager_->selected_process();
  if (process == nullptr) return false;

  const auto result = process->FindModuleByAddress(absolute_address);
  if (!result) return false;

  const std::string& module_path = result.value().first;
  const uint64_t module_base_address = result.value().second;

  const ModuleData* module = GetModuleByPath(module_path);
  if (module == nullptr) return false;

  const uint64_t relative_address = absolute_address - module_base_address;
  const FunctionInfo* function = module->FindFunctionByRelativeAddress(relative_address, false);
  if (function == nullptr) return false;

  return data_manager_->IsFunctionSelected(*function);
}

void OrbitApp::SetVisibleFunctions(absl::flat_hash_set<uint64_t> visible_functions) {
  data_manager_->set_visible_functions(std::move(visible_functions));
  NeedsRedraw();
}

[[nodiscard]] bool OrbitApp::IsFunctionVisible(uint64_t function_address) {
  return data_manager_->IsFunctionVisible(function_address);
}

ThreadID OrbitApp::selected_thread_id() const { return data_manager_->selected_thread_id(); }

void OrbitApp::set_selected_thread_id(ThreadID thread_id) {
  return data_manager_->set_selected_thread_id(thread_id);
}

const TextBox* OrbitApp::selected_text_box() const { return data_manager_->selected_text_box(); }
void OrbitApp::SelectTextBox(const TextBox* text_box) {
  data_manager_->set_selected_text_box(text_box);
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
  bool generate_summary = thread_id == SamplingProfiler::kAllThreadsFakeTid;
  SamplingProfiler sampling_profiler(*GetCaptureData().GetSelectionCallstackData(),
                                     GetCaptureData(), generate_summary);

  SetSelectionTopDownView(sampling_profiler, GetCaptureData());
  SetSelectionBottomUpView(sampling_profiler, GetCaptureData());

  SetSelectionReport(std::move(sampling_profiler),
                     GetCaptureData().GetSelectionCallstackData()->GetUniqueCallstacksCopy(),
                     generate_summary);
}

void OrbitApp::UpdateAfterSymbolLoading() {
  if (!HasCaptureData()) {
    return;
  }
  const CaptureData& capture_data = GetCaptureData();

  if (sampling_report_ != nullptr) {
    SamplingProfiler sampling_profiler(*capture_data.GetCallstackData(), capture_data);
    sampling_report_->UpdateReport(sampling_profiler,
                                   capture_data.GetCallstackData()->GetUniqueCallstacksCopy());
    GetMutableCaptureData().set_sampling_profiler(sampling_profiler);
    SetTopDownView(capture_data);
    SetBottomUpView(capture_data);
  }

  if (selection_report_ == nullptr) {
    return;
  }

  // TODO(kuebler): propagate this information
  SamplingProfiler selection_profiler(*capture_data.GetSelectionCallstackData(), capture_data,
                                      selection_report_->has_summary());

  SetSelectionTopDownView(selection_profiler, capture_data);
  SetSelectionBottomUpView(selection_profiler, capture_data);
  selection_report_->UpdateReport(
      std::move(selection_profiler),
      capture_data.GetSelectionCallstackData()->GetUniqueCallstacksCopy());
}

void OrbitApp::UpdateAfterCaptureCleared() {
  SamplingProfiler empty_profiler;
  absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> empty_unique_callstacks;

  SetSamplingReport(empty_profiler, empty_unique_callstacks);
  ClearTopDownView();
  ClearSelectionTopDownView();
  ClearBottomUpView();
  ClearSelectionBottomUpView();
  if (selection_report_) {
    SetSelectionReport(std::move(empty_profiler), empty_unique_callstacks, false);
  }
}

DataView* OrbitApp::GetOrCreateDataView(DataViewType type) {
  switch (type) {
    case DataViewType::kFunctions:
      if (!functions_data_view_) {
        functions_data_view_ = std::make_unique<FunctionsDataView>();
        panels_.push_back(functions_data_view_.get());
      }
      return functions_data_view_.get();

    case DataViewType::kCallstack:
      if (!callstack_data_view_) {
        callstack_data_view_ = std::make_unique<CallStackDataView>();
        panels_.push_back(callstack_data_view_.get());
      }
      return callstack_data_view_.get();

    case DataViewType::kModules:
      if (!modules_data_view_) {
        modules_data_view_ = std::make_unique<ModulesDataView>();
        panels_.push_back(modules_data_view_.get());
      }
      return modules_data_view_.get();

    case DataViewType::kProcesses:
      if (!processes_data_view_) {
        processes_data_view_ = std::make_unique<ProcessesDataView>();
        processes_data_view_->SetSelectionListener(
            [&](int32_t pid) { UpdateProcessAndModuleList(pid); });
        panels_.push_back(processes_data_view_.get());
      }
      return processes_data_view_.get();

    case DataViewType::kPresets:
      if (!presets_data_view_) {
        presets_data_view_ = std::make_unique<PresetsDataView>();
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
        tracepoints_data_view_ = std::make_unique<TracepointsDataView>();
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
    selection_callstack_data_view_ = std::make_unique<CallStackDataView>();
    panels_.push_back(selection_callstack_data_view_.get());
  }
  return selection_callstack_data_view_.get();
}

void OrbitApp::FilterTracks(const std::string& filter) {
  GCurrentTimeGraph->SetThreadFilter(filter);
}

void OrbitApp::CrashOrbitService(CrashOrbitServiceRequest_CrashType crash_type) {
  if (absl::GetFlag(FLAGS_devmode)) {
    thread_pool_->Schedule([crash_type, this] { crash_manager_->CrashOrbitService(crash_type); });
  }
}

bool OrbitApp::IsCapturing() const {
  return capture_client_ ? capture_client_->IsCapturing() : false;
}

ScopedStatus OrbitApp::CreateScopedStatus(const std::string& initial_message) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  CHECK(status_listener_ != nullptr);
  return ScopedStatus{main_thread_executor_.get(), status_listener_, initial_message};
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

void OrbitApp::AddFrameTrack(const FunctionInfo& function) {
  GetMutableCaptureData().InsertFrameTrack(function);
  data_manager_->set_user_defined_capture_data(GetCaptureData().user_defined_capture_data());
  AddFrameTrackTimers(function);
}

void OrbitApp::RemoveFrameTrack(const FunctionInfo& function) {
  GetMutableCaptureData().EraseFrameTrack(function);
  data_manager_->set_user_defined_capture_data(GetCaptureData().user_defined_capture_data());
  GCurrentTimeGraph->RemoveFrameTrack(function);
}

bool OrbitApp::HasFrameTrack(const FunctionInfo& function) const {
  return GetCaptureData().ContainsFrameTrack(function);
}

void OrbitApp::RefreshFrameTracks() {
  for (const auto& function :
       GetCaptureData().user_defined_capture_data().frame_track_functions()) {
    GCurrentTimeGraph->RemoveFrameTrack(function);
    AddFrameTrackTimers(function);
  }
}

void OrbitApp::AddFrameTrackTimers(const FunctionInfo& function) {
  const CaptureData& capture_data = GetCaptureData();
  const uint64_t function_address = capture_data.GetAbsoluteAddress(function);

  std::vector<std::shared_ptr<TimerChain>> chains =
      GCurrentTimeGraph->GetAllThreadTrackTimerChains();

  std::vector<uint64_t> all_start_times;

  for (const auto& chain : chains) {
    if (!chain) continue;
    for (const TimerBlock& block : *chain) {
      for (uint64_t i = 0; i < block.size(); ++i) {
        const TextBox& box = block[i];
        if (box.GetTimerInfo().function_address() == function_address) {
          all_start_times.push_back(box.GetTimerInfo().start());
        }
      }
    }
  }
  std::sort(all_start_times.begin(), all_start_times.end());

  for (size_t k = 0; k < all_start_times.size() - 1; ++k) {
    TimerInfo frame_timer;

    // TID is meaningless for this timer (start and end can be on two different threads).
    constexpr const int32_t kUnusedThreadId = -1;
    frame_timer.set_thread_id(kUnusedThreadId);
    frame_timer.set_start(all_start_times[k]);
    frame_timer.set_end(all_start_times[k + 1]);
    // We use user_data_key to keep track of the frame number.
    frame_timer.set_user_data_key(k);
    frame_timer.set_type(TimerInfo::kFrame);

    GCurrentTimeGraph->ProcessTimer(frame_timer, &function);
  }
}
