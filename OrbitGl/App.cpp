// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// clang-format off
#include "OrbitAsio.h"
// clang-format on

#include "App.h"

#include <chrono>
#include <cmath>
#include <fstream>
#include <thread>
#include <utility>
#include <outcome.hpp>

#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Tracing.h"

#include "CallStackDataView.h"
#include "Callstack.h"
#include "Capture.h"
#include "CaptureSerializer.h"
#include "CaptureWindow.h"
#include "ConnectionManager.h"
#include "Debugger.h"
#include "Disassembler.h"
#include "EventTracer.h"
#include "FunctionsDataView.h"
#include "GlCanvas.h"
#include "GlobalsDataView.h"
#include "ImGuiOrbit.h"
#include "Injection.h"
#include "Introspection.h"
#include "KeyAndString.h"
#include "LinuxCallstackEvent.h"
#include "LiveFunctionsDataView.h"
#include "Log.h"
#include "LogDataView.h"
#include "ModulesDataView.h"
#include "OrbitAsm.h"
#include "OrbitSession.h"
#include "Params.h"
#include "Pdb.h"
#include "PluginManager.h"
#include "PrintVar.h"
#include "ProcessesDataView.h"
#include "SamplingProfiler.h"
#include "SamplingReport.h"
#include "ScopeTimer.h"
#include "Serialization.h"
#include "SessionsDataView.h"
#include "StringManager.h"
#include "Systrace.h"
#include "Tcp.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "TextRenderer.h"
#include "TimerManager.h"
#include "TypesDataView.h"
#include "Utils.h"
#include "Version.h"

#define GLUT_DISABLE_ATEXIT_HACK
#include "GL/freeglut.h"

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
  thread_pool_ = ThreadPool::Create(4 /*min_size*/, 256 /*max_size*/);
  data_manager_ = std::make_unique<DataManager>(std::this_thread::get_id());
#ifdef _WIN32
  m_Debugger = std::make_unique<Debugger>();
#endif
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
  if (m_FindFileCallback) {
    return m_FindFileCallback(caption, dir, filter);
  }

  return std::string();
}

//-----------------------------------------------------------------------------
void OrbitApp::SetCommandLineArguments(const std::vector<std::string>& a_Args) {
  m_Arguments = a_Args;

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
    } else if (absl::StrContains(arg, "systrace:")) {
      m_PostInitArguments.push_back(arg);
    }
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::ProcessTimer(const Timer& timer) {
  GCurrentTimeGraph->ProcessTimer(timer);
}

//-----------------------------------------------------------------------------
void OrbitApp::ProcessSamplingCallStack(LinuxCallstackEvent& a_CallStack) {
  Capture::GSamplingProfiler->AddCallStack(a_CallStack.callstack_);
  GEventTracer.GetEventBuffer().AddCallstackEvent(
      a_CallStack.time_, a_CallStack.callstack_.m_Hash,
      a_CallStack.callstack_.m_ThreadId);
}

//-----------------------------------------------------------------------------
void OrbitApp::ProcessHashedSamplingCallStack(CallstackEvent& a_CallStack) {
  if (Capture::GSamplingProfiler == nullptr) {
    ERROR("GSamplingProfiler is null, ignoring callstack event.");
    return;
  }
  Capture::GSamplingProfiler->AddHashedCallStack(a_CallStack);
  GEventTracer.GetEventBuffer().AddCallstackEvent(
      a_CallStack.m_Time, a_CallStack.m_Id, a_CallStack.m_TID);
}

//-----------------------------------------------------------------------------
void OrbitApp::AddAddressInfo(LinuxAddressInfo address_info) {
  uint64_t address = address_info.address;
  Capture::GAddressInfos.emplace(address, std::move(address_info));
}

//-----------------------------------------------------------------------------
void OrbitApp::AddKeyAndString(uint64_t key, std::string_view str) {
  string_manager_->AddIfNotPresent(key, str);
}

//-----------------------------------------------------------------------------
void OrbitApp::UpdateThreadName(int32_t thread_id,
                                const std::string& thread_name) {
  Capture::GTargetProcess->SetThreadName(thread_id, thread_name);
}

//-----------------------------------------------------------------------------
void OrbitApp::OnValidateFramePointers(
    std::vector<std::shared_ptr<Module>> modules_to_validate) {
  thread_pool_->Schedule([modules_to_validate, this] {
    frame_pointer_validator_client_->AnalyzeModules(modules_to_validate);
  });
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadSystrace(const std::string& a_FileName) {
  SystraceManager::Get().Clear();
  Capture::ClearCaptureData();
  GCurrentTimeGraph->Clear();
  if (Capture::GClearCaptureDataFunc) {
    Capture::GClearCaptureDataFunc();
  }

  std::shared_ptr<Systrace> systrace =
      std::make_shared<Systrace>(a_FileName.c_str());

  for (Function& func : systrace->GetFunctions()) {
    Capture::GSelectedFunctionsMap[func.GetVirtualAddress()] = &func;
  }
  Capture::GVisibleFunctionsMap = Capture::GSelectedFunctionsMap;

  for (const auto& timer : systrace->GetTimers()) {
    GCurrentTimeGraph->ProcessTimer(timer);
    ++Capture::GFunctionCountMap[timer.m_FunctionAddress];
  }

  for (const auto& pair : systrace->GetThreadNames()) {
    Capture::GTargetProcess->SetThreadName(pair.first, pair.second);
  }

  SystraceManager::Get().Add(systrace);
  GOrbitApp->FireRefreshCallbacks();
  GOrbitApp->StopCapture();
  DoZoom = true;  // TODO: remove global, review logic
}

//-----------------------------------------------------------------------------
void OrbitApp::AppendSystrace(const std::string& a_FileName,
                              uint64_t a_TimeOffset) {
  std::shared_ptr<Systrace> systrace =
      std::make_shared<Systrace>(a_FileName.c_str(), a_TimeOffset);

  for (Function& func : systrace->GetFunctions()) {
    Capture::GSelectedFunctionsMap[func.GetVirtualAddress()] = &func;
  }
  Capture::GVisibleFunctionsMap = Capture::GSelectedFunctionsMap;

  for (const auto& timer : systrace->GetTimers()) {
    GCurrentTimeGraph->ProcessTimer(timer);
    Capture::GFunctionCountMap[timer.m_FunctionAddress];
  }

  for (const auto& pair : systrace->GetThreadNames()) {
    Capture::GTargetProcess->SetThreadName(pair.first, pair.second);
  }

  SystraceManager::Get().Add(systrace);
  GOrbitApp->FireRefreshCallbacks();
  GOrbitApp->StopCapture();
  DoZoom = true;  // TODO: remove global, review logic
}

//-----------------------------------------------------------------------------
bool OrbitApp::Init(ApplicationOptions&& options) {
  GOrbitApp = std::make_unique<OrbitApp>(std::move(options));
  GCoreApp = GOrbitApp.get();

  GTimerManager = std::make_unique<TimerManager>();
  GTcpServer = std::make_unique<TcpServer>();

  Path::Init();

  Capture::Init();

#ifdef _WIN32
  oqpi_tk::start_default_scheduler();
#endif

  GPluginManager.Initialize();

  GParams.Load();
  GFontSize = GParams.m_FontSize;
  GOrbitApp->LoadFileMapping();

  return true;
}

//-----------------------------------------------------------------------------
void OrbitApp::PostInit() {
  if (!options_.asio_server_address.empty()) {
    GTcpClient = std::make_unique<TcpClient>();
    ConnectionManager::Get().ConnectToRemote(options_.asio_server_address);
    SetIsRemote(true);
  }

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

  ListSessions();

  string_manager_ = std::make_shared<StringManager>();
  GCurrentTimeGraph->SetStringManager(string_manager_);

  for (std::string& arg : m_PostInitArguments) {
    if (absl::StrContains(arg, "systrace:")) {
      std::string command = Replace(arg, "systrace:", "");
      std::vector<std::string> tokens = absl::StrSplit(command, ",");
      if (!tokens.empty()) {
        GoToCapture();
        LoadSystrace(tokens[0]);
      }
      for (size_t i = 1; i + 1 < tokens.size(); i += 2) {
        AppendSystrace(tokens[i], std::stoull(tokens[i + 1]));
      }
      SystraceManager::Get().Dump();
    }
  }
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
void OrbitApp::ListSessions() {
  std::vector<std::string> session_filenames =
      Path::ListFiles(Path::GetPresetPath(), ".opr");
  std::vector<std::shared_ptr<Session>> sessions;
  for (std::string& filename : session_filenames) {
    auto session = std::make_shared<Session>();
    outcome::result<void, std::string> result = 
      ReadSessionFromFile(filename, session.get());
    if (result.has_error()) {
      ERROR("Loading session failed: \"%s\"", result.error());
      continue;
    }
    session->m_FileName = filename;
    sessions.push_back(session);
  }

  m_SessionsDataView->SetSessions(sessions);
}

//-----------------------------------------------------------------------------
void OrbitApp::RefreshCaptureView() {
  NeedsRedraw();
  GOrbitApp->FireRefreshCallbacks();
  DoZoom = true;  // TODO: remove global, review logic
}

//-----------------------------------------------------------------------------
void OrbitApp::AddWatchedVariable(Variable* a_Variable) {
#ifdef _WIN32
  for (WatchCallback& callback : m_AddToWatchCallbacks) {
    callback(a_Variable);
  }
#else
  UNUSED(a_Variable);
#endif
}

//-----------------------------------------------------------------------------
void OrbitApp::UpdateVariable(Variable* a_Variable) {
  for (WatchCallback& callback : m_UpdateWatchCallbacks) {
    callback(a_Variable);
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::ClearWatchedVariables() {
  if (Capture::GTargetProcess) {
    Capture::GTargetProcess->ClearWatchedVariables();
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::RefreshWatch() {
  if (Capture::Connect(options_.asio_server_address)) {
    Capture::GTargetProcess->RefreshWatchedVariables();
  }
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
    SendToUi(disasm.GetResult());
  });
}

//-----------------------------------------------------------------------------
void OrbitApp::OnExit() {
  if (GTimerManager && GTimerManager->m_IsRecording) {
    StopCapture();
  }

  GParams.Save();

  ConnectionManager::Get().Stop();
  GTcpClient->Stop();

  if (HasTcpServer()) {
    GTcpServer->Stop();
  }

  process_manager_->Shutdown();
  thread_pool_->ShutdownAndWait();
  main_thread_executor_->ConsumeActions();

  GTimerManager = nullptr;
  GCoreApp = nullptr;
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

  if (GTcpServer) GTcpServer->ProcessMainThreadCallbacks();
  if (GTcpClient) GTcpClient->ProcessMainThreadCallbacks();

  GOrbitApp->main_thread_executor_->ConsumeActions();

  GMainTimer.Reset();
  GTcpServer->MainThreadTick();

  if (!Capture::GProcessToInject.empty()) {
    LOG("Injecting into %s", Capture::GTargetProcess->GetFullPath());
    LOG("Orbit host: %s", GOrbitApp->options_.asio_server_address);
    GOrbitApp->SelectProcess(Capture::GProcessToInject);
    Capture::InjectRemote(GOrbitApp->options_.asio_server_address);
    exit(0);
  }

#ifdef _WIN32
  GOrbitApp->m_Debugger->MainTick();
#endif

  ++GOrbitApp->m_NumTicks;

  if (DoZoom) {
    GCurrentTimeGraph->SortTracks();
    GOrbitApp->m_CaptureWindow->ZoomAll();
    GOrbitApp->NeedsRedraw();
    DoZoom = false;
  }
}

//-----------------------------------------------------------------------------
std::string OrbitApp::GetVersion() { return OrbitVersion::GetVersion(); }

//-----------------------------------------------------------------------------
void OrbitApp::RegisterCaptureWindow(CaptureWindow* a_Capture) {
  assert(m_CaptureWindow == nullptr);
  m_CaptureWindow = a_Capture;
}

//-----------------------------------------------------------------------------
void OrbitApp::NeedsRedraw() { m_CaptureWindow->NeedsUpdate(); }

//-----------------------------------------------------------------------------
void OrbitApp::AddSamplingReport(
    std::shared_ptr<SamplingProfiler>& sampling_profiler) {
  auto report = std::make_shared<SamplingReport>(sampling_profiler);

  for (SamplingReportCallback& callback : m_SamplingReportsCallbacks) {
    DataView* callstack_data_view =
        GetOrCreateDataView(DataViewType::CALLSTACK);
    callback(callstack_data_view, report);
  }

  sampling_report_ = report;
}

//-----------------------------------------------------------------------------
void OrbitApp::AddSelectionReport(
    std::shared_ptr<SamplingProfiler>& a_SamplingProfiler) {
  auto report = std::make_shared<SamplingReport>(a_SamplingProfiler);

  for (SamplingReportCallback& callback : m_SelectionReportCallbacks) {
    DataView* callstack_data_view =
        GetOrCreateDataView(DataViewType::CALLSTACK);
    callback(callstack_data_view, report);
  }

  selection_report_ = report;
}

//-----------------------------------------------------------------------------
void OrbitApp::GoToCode(DWORD64 a_Address) {
  m_CaptureWindow->FindCode(a_Address);
  SendToUi("gotocode");
}

//-----------------------------------------------------------------------------
void OrbitApp::GoToCallstack() { SendToUi("gotocallstack"); }

//-----------------------------------------------------------------------------
void OrbitApp::GoToCapture() { SendToUi("gotocapture"); }

//-----------------------------------------------------------------------------
void OrbitApp::OnLaunchProcess(const std::string& process_name,
                               const std::string& working_dir,
                               const std::string& args) {
#ifdef _WIN32
  m_Debugger->LaunchProcess(process_name, working_dir, args);
#else
  UNUSED(process_name);
  UNUSED(working_dir);
  UNUSED(args);
#endif
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
std::string OrbitApp::GetSessionFileName() {
  return Capture::GSessionPresets ? Capture::GSessionPresets->m_FileName : "";
}

//-----------------------------------------------------------------------------
std::string OrbitApp::GetSaveFile(const std::string& extension) {
  if (!m_SaveFileCallback) return "";
  return m_SaveFileCallback(extension);
}

//-----------------------------------------------------------------------------
void OrbitApp::SetClipboard(const std::string& text) {
  if (m_ClipboardCallback) m_ClipboardCallback(text);
}

//-----------------------------------------------------------------------------
outcome::result<void, std::string> OrbitApp::OnSaveSession(
    const std::string& filename) {
  OUTCOME_TRY(Capture::SaveSession(filename));
  ListSessions();
  Refresh(DataViewType::SESSIONS);
  return outcome::success();
}

//-----------------------------------------------------------------------------
outcome::result<void, std::string> OrbitApp::ReadSessionFromFile(
    const std::string& filename, Session* session) {
  std::string file_path = filename;

  if (Path::GetDirectory(filename).empty()) {
    file_path = Path::JoinPath({Path::GetPresetPath(), filename});
  }

  std::ifstream file(file_path, std::ios::binary);
  if (file.fail()) {
    ERROR("Loading session from \"%s\": file.fail()", file_path);
    return outcome::failure("Error opening the file for reading");
  }

  try {
    cereal::BinaryInputArchive archive(file);
    archive(*session);
    file.close();
    return outcome::success();
  } catch (std::exception& e) {
    ERROR("Loading session from \"%s\": %s", file_path, e.what());
    return outcome::failure("Error reading the session");
  }
}

//-----------------------------------------------------------------------------
outcome::result<void, std::string> OrbitApp::OnLoadSession(
    const std::string& filename) {
  auto session = std::make_shared<Session>();
  OUTCOME_TRY(ReadSessionFromFile(filename, session.get()));
  session->m_FileName = filename;
  LoadSession(session);
  return outcome::success();
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadSession(const std::shared_ptr<Session>& session) {
  if (Capture::GTargetProcess->GetFullPath() == session->m_ProcessFullPath) {
    // In case we already have the correct process selected
    GOrbitApp->LoadModulesFromSession(Capture::GTargetProcess, session);
    return;
  }
  if (!SelectProcess(Path::GetFileName(session->m_ProcessFullPath))) {
    SendErrorToUi("Session loading failed",
                  absl::StrFormat("The process \"%s\" is not running.",
                                  session->m_ProcessFullPath));
    return;
  }
  Capture::GSessionPresets = session;
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
void OrbitApp::OnDisconnect() { GTcpServer->Send(Msg_Unload); }

//-----------------------------------------------------------------------------
void OrbitApp::FireRefreshCallbacks(DataViewType a_Type) {
  for (DataView* panel : m_Panels) {
    if (a_Type == DataViewType::ALL || a_Type == panel->GetType()) {
      panel->OnDataChanged();
    }
  }

  // UI callbacks
  for (RefreshCallback& callback : m_RefreshCallbacks) {
    callback(a_Type);
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::AddUiMessageCallback(
    std::function<void(const std::string&)> a_Callback) {
  GTcpServer->SetUiCallback(a_Callback);
  m_UiCallback = a_Callback;
}

bool OrbitApp::StartCapture() {
  if (Capture::IsCapturing()) {
    LOG("Ignoring Start Capture - already capturing...");
    return false;
  }

  outcome::result<void, std::string> result =
      Capture::StartCapture(options_.asio_server_address);
  if (result.has_error()) {
    SendErrorToUi("Error starting capture", result.error());
    return false;
  }

  if (m_NeedsThawing) {
#ifdef _WIN32
    m_Debugger->SendThawMessage();
#endif
    m_NeedsThawing = false;
  }

  for (const CaptureStartedCallback& callback : capture_started_callbacks_) {
    callback();
  }

  return true;
}

//-----------------------------------------------------------------------------
void OrbitApp::StopCapture() {
  Capture::StopCapture();

  for (const CaptureStopRequestedCallback& callback :
       capture_stop_requested_callbacks_) {
    callback();
  }
  FireRefreshCallbacks();
}

void OrbitApp::OnCaptureStopped() {
  Capture::FinalizeCapture();

  AddSamplingReport(Capture::GSamplingProfiler);

  for (const CaptureStopRequestedCallback& callback :
       capture_stopped_callbacks_) {
    callback();
  }
  FireRefreshCallbacks();
}

//-----------------------------------------------------------------------------
void OrbitApp::ToggleCapture() {
  if (!GTimerManager) {
    return;
  }

  if (Capture::IsCapturing()) {
    StopCapture();
  } else {
    StartCapture();
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::Unregister(DataView* a_Model) {
  for (size_t i = 0; i < m_Panels.size(); ++i) {
    if (m_Panels[i] == a_Model) {
      m_Panels.erase(m_Panels.begin() + i);
    }
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
bool OrbitApp::Inject(unsigned long a_ProcessId) {
  if (SelectProcess(a_ProcessId)) {
    return Capture::Inject(options_.asio_server_address);
  }

  return false;
}

//-----------------------------------------------------------------------------
void OrbitApp::SetCallStack(std::shared_ptr<CallStack> a_CallStack) {
  m_CallStackDataView->SetCallStack(std::move(a_CallStack));
  FireRefreshCallbacks(DataViewType::CALLSTACK);
}

//-----------------------------------------------------------------------------
void OrbitApp::SendToUi(const std::string& message) {
  main_thread_executor_->Schedule([&, message] {
    if (m_UiCallback) {
      m_UiCallback(message);
    }
  });
}

//-----------------------------------------------------------------------------
void OrbitApp::SendInfoToUi(const std::string& title, const std::string& text) {
  std::string message = "info:" + title + "\n" + text;
  SendToUi(message);
}

//-----------------------------------------------------------------------------
void OrbitApp::SendErrorToUi(const std::string& title,
                             const std::string& text) {
  std::string message = "error:" + title + "\n" + text;
  SendToUi(message);
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadModuleOnRemote(int32_t process_id,
                                  const std::shared_ptr<Module>& module,
                                  const std::shared_ptr<Session>& session) {
  thread_pool_->Schedule([this, process_id, module, session]() {
    const auto remote_symbols_result =
        process_manager_->LoadSymbols(module->m_FullName);

    if (!remote_symbols_result) {
      SendErrorToUi(
          "Error loading symbols",
          absl::StrFormat(
              "Did not find symbols on local machine for module \"%s\". "
              "Trying to load symbols from remote resulted in error "
              "message: %s",
              module->m_Name, remote_symbols_result.error()));
      return;
    }

    main_thread_executor_->Schedule(
        [this, symbols = std::move(remote_symbols_result.value()), module,
         process_id, session]() {
          symbol_helper_.LoadSymbolsIntoModule(module, symbols);
          LOG("Received and loaded %lu function symbols from remote service "
              "for module %s",
              module->m_Pdb->GetFunctions().size(), module->m_Name.c_str());
          SymbolLoadingFinished(process_id, module, session);
        });
  });
}

void OrbitApp::SymbolLoadingFinished(uint32_t process_id,
                                     const std::shared_ptr<Module>& module,
                                     const std::shared_ptr<Session>& session) {
  if (session != nullptr &&
      session->m_Modules.find(module->m_FullName) != session->m_Modules.end()) {
    module->m_Pdb->ApplyPresets(*session);
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
                           const std::shared_ptr<Session>& session) {
  // TODO(159868905) use ModuleData instead of Module
  for (const auto& module : modules) {
    if (modules_currently_loading_.contains(module->m_FullName)) {
      continue;
    }
    modules_currently_loading_.insert(module->m_FullName);

    // TODO (159889010) Move symbol loading off the main thread.
    if (symbol_helper_.LoadSymbolsUsingSymbolsFile(module)) {
      LOG("Loaded %lu function symbols locally for modules %s",
          module->m_Pdb->GetFunctions().size(), module->m_Name);
      SymbolLoadingFinished(process_id, module, session);
    } else {
      LOG("Did not find local symbols for module: %s", module->m_Name);
      LoadModuleOnRemote(process_id, module, session);
    }
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadModulesFromSession(const std::shared_ptr<Process>& process,
                                      const std::shared_ptr<Session>& session) {
  std::vector<std::shared_ptr<Module>> modules;
  for (const auto& pair : session->m_Modules) {
    const std::string& module_path = pair.first;
    const auto& module = process->GetModuleFromPath(module_path);
    if (module != nullptr && !module->IsLoaded()) {
      modules.emplace_back(std::move(module));
    }
  }
  LoadModules(process->GetID(), modules, session);
}

//-----------------------------------------------------------------------------
bool OrbitApp::IsLoading() { return GPdbDbg->IsLoading(); }

//-----------------------------------------------------------------------------
void OrbitApp::SetTrackContextSwitches(bool a_Value) {
  GParams.m_TrackContextSwitches = a_Value;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetTrackContextSwitches() {
  return GParams.m_TrackContextSwitches;
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableUnrealSupport(bool a_Value) {
  GParams.m_UnrealSupport = a_Value;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetUnrealSupportEnabled() { return GParams.m_UnrealSupport; }

//-----------------------------------------------------------------------------
void OrbitApp::EnableUnsafeHooking(bool a_Value) {
  GParams.m_AllowUnsafeHooking = a_Value;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetUnsafeHookingEnabled() {
  return GParams.m_AllowUnsafeHooking;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetOutputDebugStringEnabled() {
  return GParams.m_HookOutputDebugString;
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableOutputDebugString(bool a_Value) {
  GParams.m_HookOutputDebugString = a_Value;
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableSampling(bool a_Value) {
  GParams.m_TrackSamplingEvents = a_Value;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetSamplingEnabled() { return GParams.m_TrackSamplingEvents; }

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

        std::shared_ptr<Session> session = Capture::GSessionPresets;
        if (session) {
          LoadModulesFromSession(process, session);
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
bool OrbitApp::GetUploadDumpsToServerEnabled() const {
  return GParams.m_UploadDumpsToServer;
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableUploadDumpsToServer(bool a_Value) {
  GParams.m_UploadDumpsToServer = a_Value;
  GParams.Save();
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

    case DataViewType::TYPES:
      if (!m_TypesDataView) {
        m_TypesDataView = std::make_unique<TypesDataView>();
        m_Panels.push_back(m_TypesDataView.get());
      }
      return m_TypesDataView.get();

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

    case DataViewType::GLOBALS:
      if (!m_GlobalsDataView) {
        m_GlobalsDataView = std::make_unique<GlobalsDataView>();
        m_Panels.push_back(m_GlobalsDataView.get());
      }
      return m_GlobalsDataView.get();

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

    case DataViewType::SESSIONS:
      if (!m_SessionsDataView) {
        m_SessionsDataView = std::make_unique<SessionsDataView>();
        m_Panels.push_back(m_SessionsDataView.get());
      }
      return m_SessionsDataView.get();

    case DataViewType::LOG:
      if (!m_LogDataView) {
        m_LogDataView = std::make_unique<LogDataView>();
        m_Panels.push_back(m_LogDataView.get());
      }
      return m_LogDataView.get();

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
void OrbitApp::CrashOrbitService(GetCrashRequest_CrashType crash_type) {
  if (absl::GetFlag(FLAGS_devmode)) {
    thread_pool_->Schedule(
        [this, crash_type] { crash_manager_->CrashOrbitService(crash_type); });
  }
}
