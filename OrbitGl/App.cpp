// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "App.h"

#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <outcome.hpp>
#include <thread>
#include <utility>

#include "CallStackDataView.h"
#include "Callstack.h"
#include "Capture.h"
#include "CaptureSerializer.h"
#include "CaptureWindow.h"
#include "Disassembler.h"
#include "DisassemblyReport.h"
#include "EventTracer.h"
#include "FunctionUtils.h"
#include "FunctionsDataView.h"
#include "GlCanvas.h"
#include "ImGuiOrbit.h"
#include "ModulesDataView.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Tracing.h"
#include "Path.h"
#include "Pdb.h"
#include "PresetsDataView.h"
#include "ProcessesDataView.h"
#include "SamplingProfiler.h"
#include "SamplingReport.h"
#include "ScopeTimer.h"
#include "StringManager.h"
#include "Utils.h"

ABSL_DECLARE_FLAG(bool, devmode);

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::PresetFile;
using orbit_client_protos::PresetInfo;
using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::CrashOrbitServiceRequest_CrashType;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ProcessInfo;

std::unique_ptr<OrbitApp> GOrbitApp;
bool DoZoom = false;

OrbitApp::OrbitApp(ApplicationOptions&& options,
                   std::unique_ptr<MainThreadExecutor> main_thread_executor)
    : options_(std::move(options)), main_thread_executor_(std::move(main_thread_executor)) {
  thread_pool_ = ThreadPool::Create(4 /*min_size*/, 256 /*max_size*/, absl::Seconds(1));
  main_thread_id_ = std::this_thread::get_id();
  data_manager_ = std::make_unique<DataManager>(main_thread_id_);
}

OrbitApp::~OrbitApp() {
#ifdef _WIN32
  oqpi_tk::stop_scheduler();
#endif
}

void OrbitApp::OnCaptureStarted() {
  // We need to block until initialization is complete to
  // avoid races when capture thread start processing data.
  absl::Mutex mutex;
  absl::MutexLock mutex_lock(&mutex);
  bool initialization_complete = false;
  captured_address_infos_.clear();

  main_thread_executor_->Schedule([this, &initialization_complete, &mutex] {
    ClearCapture();

    if (capture_started_callback_) {
      capture_started_callback_();
    }

    if (!Capture::capture_data_.selected_functions().empty() && select_live_tab_callback_) {
      select_live_tab_callback_();
    }

    absl::MutexLock lock(&mutex);
    initialization_complete = true;
  });

  bool (*IsTrue)(bool*) = [](bool* value) { return *value; };
  mutex.Await(absl::Condition(IsTrue, &initialization_complete));
}

void OrbitApp::OnCaptureComplete() {
  main_thread_executor_->Schedule([this] {
    Capture::capture_data_.set_address_infos(std::move(captured_address_infos_));
    Capture::FinalizeCapture();

    RefreshCaptureView();

    AddSamplingReport(Capture::GSamplingProfiler);
    AddTopDownView(*Capture::GSamplingProfiler);

    if (capture_stopped_callback_) {
      capture_stopped_callback_();
    }

    FireRefreshCallbacks();
  });
}

void OrbitApp::OnTimer(const TimerInfo& timer_info) { GCurrentTimeGraph->ProcessTimer(timer_info); }

void OrbitApp::OnKeyAndString(uint64_t key, std::string str) {
  string_manager_->AddIfNotPresent(key, std::move(str));
}

void OrbitApp::OnCallstack(CallStack callstack) {
  Capture::GSamplingProfiler->AddUniqueCallStack(callstack);
}

void OrbitApp::OnCallstackEvent(CallstackEvent callstack_event) {
  if (Capture::GSamplingProfiler == nullptr) {
    ERROR("GSamplingProfiler is null, ignoring callstack event.");
    return;
  }
  GEventTracer.GetEventBuffer().AddCallstackEvent(
      callstack_event.time(), callstack_event.callstack_hash(), callstack_event.thread_id());
  Capture::GSamplingProfiler->AddCallStack(std::move(callstack_event));
}

void OrbitApp::OnThreadName(int32_t thread_id, std::string thread_name) {
  main_thread_executor_->Schedule([thread_id, thread_name = std::move(thread_name)]() mutable {
    Capture::capture_data_.AddOrAssignThreadName(thread_id, std::move(thread_name));
  });
}

void OrbitApp::OnAddressInfo(LinuxAddressInfo address_info) {
  uint64_t address = address_info.absolute_address();
  captured_address_infos_.emplace(address, std::move(address_info));
}

void OrbitApp::OnValidateFramePointers(std::vector<std::shared_ptr<Module>> modules_to_validate) {
  thread_pool_->Schedule([modules_to_validate = std::move(modules_to_validate), this] {
    frame_pointer_validator_client_->AnalyzeModules(modules_to_validate);
  });
}

bool OrbitApp::Init(ApplicationOptions&& options,
                    std::unique_ptr<MainThreadExecutor> main_thread_executor) {
  GOrbitApp = std::make_unique<OrbitApp>(std::move(options), std::move(main_thread_executor));

  Path::Init();

  Capture::Init();

#ifdef _WIN32
  oqpi_tk::start_default_scheduler();
#endif

  GOrbitApp->LoadFileMapping();

  return true;
}

void OrbitApp::PostInit() {
  if (!options_.grpc_server_address.empty()) {
    grpc::ChannelArguments channel_arguments;
    // TODO (159888769) move symbol loading to grpc stream.
    // The default receive message size is 4mb. Symbol data can easily be more
    // than this. This is set to an arbitrary size of 2gb (numeric max), which
    // seems to be enough and leaves some headroom. As an example, a 1.1gb
    // .debug symbols file results in a message size of 88mb.
    channel_arguments.SetMaxReceiveMessageSize(std::numeric_limits<int32_t>::max());
    grpc_channel_ = grpc::CreateCustomChannel(
        options_.grpc_server_address, grpc::InsecureChannelCredentials(), channel_arguments);
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
        {
          // TODO: remove this part when client stops using Process class
          absl::MutexLock lock(&process_map_mutex_);
          for (const ProcessInfo& info : process_infos) {
            auto it = process_map_.find(info.pid());
            if (it != process_map_.end()) {
              continue;
            }

            std::shared_ptr<Process> process = std::make_shared<Process>();
            process->SetID(info.pid());
            process->SetName(info.name());
            process->SetFullPath(info.full_path());
            process->SetIs64Bit(info.is_64_bit());
            // The other fields do not appear to be used at the moment.

            process_map_.insert_or_assign(process->GetID(), process);
          }
        }

        if (processes_data_view_->GetSelectedProcessId() == -1 &&
            processes_data_view_->GetFirstProcessId() != -1) {
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
  std::vector<std::string> preset_filenames = Path::ListFiles(Path::GetPresetPath(), ".opr");
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
  NeedsRedraw();
  GOrbitApp->FireRefreshCallbacks();
  DoZoom = true;  // TODO: remove global, review logic
}

void OrbitApp::Disassemble(int32_t pid, const FunctionInfo& function) {
  thread_pool_->Schedule([this, pid, function] {
    auto result = process_manager_->LoadProcessMemory(
        pid, FunctionUtils::GetAbsoluteAddress(function), function.size());
    if (!result.has_value()) {
      SendErrorToUi("Error reading memory", absl::StrFormat("Could not read process memory: %s.",
                                                            result.error().message()));
      return;
    }

    const std::string& memory = result.value();
    Disassembler disasm;
    disasm.AddLine(absl::StrFormat("asm: /* %s */", FunctionUtils::GetDisplayName(function)));
    disasm.Disassemble(memory.data(), memory.size(), FunctionUtils::GetAbsoluteAddress(function),
                       Capture::GTargetProcess->GetIs64Bit());
    if (!sampling_report_ || !sampling_report_->GetProfiler()) {
      DisassemblyReport empty_report(disasm);
      SendDisassemblyToUi(disasm.GetResult(), std::move(empty_report));
      return;
    }
    std::shared_ptr<SamplingProfiler> profiler = sampling_report_->GetProfiler();

    DisassemblyReport report(disasm, FunctionUtils::GetAbsoluteAddress(function), profiler);
    SendDisassemblyToUi(disasm.GetResult(), std::move(report));
  });
}

void OrbitApp::OnExit() {
  StopCapture();

  process_manager_->Shutdown();
  thread_pool_->ShutdownAndWait();

  GOrbitApp = nullptr;
  Orbit_ImGui_Shutdown();
}

Timer GMainTimer;

// TODO: make it non-static
void OrbitApp::MainTick() {
  ORBIT_SCOPE_FUNC;

  GMainTimer.Reset();

  if (DoZoom) {
    GCurrentTimeGraph->SortTracks();
    GOrbitApp->capture_window_->ZoomAll();
    GOrbitApp->NeedsRedraw();
    DoZoom = false;
  }
}

void OrbitApp::RegisterCaptureWindow(CaptureWindow* capture) {
  CHECK(capture_window_ == nullptr);
  capture_window_ = capture;
}

void OrbitApp::NeedsRedraw() {
  if (capture_window_ != nullptr) {
    capture_window_->NeedsUpdate();
  }
}

void OrbitApp::AddSamplingReport(std::shared_ptr<SamplingProfiler> sampling_profiler) {
  auto report = std::make_shared<SamplingReport>(std::move(sampling_profiler));

  if (sampling_reports_callback_) {
    DataView* callstack_data_view = GetOrCreateDataView(DataViewType::kCallstack);
    sampling_reports_callback_(callstack_data_view, report);
  }

  sampling_report_ = report;
}

void OrbitApp::AddSelectionReport(std::shared_ptr<SamplingProfiler> sampling_profiler) {
  auto report = std::make_shared<SamplingReport>(std::move(sampling_profiler));

  if (selection_report_callback_) {
    DataView* callstack_data_view = GetOrCreateDataView(DataViewType::kCallstack);
    selection_report_callback_(callstack_data_view, report);
  }

  selection_report_ = report;
}

void OrbitApp::AddTopDownView(const SamplingProfiler& sampling_profiler) {
  if (!top_down_view_callback_) {
    return;
  }
  std::unique_ptr<TopDownView> top_down_view = TopDownView::CreateFromSamplingProfiler(
      sampling_profiler, Capture::capture_data_.process_name(),
      Capture::capture_data_.thread_names());
  top_down_view_callback_(std::move(top_down_view));
}

std::string OrbitApp::GetCaptureFileName() {
  time_t timestamp =
      std::chrono::system_clock::to_time_t(Capture::capture_data_.capture_start_time());
  std::string result;
  result.append(Path::StripExtension(Capture::capture_data_.process_name()));
  result.append("_");
  result.append(OrbitUtils::FormatTime(timestamp));
  result.append(".orbit");
  return result;
}

std::string OrbitApp::GetCaptureTime() {
  double time = GCurrentTimeGraph != nullptr ? GCurrentTimeGraph->GetCaptureTimeSpanUs() : 0;
  return GetPrettyTime(absl::Microseconds(time));
}

std::string OrbitApp::GetSaveFile(const std::string& extension) {
  if (!save_file_callback_) {
    return "";
  }
  return save_file_callback_(extension);
}

void OrbitApp::SetClipboard(const std::string& text) {
  if (clipboard_callback_) {
    clipboard_callback_(text);
  }
}

ErrorMessageOr<void> OrbitApp::OnSavePreset(const std::string& filename) {
  OUTCOME_TRY(SavePreset(filename));
  ListPresets();
  Refresh(DataViewType::kPresets);
  return outcome::success();
}

ErrorMessageOr<void> OrbitApp::SavePreset(const std::string& filename) {
  PresetInfo preset;
  const int32_t pid = processes_data_view_->GetSelectedProcessId();
  const std::shared_ptr<Process>& process = FindProcessByPid(pid);
  preset.set_process_full_path(data_manager_->GetProcessByPid(pid)->full_path());

  for (uint64_t function_address : data_manager_->selected_functions()) {
    FunctionInfo* func = process->GetFunctionFromAddress(function_address);
    // No need to store the manually instrumented functions
    if (!FunctionUtils::IsOrbitFunc(*func)) {
      uint64_t hash = FunctionUtils::GetHash(*func);
      (*preset.mutable_path_to_module())[func->loaded_module_path()].add_function_hashes(hash);
    }
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
    file_path = Path::JoinPath({Path::GetPresetPath(), filename});
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

void OrbitApp::LoadPreset(const std::shared_ptr<PresetFile>& preset) {
  const std::string& process_full_path = preset->preset_info().process_full_path();
  if (Capture::GTargetProcess->GetFullPath() == process_full_path) {
    // In case we already have the correct process selected
    GOrbitApp->LoadModulesFromPreset(Capture::GTargetProcess, preset);
    return;
  }
  if (!SelectProcess(Path::GetFileName(process_full_path))) {
    SendErrorToUi("Preset loading failed",
                  absl::StrFormat("The process \"%s\" is not running.", process_full_path));
    return;
  }
  Capture::GSessionPresets = preset;
}

ErrorMessageOr<void> OrbitApp::OnSaveCapture(const std::string& file_name) {
  CaptureSerializer ar;
  ar.time_graph_ = GCurrentTimeGraph;
  return ar.Save(file_name);
}

ErrorMessageOr<void> OrbitApp::OnLoadCapture(const std::string& file_name) {
  ClearCapture();

  CaptureSerializer ar;
  ar.time_graph_ = GCurrentTimeGraph;
  OUTCOME_TRY(ar.Load(file_name));

  DoZoom = true;  // TODO: remove global, review logic
  return outcome::success();
}

void OrbitApp::FireRefreshCallbacks(DataViewType type) {
  for (DataView* panel : m_Panels) {
    if (type == DataViewType::kAll || type == panel->GetType()) {
      panel->OnDataChanged();
    }
  }

  if (refresh_callback_) {
    refresh_callback_(type);
  }
}

bool OrbitApp::StartCapture() {
  int32_t pid = processes_data_view_->GetSelectedProcessId();
  std::string process_name = data_manager_->GetProcessByPid(pid)->name();
  absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions =
      GetSelectedFunctionsAndOrbitFunctions();
  ErrorMessageOr<void> result = Capture::StartCapture(pid, process_name, selected_functions);
  if (result.has_error()) {
    SendErrorToUi("Error starting capture", result.error().message());
    return false;
  }

  result = capture_client_->StartCapture(thread_pool_.get(), pid, selected_functions);

  if (!result) {
    SendErrorToUi("Error starting capture", result.error().message());
    return false;
  }

  return true;
}

absl::flat_hash_map<uint64_t, FunctionInfo> OrbitApp::GetSelectedFunctionsAndOrbitFunctions()
    const {
  absl::flat_hash_map<uint64_t, FunctionInfo> selected_functions;
  for (const auto& func : Capture::GTargetProcess->GetFunctions()) {
    if (IsFunctionSelected(*func) || FunctionUtils::IsOrbitFunc(*func)) {
      uint64_t address = FunctionUtils::GetAbsoluteAddress(*func);
      selected_functions[address] = *func;
    }
  }
  return selected_functions;
}

void OrbitApp::StopCapture() {
  if (!capture_client_->StopCapture()) {
    return;
  }

  if (capture_stop_requested_callback_) {
    capture_stop_requested_callback_();
  }

  FireRefreshCallbacks();
}

void OrbitApp::ClearCapture() {
  set_selected_thread_id(-1);
  Capture::ClearCaptureData();

  // Trigger deallocation of previous sampling related data.
  sampling_report_ = nullptr;
  auto empty_sampling_profiler = std::make_shared<SamplingProfiler>();
  AddSamplingReport(empty_sampling_profiler);
  AddTopDownView(*empty_sampling_profiler);

  if (GCurrentTimeGraph != nullptr) {
    GCurrentTimeGraph->Clear();
  }
  GOrbitApp->FireRefreshCallbacks(DataViewType::kLiveFunctions);

  if (capture_cleared_callback_) {
    capture_cleared_callback_();
  }
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

void OrbitApp::SendDisassemblyToUi(std::string disassembly, DisassemblyReport report) {
  main_thread_executor_->Schedule(
      [this, disassembly = std::move(disassembly), report = std::move(report)]() mutable {
        if (disassembly_callback_) {
          disassembly_callback_(std::move(disassembly), std::move(report));
        }
      });
}

void OrbitApp::SendTooltipToUi(const std::string& tooltip) {
  main_thread_executor_->Schedule([this, tooltip] {
    if (tooltip_callback_) {
      tooltip_callback_(tooltip);
    }
  });
}

void OrbitApp::SendInfoToUi(const std::string& title, const std::string& text) {
  main_thread_executor_->Schedule([this, title, text] {
    if (info_message_callback_) {
      info_message_callback_(title, text);
    }
  });
}

void OrbitApp::SendErrorToUi(const std::string& title, const std::string& text) {
  main_thread_executor_->Schedule([this, title, text] {
    if (error_message_callback_) {
      error_message_callback_(title, text);
    }
  });
}

void OrbitApp::LoadModuleOnRemote(int32_t process_id, const std::shared_ptr<Module>& module,
                                  const std::shared_ptr<PresetFile>& preset) {
  ScopedStatus scoped_status =
      CreateScopedStatus(absl::StrFormat("Loading symbols for \"%s\"...", module->m_FullName));
  thread_pool_->Schedule([this, process_id, module, preset,
                          scoped_status = std::move(scoped_status)]() mutable {
    const std::string& module_path = module->m_FullName;
    const std::string& build_id = module->m_DebugSignature;

    scoped_status.UpdateMessage(
        absl::StrFormat("Looking for debug info file for \"%s\"...", module_path));

    const auto result = process_manager_->FindDebugInfoFile(module_path, build_id);

    if (!result) {
      SendErrorToUi("Error loading symbols",
                    absl::StrFormat("Did not find symbols on remote for module \"%s\": %s",
                                    module_path, result.error().message()));
      main_thread_executor_->Schedule(
          [this, module]() { modules_currently_loading_.erase(module->m_FullName); });
      return;
    }

    const std::string& debug_file_path = result.value();

    LOG("Found file on the remote: \"%s\" - loading it using scp...", debug_file_path);

    main_thread_executor_->Schedule([this, module, module_path, build_id, process_id, preset,
                                     debug_file_path,
                                     scoped_status = std::move(scoped_status)]() mutable {
      const std::string local_debug_file_path = symbol_helper_.GenerateCachedFileName(module_path);

      {
        scoped_status.UpdateMessage(
            absl::StrFormat(R"(Copying debug info file for "%s" from remote: "%s"...)", module_path,
                            debug_file_path));
        SCOPE_TIMER_LOG(absl::StrFormat("Copying %s", debug_file_path));
        auto scp_result = secure_copy_callback_(debug_file_path, local_debug_file_path);
        if (!scp_result) {
          SendErrorToUi("Error loading symbols",
                        absl::StrFormat("Could not copy debug info file from the remote: %s",
                                        scp_result.error().message()));
          return;
        }
      }

      scoped_status.UpdateMessage(
          absl::StrFormat(R"(Loading symbols from "%s"...)", debug_file_path));
      const auto result = symbol_helper_.LoadSymbolsFromFile(local_debug_file_path, build_id);
      if (!result) {
        SendErrorToUi(
            "Error loading symbols",
            absl::StrFormat(R"(Did not load symbols for module "%s" (debug info file "%s"): %s)",
                            module_path, local_debug_file_path, result.error().message()));
        return;
      }
      module->LoadSymbols(result.value());
      LOG("Received and loaded %lu function symbols from remote service "
          "for module %s",
          module->m_Pdb->GetFunctions().size(), module->m_Name.c_str());
      SymbolLoadingFinished(process_id, module, preset);
    });
  });
}

void OrbitApp::SymbolLoadingFinished(uint32_t process_id, const std::shared_ptr<Module>& module,
                                     const std::shared_ptr<PresetFile>& preset) {
  if (preset != nullptr) {
    auto it = preset->preset_info().path_to_module().find(module->m_FullName);
    if (it != preset->preset_info().path_to_module().end()) {
      for (const FunctionInfo* func : module->m_Pdb->GetSelectedFunctionsFromPreset(*preset)) {
        SelectFunction(*func);
      }
    }
  }

  data_manager_->FindModuleByAddressStart(process_id, module->m_AddressStart)->set_loaded(true);

  modules_currently_loading_.erase(module->m_FullName);

  UpdateSamplingReport();
  AddTopDownView(*Capture::GSamplingProfiler);
  GOrbitApp->FireRefreshCallbacks();
}

void OrbitApp::LoadModules(int32_t process_id, const std::vector<std::shared_ptr<Module>>& modules,
                           const std::shared_ptr<PresetFile>& preset) {
  // TODO(159868905) use ModuleData instead of Module
  for (const auto& module : modules) {
    if (modules_currently_loading_.contains(module->m_FullName)) {
      continue;
    }
    modules_currently_loading_.insert(module->m_FullName);

    // TODO (159889010) Move symbol loading off the main thread.
    const std::string& module_path = module->m_FullName;
    const std::string& build_id = module->m_DebugSignature;
    auto scoped_status = CreateScopedStatus(
        absl::StrFormat("Trying to find symbols for \"%s\" on local machine...", module_path));
    auto symbols = symbol_helper_.LoadUsingSymbolsPathFile(module_path, build_id);

    // Try loading from the cache
    if (!symbols) {
      const std::string cached_file_name = symbol_helper_.GenerateCachedFileName(module_path);
      symbols = symbol_helper_.LoadSymbolsFromFile(cached_file_name, build_id);
    }

    if (symbols) {
      module->LoadSymbols(symbols.value());
      LOG("Loaded %lu function symbols locally for module \"%s\"",
          symbols.value().symbol_infos().size(), module->m_FullName);
      SymbolLoadingFinished(process_id, module, preset);
    } else {
      LOG("Did not find local symbols for module: %s", module->m_Name);
      LoadModuleOnRemote(process_id, module, preset);
    }
  }
}

void OrbitApp::LoadModulesFromPreset(const std::shared_ptr<Process>& process,
                                     const std::shared_ptr<PresetFile>& preset) {
  std::vector<std::shared_ptr<Module>> modules_to_load;
  std::vector<std::string> modules_not_found;
  for (const auto& pair : preset->preset_info().path_to_module()) {
    const std::string& module_path = pair.first;
    std::shared_ptr<Module> module = process->GetModuleFromPath(module_path);
    if (module == nullptr) {
      modules_not_found.push_back(module_path);
      continue;
    }
    if (module->IsLoaded()) {
      CHECK(module->m_Pdb != nullptr);
      for (const FunctionInfo* func : module->m_Pdb->GetSelectedFunctionsFromPreset(*preset)) {
        SelectFunction(*func);
      }
      continue;
    }
    modules_to_load.emplace_back(std::move(module));
  }
  if (!modules_not_found.empty()) {
    SendErrorToUi(
        "Preset loading incomplete",
        absl::StrFormat("Unable to load the preset for the following modules:\n\"%s\"\nThe "
                        "modules are not loaded by process \"%s\".",
                        absl::StrJoin(modules_not_found, "\"\n\""), process->GetName()));
  }
  if (!modules_to_load.empty()) {
    LoadModules(process->GetID(), modules_to_load, preset);
  }
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

    main_thread_executor_->Schedule([pid, result, this] {
      // Make sure that pid is actually what user has selected at
      // the moment we arrive here. If not - ignore the result.
      const std::vector<ModuleInfo>& module_infos = result.value();
      data_manager_->UpdateModuleInfos(pid, module_infos);
      if (pid != processes_data_view_->GetSelectedProcessId()) {
        return;
      }

      modules_data_view_->SetModules(pid, data_manager_->GetModules(pid));

      // TODO: remove this part when all client code is moved to
      // new data model.
      std::shared_ptr<Process> process = FindProcessByPid(pid);
      CHECK(process != nullptr);

      for (const ModuleInfo& info : module_infos) {
        // if module already exists, don't create it again.
        if (process->GetModuleFromPath(info.file_path()) != nullptr) {
          continue;
        }
        std::shared_ptr<Module> module = std::make_shared<Module>();
        module->m_Name = info.name();
        module->m_FullName = info.file_path();
        module->m_PdbSize = info.file_size();
        module->m_AddressStart = info.address_start();
        module->m_AddressEnd = info.address_end();
        module->m_DebugSignature = info.build_id();
        module->SetLoadable(true);
        process->AddModule(module);
      }

      std::shared_ptr<PresetFile> preset = Capture::GSessionPresets;
      if (preset) {
        LoadModulesFromPreset(process, preset);
        Capture::GSessionPresets = nullptr;
      }
      // To this point ----------------------------------

      // To this point all data is ready. We can set the Process and then
      // propagate the changes to the UI.

      if (pid != Capture::GTargetProcess->GetID()) {
        data_manager_->ClearSelectedFunctions();
        Capture::SetTargetProcess(std::move(process));
      }

      FireRefreshCallbacks();
    });
  });
}

std::shared_ptr<Process> OrbitApp::FindProcessByPid(int32_t pid) {
  absl::MutexLock lock(&process_map_mutex_);
  auto it = process_map_.find(pid);
  if (it == process_map_.end()) {
    return nullptr;
  }

  return it->second;
}

void OrbitApp::SelectFunction(const orbit_client_protos::FunctionInfo& func) {
  uint64_t absolute_address = FunctionUtils::GetAbsoluteAddress(func);
  LOG("Selected %s at 0x%" PRIx64 " (address_=0x%" PRIx64 ", load_bias_= 0x%" PRIx64
      ", base_address=0x%" PRIx64 ")",
      func.pretty_name(), absolute_address, func.address(), func.load_bias(),
      func.module_base_address());
  data_manager_->SelectFunction(absolute_address);
}

void OrbitApp::DeselectFunction(const orbit_client_protos::FunctionInfo& func) {
  uint64_t absolute_address = FunctionUtils::GetAbsoluteAddress(func);
  data_manager_->DeselectFunction(absolute_address);
}

void OrbitApp::ClearSelectedFunctions() { data_manager_->ClearSelectedFunctions(); }

[[nodiscard]] bool OrbitApp::IsFunctionSelected(
    const orbit_client_protos::FunctionInfo& func) const {
  uint64_t absolute_address = FunctionUtils::GetAbsoluteAddress(func);
  return data_manager_->IsFunctionSelected(absolute_address);
}

[[nodiscard]] bool OrbitApp::IsFunctionSelected(const SampledFunction& func) const {
  return data_manager_->IsFunctionSelected(func.address);
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

void OrbitApp::UpdateSamplingReport() {
  if (sampling_report_ != nullptr) {
    sampling_report_->UpdateReport();
  }

  if (selection_report_ != nullptr) {
    selection_report_->UpdateReport();
  }
}

DataView* OrbitApp::GetOrCreateDataView(DataViewType type) {
  switch (type) {
    case DataViewType::kFunctions:
      if (!functions_data_view_) {
        functions_data_view_ = std::make_unique<FunctionsDataView>();
        m_Panels.push_back(functions_data_view_.get());
      }
      return functions_data_view_.get();

    case DataViewType::kCallstack:
      if (!callstack_data_view_) {
        callstack_data_view_ = std::make_unique<CallStackDataView>();
        m_Panels.push_back(callstack_data_view_.get());
      }
      return callstack_data_view_.get();

    case DataViewType::kModules:
      if (!modules_data_view_) {
        modules_data_view_ = std::make_unique<ModulesDataView>();
        m_Panels.push_back(modules_data_view_.get());
      }
      return modules_data_view_.get();

    case DataViewType::kProcesses:
      if (!processes_data_view_) {
        processes_data_view_ = std::make_unique<ProcessesDataView>();
        processes_data_view_->SetSelectionListener(
            [&](int32_t pid) { UpdateProcessAndModuleList(pid); });
        m_Panels.push_back(processes_data_view_.get());
      }
      return processes_data_view_.get();

    case DataViewType::kPresets:
      if (!presets_data_view_) {
        presets_data_view_ = std::make_unique<PresetsDataView>();
        m_Panels.push_back(presets_data_view_.get());
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

    case DataViewType::kInvalid:
      FATAL("DataViewType::kInvalid should not be used with the factory.");
  }

  FATAL("Unreachable");
}

void OrbitApp::FilterTracks(const std::string& filter) {
  GCurrentTimeGraph->SetThreadFilter(filter);
}

void OrbitApp::CrashOrbitService(CrashOrbitServiceRequest_CrashType crash_type) {
  if (absl::GetFlag(FLAGS_devmode)) {
    thread_pool_->Schedule([crash_type, this] { crash_manager_->CrashOrbitService(crash_type); });
  }
}

bool OrbitApp::IsCapturing() const { return capture_client_->IsCapturing(); }

ScopedStatus OrbitApp::CreateScopedStatus(const std::string& initial_message) {
  CHECK(std::this_thread::get_id() == main_thread_id_);
  CHECK(status_listener_ != nullptr);
  return ScopedStatus{main_thread_executor_.get(), status_listener_, initial_message};
}
