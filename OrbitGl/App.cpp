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
#include "Injection.h"
#include "Introspection.h"
#include "KeyAndString.h"
#include "LinuxCallstackEvent.h"
#include "LiveFunctionsDataView.h"
#include "Log.h"
#include "ModulesDataView.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Tracing.h"
#include "OrbitVersion.h"
#include "Params.h"
#include "Pdb.h"
#include "PresetsDataView.h"
#include "PrintVar.h"
#include "ProcessesDataView.h"
#include "SamplingProfiler.h"
#include "SamplingReport.h"
#include "SamplingUtils.h"
#include "ScopeTimer.h"
#include "StringManager.h"
#include "TextRenderer.h"
#include "Utils.h"

#if __linux__
#include <OrbitLinuxTracing/OrbitTracing.h>
#endif

ABSL_DECLARE_FLAG(bool, devmode);

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::PresetFile;
using orbit_client_protos::PresetInfo;
using orbit_client_protos::TimerInfo;

std::unique_ptr<OrbitApp> GOrbitApp;
float GFontSize;
bool DoZoom = false;

OrbitApp::OrbitApp(ApplicationOptions&& options,
                   std::unique_ptr<MainThreadExecutor> main_thread_executor)
    : options_(std::move(options)),
      main_thread_executor_(std::move(main_thread_executor)) {
  thread_pool_ =
      ThreadPool::Create(4 /*min_size*/, 256 /*max_size*/, absl::Seconds(1));
  data_manager_ = std::make_unique<DataManager>(std::this_thread::get_id());
}

//-----------------------------------------------------------------------------
OrbitApp::~OrbitApp() {
#ifdef _WIN32
  oqpi_tk::stop_scheduler();
#endif
}

//-----------------------------------------------------------------------------
std::string OrbitApp::FindFile(const std::string& caption,
                               const std::string& dir,
                               const std::string& filter) {
  if (find_file_callback_) {
    return find_file_callback_(caption, dir, filter);
  }

  return std::string();
}

//-----------------------------------------------------------------------------
void OrbitApp::SetCommandLineArguments(const std::vector<std::string>& a_Args) {
  for (const std::string& arg : a_Args) {
    if (absl::StrContains(arg, "preset:")) {
      std::vector<std::string> vec = absl::StrSplit(arg, ":");
      if (vec.size() > 1) {
        Capture::GPresetToLoad = vec[1];
      }
    } else if (absl::StrContains(arg, "inject:")) {
      std::vector<std::string> vec = absl::StrSplit(arg, ":");
      if (vec.size() > 1) {
        Capture::GProcessToInject = vec[1];
      }
    }
  }
}

void OrbitApp::OnTimer(const TimerInfo& timer_info) {
  GCurrentTimeGraph->ProcessTimer(timer_info);
}

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
  Capture::GSamplingProfiler->AddCallStack(callstack_event);
  GEventTracer.GetEventBuffer().AddCallstackEvent(
      callstack_event.time(), callstack_event.callstack_hash(),
      callstack_event.thread_id());
}

void OrbitApp::OnThreadName(int32_t thread_id, std::string thread_name) {
  Capture::GThreadNames.insert_or_assign(thread_id, std::move(thread_name));
}

void OrbitApp::OnAddressInfo(LinuxAddressInfo address_info) {
  uint64_t address = address_info.absolute_address();
  Capture::GAddressInfos.emplace(address, std::move(address_info));
}

//-----------------------------------------------------------------------------
void OrbitApp::OnValidateFramePointers(
    std::vector<std::shared_ptr<Module>> modules_to_validate) {
  thread_pool_->Schedule([modules_to_validate, this] {
    frame_pointer_validator_client_->AnalyzeModules(modules_to_validate);
  });
}

//-----------------------------------------------------------------------------
bool OrbitApp::Init(ApplicationOptions&& options,
                    std::unique_ptr<MainThreadExecutor> main_thread_executor) {
  GOrbitApp = std::make_unique<OrbitApp>(std::move(options),
                                         std::move(main_thread_executor));

  Path::Init();

  Capture::Init();

#ifdef _WIN32
  oqpi_tk::start_default_scheduler();
#endif

  GFontSize = GParams.font_size;
  GOrbitApp->LoadFileMapping();

  return true;
}

//-----------------------------------------------------------------------------
void OrbitApp::PostInit() {
  if (!options_.grpc_server_address.empty()) {
    grpc::ChannelArguments channel_arguments;
    // TODO (159888769) move symbol loading to grpc stream.
    // The default receive message size is 4mb. Symbol data can easily be more
    // than this. This is set to an arbitrary size of 2gb (numeric max), which
    // seems to be enough and leaves some headroom. As an example, a 1.1gb
    // .debug symbols file results in a message size of 88mb.
    channel_arguments.SetMaxReceiveMessageSize(
        std::numeric_limits<int32_t>::max());
    grpc_channel_ = grpc::CreateCustomChannel(
        options_.grpc_server_address, grpc::InsecureChannelCredentials(),
        channel_arguments);
    if (!grpc_channel_) {
      ERROR("Unable to create GRPC channel to %s",
            options_.grpc_server_address);
    }

    capture_client_ = std::make_unique<CaptureClient>(grpc_channel_, this);

    // TODO: Replace refresh_timeout with config option. Let users to modify it.
    process_manager_ =
        ProcessManager::Create(grpc_channel_, absl::Milliseconds(1000));

    auto callback = [this](ProcessManager* process_manager) {
      main_thread_executor_->Schedule([this, process_manager]() {
        const std::vector<ProcessInfo>& process_infos =
            process_manager->GetProcessList();
        data_manager_->UpdateProcessInfos(process_infos);
        m_ProcessesDataView->SetProcessList(process_infos);
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

        if (m_ProcessesDataView->GetSelectedProcessId() == -1 &&
            m_ProcessesDataView->GetFirstProcessId() != -1) {
          m_ProcessesDataView->SelectProcess(
              m_ProcessesDataView->GetFirstProcessId());
        }
        FireRefreshCallbacks(DataViewType::PROCESSES);
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

//-----------------------------------------------------------------------------
void OrbitApp::LoadFileMapping() {
  m_FileMapping.clear();
  std::string fileName = Path::GetFileMappingFileName();
  if (!Path::FileExists(fileName)) {
    std::ofstream outfile(fileName);
    outfile << "//-------------------" << std::endl
            << "// Orbit File Mapping" << std::endl
            << "//-------------------" << std::endl
            << "// If the file path in the pdb is \"D:\\NoAccess\\File.cpp\""
            << std::endl
            << "// and File.cpp is locally available in \"C:\\Available\\\""
            << std::endl
            << "// then enter a file mapping on its own line like so:"
            << std::endl
            << "// \"D:\\NoAccess\\File.cpp\" \"C:\\Available\\\"" << std::endl
            << std::endl
            << "\"D:\\NoAccess\" \"C:\\Available\"" << std::endl;

    outfile.close();
  }

  std::fstream infile(fileName);
  if (!infile.fail()) {
    std::string line;
    while (std::getline(infile, line)) {
      if (absl::StartsWith(line, "//")) continue;

      bool containsQuotes = absl::StrContains(line, "\"");

      std::vector<std::string> tokens = absl::StrSplit(line, ' ');

      if (tokens.size() == 2 && !containsQuotes) {
        m_FileMapping[ToLower(tokens[0])] = ToLower(tokens[1]);
      } else {
        std::vector<std::string> validTokens;
        std::vector<std::string> tokens = absl::StrSplit(line, '"');
        for (const std::string& token : tokens) {
          if (!IsBlank(token)) {
            validTokens.push_back(token);
          }
        }

        if (validTokens.size() > 1) {
          m_FileMapping[ToLower(validTokens[0])] = ToLower(validTokens[1]);
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::ListPresets() {
  std::vector<std::string> preset_filenames =
      Path::ListFiles(Path::GetPresetPath(), ".opr");
  std::vector<std::shared_ptr<PresetFile>> presets;
  for (std::string& filename : preset_filenames) {
    ErrorMessageOr<PresetInfo> preset_result = ReadPresetFromFile(filename);
    if (preset_result.has_error()) {
      ERROR("Loading preset from \"%s\" failed: %s", filename,
            preset_result.error().message());
      continue;
    }

    auto preset = std::make_shared<PresetFile>();
    preset->set_file_name(filename);
    preset->mutable_preset_info()->CopyFrom(preset_result.value());
    presets.push_back(preset);
  }

  m_PresetsDataView->SetPresets(presets);
}

//-----------------------------------------------------------------------------
void OrbitApp::RefreshCaptureView() {
  NeedsRedraw();
  GOrbitApp->FireRefreshCallbacks();
  DoZoom = true;  // TODO: remove global, review logic
}

//-----------------------------------------------------------------------------
void OrbitApp::Disassemble(int32_t pid, const FunctionInfo& function) {
  thread_pool_->Schedule([this, pid, function] {
    auto result = process_manager_->LoadProcessMemory(
        pid, FunctionUtils::GetAbsoluteAddress(function), function.size());
    if (!result.has_value()) {
      SendErrorToUi("Error reading memory",
                    absl::StrFormat("Could not read process memory: %s.",
                                    result.error().message()));
      return;
    }

    const std::string& memory = result.value();
    Disassembler disasm;
    disasm.LOGF(absl::StrFormat("asm: /* %s */\n",
                                FunctionUtils::GetDisplayName(function)));
    disasm.Disassemble(reinterpret_cast<const uint8_t*>(memory.data()),
                       memory.size(),
                       FunctionUtils::GetAbsoluteAddress(function),
                       Capture::GTargetProcess->GetIs64Bit());
    if (!sampling_report_ || !sampling_report_->GetProfiler()) {
      DisassemblyReport empty_report(disasm);
      SendDisassemblyToUi(disasm.GetResult(), std::move(empty_report));
      return;
    }
    std::shared_ptr<SamplingProfiler> profiler =
        sampling_report_->GetProfiler();

    DisassemblyReport report(
        disasm, FunctionUtils::GetAbsoluteAddress(function), profiler);
    SendDisassemblyToUi(disasm.GetResult(), std::move(report));
  });
}

//-----------------------------------------------------------------------------
void OrbitApp::OnExit() {
  if (Capture::GState == Capture::State::kStarted) {
    StopCapture();
  }

  process_manager_->Shutdown();
  thread_pool_->ShutdownAndWait();

  GOrbitApp = nullptr;
  Orbit_ImGui_Shutdown();
}

//-----------------------------------------------------------------------------
Timer GMainTimer;

//-----------------------------------------------------------------------------
// TODO: make it non-static
void OrbitApp::MainTick() {
  ORBIT_SCOPE_FUNC;
  TRACE_VAR(GMainTimer.QueryMillis());

  GMainTimer.Reset();

  ++GOrbitApp->m_NumTicks;

  if (DoZoom) {
    GCurrentTimeGraph->SortTracks();
    GOrbitApp->m_CaptureWindow->ZoomAll();
    GOrbitApp->NeedsRedraw();
    DoZoom = false;
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterCaptureWindow(CaptureWindow* a_Capture) {
  CHECK(m_CaptureWindow == nullptr);
  m_CaptureWindow = a_Capture;
}

//-----------------------------------------------------------------------------
void OrbitApp::NeedsRedraw() {
  if (m_CaptureWindow != nullptr) {
    m_CaptureWindow->NeedsUpdate();
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::AddSamplingReport(
    std::shared_ptr<SamplingProfiler> sampling_profiler) {
  auto report = std::make_shared<SamplingReport>(std::move(sampling_profiler));

  if (sampling_reports_callback_) {
    DataView* callstack_data_view =
        GetOrCreateDataView(DataViewType::CALLSTACK);
    sampling_reports_callback_(callstack_data_view, report);
  }

  sampling_report_ = report;
}

//-----------------------------------------------------------------------------
void OrbitApp::AddSelectionReport(
    std::shared_ptr<SamplingProfiler> a_SamplingProfiler) {
  auto report = std::make_shared<SamplingReport>(std::move(a_SamplingProfiler));

  if (selection_report_callback_) {
    DataView* callstack_data_view =
        GetOrCreateDataView(DataViewType::CALLSTACK);
    selection_report_callback_(callstack_data_view, report);
  }

  selection_report_ = report;
}

void OrbitApp::AddTopDownView(const SamplingProfiler& sampling_profiler) {
  if (!top_down_view_callback_) {
    return;
  }
  std::unique_ptr<TopDownView> top_down_view =
      TopDownView::CreateFromSamplingProfiler(
          sampling_profiler, Capture::GProcessName, Capture::GThreadNames,
          Capture::GAddressToFunctionName);
  top_down_view_callback_(std::move(top_down_view));
}

//-----------------------------------------------------------------------------
std::string OrbitApp::GetCaptureFileName() {
  time_t timestamp =
      std::chrono::system_clock::to_time_t(Capture::GCaptureTimePoint);
  std::string result;
  result.append(Path::StripExtension(Capture::GProcessName));
  result.append("_");
  result.append(OrbitUtils::FormatTime(timestamp));
  result.append(".orbit");
  return result;
}

std::string OrbitApp::GetCaptureTime() {
  double time =
      GCurrentTimeGraph ? GCurrentTimeGraph->GetCaptureTimeSpanUs() : 0;
  return GetPrettyTime(absl::Microseconds(time));
}

//-----------------------------------------------------------------------------
std::string OrbitApp::GetSaveFile(const std::string& extension) {
  if (!save_file_callback_) {
    return "";
  }
  return save_file_callback_(extension);
}

//-----------------------------------------------------------------------------
void OrbitApp::SetClipboard(const std::string& text) {
  if (clipboard_callback_) {
    clipboard_callback_(text);
  }
}

//-----------------------------------------------------------------------------
ErrorMessageOr<void> OrbitApp::OnSavePreset(const std::string& filename) {
  OUTCOME_TRY(Capture::SavePreset(filename));
  ListPresets();
  Refresh(DataViewType::PRESETS);
  return outcome::success();
}

//-----------------------------------------------------------------------------
ErrorMessageOr<PresetInfo> OrbitApp::ReadPresetFromFile(
    const std::string& filename) {
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

//-----------------------------------------------------------------------------
ErrorMessageOr<void> OrbitApp::OnLoadPreset(const std::string& filename) {
  OUTCOME_TRY(preset_info, ReadPresetFromFile(filename));

  auto preset = std::make_shared<PresetFile>();
  preset->set_file_name(filename);
  preset->mutable_preset_info()->CopyFrom(preset_info);
  LoadPreset(preset);
  return outcome::success();
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadPreset(const std::shared_ptr<PresetFile>& preset) {
  const std::string& process_full_path =
      preset->preset_info().process_full_path();
  if (Capture::GTargetProcess->GetFullPath() == process_full_path) {
    // In case we already have the correct process selected
    GOrbitApp->LoadModulesFromPreset(Capture::GTargetProcess, preset);
    return;
  }
  if (!SelectProcess(Path::GetFileName(process_full_path))) {
    SendErrorToUi("Preset loading failed",
                  absl::StrFormat("The process \"%s\" is not running.",
                                  process_full_path));
    return;
  }
  Capture::GSessionPresets = preset;
}

//-----------------------------------------------------------------------------
ErrorMessageOr<void> OrbitApp::OnSaveCapture(const std::string& file_name) {
  CaptureSerializer ar;
  ar.time_graph_ = GCurrentTimeGraph;
  return ar.Save(file_name);
}

//-----------------------------------------------------------------------------
ErrorMessageOr<void> OrbitApp::OnLoadCapture(const std::string& file_name) {
  ClearCapture();

  CaptureSerializer ar;
  ar.time_graph_ = GCurrentTimeGraph;
  OUTCOME_TRY(ar.Load(file_name));

  DoZoom = true;  // TODO: remove global, review logic
  return outcome::success();
}

//-----------------------------------------------------------------------------
void OrbitApp::FireRefreshCallbacks(DataViewType type) {
  for (DataView* panel : m_Panels) {
    if (type == DataViewType::ALL || type == panel->GetType()) {
      panel->OnDataChanged();
    }
  }

  if (refresh_callback_) {
    refresh_callback_(type);
  }
}

bool OrbitApp::StartCapture() {
  CHECK(!Capture::IsCapturing());

  ClearCapture();

  ErrorMessageOr<void> result = Capture::StartCapture();
  if (result.has_error()) {
    SendErrorToUi("Error starting capture", result.error().message());
    return false;
  }

  int32_t pid = Capture::GProcessId;
  std::vector<std::shared_ptr<FunctionInfo>> selected_functions =
      Capture::GSelectedFunctions;
  thread_pool_->Schedule([this, pid, selected_functions] {
    capture_client_->Capture(pid, selected_functions);
    main_thread_executor_->Schedule([this] { OnCaptureStopped(); });
  });

  if (capture_started_callback_) {
    capture_started_callback_();
  }
  if (!Capture::GSelectedFunctionsMap.empty() && select_live_tab_callback_) {
    select_live_tab_callback_();
  }
  return true;
}

//-----------------------------------------------------------------------------
void OrbitApp::StopCapture() {
  CHECK(Capture::GState == Capture::State::kStarted);
  Capture::StopCapture();

  capture_client_->StopCapture();

  if (capture_stop_requested_callback_) {
    capture_stop_requested_callback_();
  }
  FireRefreshCallbacks();
}

void OrbitApp::ClearCapture() {
  CHECK(!Capture::IsCapturing());
  Capture::ClearCaptureData();
  Capture::GClearCaptureDataFunc();
  GCurrentTimeGraph->Clear();
  if (capture_cleared_callback_) {
    capture_cleared_callback_();
  }
}

void OrbitApp::ToggleDrawHelp() {
  if (m_CaptureWindow) {
    m_CaptureWindow->ToggleDrawHelp();
  }
}

void OrbitApp::OnCaptureStopped() {
  Capture::FinalizeCapture();

  RefreshCaptureView();

  AddSamplingReport(Capture::GSamplingProfiler);
  AddTopDownView(*Capture::GSamplingProfiler);

  if (capture_stopped_callback_) {
    capture_stopped_callback_();
  }
  FireRefreshCallbacks();
}

//-----------------------------------------------------------------------------
void OrbitApp::ToggleCapture() {
  if (Capture::GState == Capture::State::kStarted) {
    StopCapture();
  } else if (Capture::GState == Capture::State::kDone ||
             Capture::GState == Capture::State::kEmpty) {
    StartCapture();
  }
}

//-----------------------------------------------------------------------------
bool OrbitApp::SelectProcess(const std::string& a_Process) {
  if (m_ProcessesDataView) {
    return m_ProcessesDataView->SelectProcess(a_Process);
  }

  return false;
}

//-----------------------------------------------------------------------------
bool OrbitApp::SelectProcess(int32_t a_ProcessID) {
  if (m_ProcessesDataView) {
    return m_ProcessesDataView->SelectProcess(a_ProcessID);
  }

  return false;
}

//-----------------------------------------------------------------------------
void OrbitApp::SetCallStack(std::shared_ptr<CallStack> a_CallStack) {
  m_CallStackDataView->SetCallStack(std::move(a_CallStack));
  FireRefreshCallbacks(DataViewType::CALLSTACK);
}

void OrbitApp::RequestOpenCaptureToUi() {
  main_thread_executor_->Schedule([this] {
    if (open_capture_callback_) {
      open_capture_callback_();
    }
  });
}

void OrbitApp::RequestSaveCaptureToUi() {
  main_thread_executor_->Schedule([this] {
    if (save_capture_callback_) {
      save_capture_callback_();
    }
  });
}

void OrbitApp::SendDisassemblyToUi(std::string disassembly,
                                   DisassemblyReport report) {
  main_thread_executor_->Schedule([this, disassembly = std::move(disassembly),
                                   report = std::move(report)]() mutable {
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

void OrbitApp::RequestFeedbackDialogToUi() {
  main_thread_executor_->Schedule([this] {
    if (feedback_dialog_callback_) {
      feedback_dialog_callback_();
    }
  });
}

//-----------------------------------------------------------------------------
void OrbitApp::SendInfoToUi(const std::string& title, const std::string& text) {
  main_thread_executor_->Schedule([this, title, text] {
    if (info_message_callback_) {
      info_message_callback_(title, text);
    }
  });
}

//-----------------------------------------------------------------------------
void OrbitApp::SendErrorToUi(const std::string& title,
                             const std::string& text) {
  main_thread_executor_->Schedule([this, title, text] {
    if (error_message_callback_) {
      error_message_callback_(title, text);
    }
  });
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadModuleOnRemote(int32_t process_id,
                                  const std::shared_ptr<Module>& module,
                                  const std::shared_ptr<PresetFile>& preset) {
  thread_pool_->Schedule([this, process_id, module, preset]() {
    const auto remote_symbols_result =
        process_manager_->LoadSymbols(module->m_FullName);

    if (!remote_symbols_result) {
      SendErrorToUi(
          "Error loading symbols",
          absl::StrFormat(
              "Did not find symbols on local machine for module \"%s\".\n"
              "Trying to load symbols from remote resulted in error "
              "message: %s",
              module->m_Name, remote_symbols_result.error().message()));
      return;
    }

    main_thread_executor_->Schedule(
        [this, symbols = std::move(remote_symbols_result.value()), module,
         process_id, preset]() {
          module->LoadSymbols(symbols);
          LOG("Received and loaded %lu function symbols from remote service "
              "for module %s",
              module->m_Pdb->GetFunctions().size(), module->m_Name.c_str());
          SymbolLoadingFinished(process_id, module, preset);
        });
  });
}

void OrbitApp::SymbolLoadingFinished(
    uint32_t process_id, const std::shared_ptr<Module>& module,
    const std::shared_ptr<PresetFile>& preset) {
  if (preset != nullptr) {
    auto it = preset->preset_info().path_to_module().find(module->m_FullName);
    if (it != preset->preset_info().path_to_module().end()) {
      module->m_Pdb->ApplyPreset(*preset);
    }
  }

  data_manager_->FindModuleByAddressStart(process_id, module->m_AddressStart)
      ->set_loaded(true);

  modules_currently_loading_.erase(module->m_FullName);

  UpdateSamplingReport();
  GOrbitApp->FireRefreshCallbacks();
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadModules(int32_t process_id,
                           const std::vector<std::shared_ptr<Module>>& modules,
                           const std::shared_ptr<PresetFile>& preset) {
  // TODO(159868905) use ModuleData instead of Module
  for (const auto& module : modules) {
    if (modules_currently_loading_.contains(module->m_FullName)) {
      continue;
    }
    modules_currently_loading_.insert(module->m_FullName);

    // TODO (159889010) Move symbol loading off the main thread.
    const auto symbols = symbol_helper_.LoadUsingSymbolsPathFile(
        module->m_FullName, module->m_DebugSignature);

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

//-----------------------------------------------------------------------------
void OrbitApp::LoadModulesFromPreset(
    const std::shared_ptr<Process>& process,
    const std::shared_ptr<PresetFile>& preset) {
  std::vector<std::shared_ptr<Module>> modules_to_load;
  std::vector<std::string> modules_not_found;
  for (const auto& pair : preset->preset_info().path_to_module()) {
    const std::string& module_path = pair.first;
    const auto& module = process->GetModuleFromPath(module_path);
    if (module == nullptr) {
      modules_not_found.push_back(module_path);
      continue;
    }
    if (module->IsLoaded()) {
      CHECK(module->m_Pdb != nullptr);
      module->m_Pdb->ApplyPreset(*preset);
      continue;
    }
    modules_to_load.emplace_back(std::move(module));
  }
  if (!modules_not_found.empty()) {
    SendErrorToUi(
        "Preset loading incomplete",
        absl::StrFormat(
            "Unable to load the preset for the following modules:\n\"%s\"\nThe "
            "modules are not loaded by process \"%s\".",
            absl::StrJoin(modules_not_found, "\"\n\""), process->GetName()));
  }
  if (!modules_to_load.empty()) {
    LoadModules(process->GetID(), modules_to_load, preset);
  }
}

void OrbitApp::UpdateModuleList(int32_t pid) {
  thread_pool_->Schedule([pid, this] {
    ErrorMessageOr<std::vector<ModuleInfo>> result =
        process_manager_->LoadModuleList(pid);

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
      if (pid != m_ProcessesDataView->GetSelectedProcessId()) {
        return;
      }

      m_ModulesDataView->SetModules(pid, data_manager_->GetModules(pid));

      // TODO: remove this part when all client code is moved to
      // new data model.
      std::shared_ptr<Process> process = FindProcessByPid(pid);

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

      FireRefreshCallbacks();
    });
  });
}

void OrbitApp::OnProcessSelected(int32_t pid) {
  CHECK(m_ProcessesDataView->GetSelectedProcessId() == pid);

  std::shared_ptr<Process> process = FindProcessByPid(pid);
  CHECK(process != nullptr);
  Capture::SetTargetProcess(std::move(process));

  // Update modules when process changed
  UpdateModuleList(pid);
}

//-----------------------------------------------------------------------------
std::shared_ptr<Process> OrbitApp::FindProcessByPid(int32_t pid) {
  absl::MutexLock lock(&process_map_mutex_);
  auto it = process_map_.find(pid);
  if (it == process_map_.end()) {
    return nullptr;
  }

  return it->second;
}

void OrbitApp::UpdateSamplingReport() {
  if (sampling_report_ != nullptr) {
    sampling_report_->UpdateReport();
  }

  if (selection_report_ != nullptr) {
    selection_report_->UpdateReport();
  }
}

//-----------------------------------------------------------------------------
DataView* OrbitApp::GetOrCreateDataView(DataViewType type) {
  switch (type) {
    case DataViewType::FUNCTIONS:
      if (!m_FunctionsDataView) {
        m_FunctionsDataView = std::make_unique<FunctionsDataView>();
        m_Panels.push_back(m_FunctionsDataView.get());
      }
      return m_FunctionsDataView.get();

    case DataViewType::CALLSTACK:
      if (!m_CallStackDataView) {
        m_CallStackDataView = std::make_unique<CallStackDataView>();
        m_Panels.push_back(m_CallStackDataView.get());
      }
      return m_CallStackDataView.get();

    case DataViewType::MODULES:
      if (!m_ModulesDataView) {
        m_ModulesDataView = std::make_unique<ModulesDataView>();
        m_Panels.push_back(m_ModulesDataView.get());
      }
      return m_ModulesDataView.get();

    case DataViewType::PROCESSES:
      if (!m_ProcessesDataView) {
        m_ProcessesDataView = std::make_unique<ProcessesDataView>();
        m_ProcessesDataView->SetSelectionListener(
            [&](int32_t pid) { OnProcessSelected(pid); });
        m_Panels.push_back(m_ProcessesDataView.get());
      }
      return m_ProcessesDataView.get();

    case DataViewType::PRESETS:
      if (!m_PresetsDataView) {
        m_PresetsDataView = std::make_unique<PresetsDataView>();
        m_Panels.push_back(m_PresetsDataView.get());
      }
      return m_PresetsDataView.get();

    case DataViewType::SAMPLING:
      FATAL(
          "DataViewType::SAMPLING Data View construction is not supported by"
          "the factory.");
    case DataViewType::LIVE_FUNCTIONS:
      FATAL(
          "DataViewType::LIVE_FUNCTIONS should not be used with the factory.");

    case DataViewType::ALL:
      FATAL("DataViewType::ALL should not be used with the factory.");

    case DataViewType::INVALID:
      FATAL("DataViewType::INVALID should not be used with the factory.");
  }

  FATAL("Unreachable");
}

//-----------------------------------------------------------------------------
void OrbitApp::FilterFunctions(const std::string& filter) {
  Capture::GFunctionFilter = filter;
}

//-----------------------------------------------------------------------------
void OrbitApp::FilterTracks(const std::string& filter) {
  GCurrentTimeGraph->SetThreadFilter(filter);
}

//-----------------------------------------------------------------------------
void OrbitApp::CrashOrbitService(
    CrashOrbitServiceRequest_CrashType crash_type) {
  if (absl::GetFlag(FLAGS_devmode)) {
    crash_manager_->CrashOrbitService(crash_type);
  }
}
