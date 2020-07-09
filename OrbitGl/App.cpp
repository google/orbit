// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "App.h"

#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <outcome.hpp>
#include <thread>
#include <utility>

#include "CallStackDataView.h"
#include "Callstack.h"
#include "Capture.h"
#include "CaptureListener.h"
#include "CaptureSerializer.h"
#include "CaptureWindow.h"
#include "Disassembler.h"
#include "EventTracer.h"
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
#include "OrbitSession.h"
#include "Params.h"
#include "Pdb.h"
#include "PrintVar.h"
#include "ProcessesDataView.h"
#include "SamplingProfiler.h"
#include "SamplingReport.h"
#include "ScopeTimer.h"
#include "SessionsDataView.h"
#include "StringManager.h"
#include "TextRenderer.h"
#include "Utils.h"
#include "Version.h"

#if __linux__
#include <OrbitLinuxTracing/OrbitTracing.h>
#endif

ABSL_DECLARE_FLAG(bool, devmode);

std::unique_ptr<OrbitApp> GOrbitApp;
float GFontSize;
bool DoZoom = false;

//-----------------------------------------------------------------------------
OrbitApp::OrbitApp(ApplicationOptions&& options)
    : options_(std::move(options)) {
  main_thread_executor_ = MainThreadExecutor::Create();
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

void OrbitApp::OnTimer(Timer timer) { GCurrentTimeGraph->ProcessTimer(timer); }

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
  Capture::GSamplingProfiler->AddHashedCallStack(callstack_event);
  GEventTracer.GetEventBuffer().AddCallstackEvent(
      callstack_event.m_Time, callstack_event.m_Id, callstack_event.m_TID);
}

void OrbitApp::OnThreadName(int32_t thread_id, std::string thread_name) {
  Capture::GThreadNames.insert_or_assign(thread_id, std::move(thread_name));
}

void OrbitApp::OnAddressInfo(LinuxAddressInfo address_info) {
  uint64_t address = address_info.address;
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
bool OrbitApp::Init(ApplicationOptions&& options) {
  GOrbitApp = std::make_unique<OrbitApp>(std::move(options));

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
            process->SetIsRemote(true);
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
  std::vector<std::shared_ptr<Preset>> presets;
  for (std::string& filename : preset_filenames) {
    auto preset = std::make_shared<Preset>();
    outcome::result<void, std::string> result =
        ReadPresetFromFile(filename, preset.get());
    if (result.has_error()) {
      ERROR("Loading preset failed: \"%s\"", result.error());
      continue;
    }
    preset->m_FileName = filename;
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
void OrbitApp::Disassemble(int32_t pid, const Function& function) {
  thread_pool_->Schedule([this, pid, function] {
    auto result = process_manager_->LoadProcessMemory(
        pid, function.GetVirtualAddress(), function.Size());
    if (!result.has_value()) {
      SendErrorToUi("Error reading memory",
                    absl::StrFormat("Could not read process memory: %s.",
                                    result.error().message));
      return;
    }

    const std::string& memory = result.value();
    Disassembler disasm;
    disasm.LOGF(absl::StrFormat("asm: /* %s */\n", function.PrettyName()));
    disasm.Disassemble(reinterpret_cast<const uint8_t*>(memory.data()),
                       memory.size(), function.GetVirtualAddress(),
                       Capture::GTargetProcess->GetIs64Bit());
    SendDisassemblyToUi(disasm.GetResult());
  });
}

//-----------------------------------------------------------------------------
void OrbitApp::OnExit() {
  if (Capture::GState == Capture::State::kStarted) {
    StopCapture();
  }

  process_manager_->Shutdown();
  thread_pool_->ShutdownAndWait();
  main_thread_executor_->ConsumeActions();

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

  GOrbitApp->main_thread_executor_->ConsumeActions();

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
void OrbitApp::NeedsRedraw() { m_CaptureWindow->NeedsUpdate(); }

//-----------------------------------------------------------------------------
void OrbitApp::AddSamplingReport(
    std::shared_ptr<SamplingProfiler>& sampling_profiler) {
  auto report = std::make_shared<SamplingReport>(sampling_profiler);

  if (sampling_reports_callback_) {
    DataView* callstack_data_view =
        GetOrCreateDataView(DataViewType::CALLSTACK);
    sampling_reports_callback_(callstack_data_view, report);
  }

  sampling_report_ = report;
}

//-----------------------------------------------------------------------------
void OrbitApp::AddSelectionReport(
    std::shared_ptr<SamplingProfiler>& a_SamplingProfiler) {
  auto report = std::make_shared<SamplingReport>(a_SamplingProfiler);

  if (selection_report_callback_) {
    DataView* callstack_data_view =
        GetOrCreateDataView(DataViewType::CALLSTACK);
    selection_report_callback_(callstack_data_view, report);
  }

  selection_report_ = report;
}

//-----------------------------------------------------------------------------
std::string OrbitApp::GetCaptureFileName() {
  CHECK(Capture::GTargetProcess != nullptr);
  time_t timestamp =
      std::chrono::system_clock::to_time_t(Capture::GCaptureTimePoint);
  std::string result;
  result.append(Path::StripExtension(Capture::GTargetProcess->GetName()));
  result.append("_");
  result.append(OrbitUtils::FormatTime(timestamp));
  result.append(".orbit");
  return result;
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
outcome::result<void, std::string> OrbitApp::OnSavePreset(
    const std::string& filename) {
  OUTCOME_TRY(Capture::SavePreset(filename));
  ListPresets();
  Refresh(DataViewType::PRESETS);
  return outcome::success();
}

//-----------------------------------------------------------------------------
outcome::result<void, std::string> OrbitApp::ReadPresetFromFile(
    const std::string& filename, Preset* preset) {
  std::string file_path = filename;

  if (Path::GetDirectory(filename).empty()) {
    file_path = Path::JoinPath({Path::GetPresetPath(), filename});
  }

  std::ifstream file(file_path, std::ios::binary);
  if (file.fail()) {
    ERROR("Loading preset from \"%s\": file.fail()", file_path);
    return outcome::failure("Error opening the file for reading");
  }

  try {
    cereal::BinaryInputArchive archive(file);
    archive(*preset);
    file.close();
    return outcome::success();
  } catch (std::exception& e) {
    ERROR("Loading preset from \"%s\": %s", file_path, e.what());
    return outcome::failure("Error reading the preset");
  }
}

//-----------------------------------------------------------------------------
outcome::result<void, std::string> OrbitApp::OnLoadPreset(
    const std::string& filename) {
  auto preset = std::make_shared<Preset>();
  OUTCOME_TRY(ReadPresetFromFile(filename, preset.get()));
  preset->m_FileName = filename;
  LoadPreset(preset);
  return outcome::success();
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadPreset(const std::shared_ptr<Preset>& preset) {
  if (Capture::GTargetProcess->GetFullPath() == preset->m_ProcessFullPath) {
    // In case we already have the correct process selected
    GOrbitApp->LoadModulesFromPreset(Capture::GTargetProcess, preset);
    return;
  }
  if (!SelectProcess(Path::GetFileName(preset->m_ProcessFullPath))) {
    SendErrorToUi("Session loading failed",
                  absl::StrFormat("The process \"%s\" is not running.",
                                  preset->m_ProcessFullPath));
    return;
  }
  Capture::GSessionPresets = preset;
}

//-----------------------------------------------------------------------------
outcome::result<void, std::string> OrbitApp::OnSaveCapture(
    const std::string& file_name) {
  CaptureSerializer ar;
  ar.time_graph_ = GCurrentTimeGraph;
  return ar.Save(file_name);
}

//-----------------------------------------------------------------------------
outcome::result<void, std::string> OrbitApp::OnLoadCapture(
    const std::string& file_name) {
  Capture::ClearCaptureData();
  GCurrentTimeGraph->Clear();
  if (Capture::GClearCaptureDataFunc) {
    Capture::GClearCaptureDataFunc();
  }

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
  if (Capture::IsCapturing()) {
    LOG("Ignoring Start Capture - already capturing...");
    return false;
  }

  outcome::result<void, std::string> result = Capture::StartCapture();
  if (result.has_error()) {
    SendErrorToUi("Error starting capture", result.error());
    return false;
  }

  int32_t pid = Capture::GTargetProcess->GetID();
  std::vector<std::shared_ptr<Function>> selected_functions =
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
  Capture::StopCapture();

  capture_client_->StopCapture();

  if (capture_stop_requested_callback_) {
    capture_stop_requested_callback_();
  }
  FireRefreshCallbacks();
}

void OrbitApp::OnCaptureStopped() {
  Capture::FinalizeCapture();

  RefreshCaptureView();

  AddSamplingReport(Capture::GSamplingProfiler);

  if (capture_stopped_callback_) {
    capture_stopped_callback_();
  }
  FireRefreshCallbacks();
}

//-----------------------------------------------------------------------------
void OrbitApp::ToggleCapture() {
  if (Capture::IsCapturing()) {
    StopCapture();
  } else {
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
bool OrbitApp::Inject(unsigned long /*a_ProcessId*/) { return false; }

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

void OrbitApp::SendDisassemblyToUi(const std::string& disassembly) {
  main_thread_executor_->Schedule([this, disassembly] {
    if (disassembly_callback_) {
      disassembly_callback_(disassembly);
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
                                  const std::shared_ptr<Preset>& preset) {
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
              module->m_Name, remote_symbols_result.error()));
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

void OrbitApp::SymbolLoadingFinished(uint32_t process_id,
                                     const std::shared_ptr<Module>& module,
                                     const std::shared_ptr<Preset>& preset) {
  if (preset != nullptr &&
      preset->m_Modules.find(module->m_FullName) != preset->m_Modules.end()) {
    module->m_Pdb->ApplyPreset(*preset);
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
                           const std::shared_ptr<Preset>& preset) {
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
void OrbitApp::LoadModulesFromPreset(const std::shared_ptr<Process>& process,
                                     const std::shared_ptr<Preset>& preset) {
  std::vector<std::shared_ptr<Module>> modules;
  for (const auto& pair : preset->m_Modules) {
    const std::string& module_path = pair.first;
    const auto& module = process->GetModuleFromPath(module_path);
    if (module != nullptr && !module->IsLoaded()) {
      modules.emplace_back(std::move(module));
    }
  }
  LoadModules(process->GetID(), modules, preset);
}

//-----------------------------------------------------------------------------

void OrbitApp::OnProcessSelected(int32_t pid) {
  thread_pool_->Schedule([pid, this] {
    outcome::result<std::vector<ModuleInfo>, std::string> result =
        process_manager_->LoadModuleList(pid);

    if (result) {
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
        Capture::SetTargetProcess(process);

        for (const ModuleInfo& info : module_infos) {
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

        std::shared_ptr<Preset> preset = Capture::GSessionPresets;
        if (preset) {
          LoadModulesFromPreset(process, preset);
          Capture::GSessionPresets = nullptr;
        }
        // To this point ----------------------------------

        FireRefreshCallbacks();
      });
    } else {
      ERROR("Error retrieving modules: %s", result.error());
      SendErrorToUi("Error retrieving modules", result.error());
    }
  });
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

    case DataViewType::LIVE_FUNCTIONS:
      if (!m_LiveFunctionsDataView) {
        m_LiveFunctionsDataView = std::make_unique<LiveFunctionsDataView>();
        m_Panels.push_back(m_LiveFunctionsDataView.get());
      }
      return m_LiveFunctionsDataView.get();

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
  m_LiveFunctionsDataView->SetUiFilterString(filter);
}

//-----------------------------------------------------------------------------
void OrbitApp::CrashOrbitService(
    CrashOrbitServiceRequest_CrashType crash_type) {
  if (absl::GetFlag(FLAGS_devmode)) {
    crash_manager_->CrashOrbitService(crash_type);
  }
}
