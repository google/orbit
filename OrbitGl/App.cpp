//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

// clang-format off
#include "OrbitAsio.h"
// clang-format on

#include "App.h"

#include <chrono>
#include <cmath>
#include <fstream>
#include <thread>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Tracing.h"

#include "CallStackDataView.h"
#include "Callstack.h"
#include "Capture.h"
#include "CaptureSerializer.h"
#include "CaptureWindow.h"
#include "ConnectionManager.h"
#include "Debugger.h"
#include "DiaManager.h"
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
#include "ModuleManager.h"
#include "OrbitAsm.h"
#include "OrbitSession.h"
#include "Params.h"
#include "Pdb.h"
#include "PluginManager.h"
#include "PrintVar.h"
#include "ProcessesDataView.h"
#include "RuleEditor.h"
#include "SamplingProfiler.h"
#include "SamplingReport.h"
#include "ScopeTimer.h"
#include "Serialization.h"
#include "SessionsDataView.h"
#include "StringManager.h"
#include "SymbolsManager.h"
#include "Systrace.h"
#include "Tcp.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "TestRemoteMessages.h"
#include "TextRenderer.h"
#include "TimerManager.h"
#include "TransactionManager.h"
#include "TypesDataView.h"
#include "Utils.h"
#include "Version.h"

#define GLUT_DISABLE_ATEXIT_HACK
#include "GL/freeglut.h"

#if __linux__
#include <OrbitLinuxTracing/OrbitTracing.h>
#endif

std::unique_ptr<OrbitApp> GOrbitApp;
float GFontSize;
bool DoZoom = false;

//-----------------------------------------------------------------------------
OrbitApp::OrbitApp(ApplicationOptions&& options)
    : options_(std::move(options)) {
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
std::wstring OrbitApp::FindFile(const std::wstring& a_Caption,
                                const std::wstring& a_Dir,
                                const std::wstring& a_Filter) {
  if (m_FindFileCallback) {
    return m_FindFileCallback(a_Caption, a_Dir, a_Filter);
  }

  return std::wstring();
}

//-----------------------------------------------------------------------------
void OrbitApp::SetCommandLineArguments(const std::vector<std::string>& a_Args) {
  m_Arguments = a_Args;

  for (const std::string& arg : a_Args) {
    if (absl::StrContains(arg, "preset:")) {
      std::vector<std::string> vec = Tokenize(arg, ":");
      if (vec.size() > 1) {
        Capture::GPresetToLoad = vec[1];
      }
    } else if (absl::StrContains(arg, "inject:")) {
      std::vector<std::string> vec = Tokenize(arg, ":");
      if (vec.size() > 1) {
        Capture::GProcessToInject = vec[1];
      }
    } else if (absl::StrContains(arg, "systrace:")) {
      m_PostInitArguments.push_back(arg);
    }
  }
}

// Get the horizontal and vertical screen sizes in pixel
//-----------------------------------------------------------------------------
void GetDesktopResolution(int& horizontal, int& vertical) {
#ifdef _WIN32
  RECT desktop;
  // Get a handle to the desktop window
  const HWND hDesktop = GetDesktopWindow();
  // Get the size of screen to the variable desktop
  GetWindowRect(hDesktop, &desktop);
  // The top left corner will have coordinates (0,0)
  // and the bottom right corner will have coordinates
  // (horizontal, vertical)
  horizontal = desktop.right;
  vertical = desktop.bottom;
#else
  UNUSED(horizontal);
  UNUSED(vertical);
#endif
}

//-----------------------------------------------------------------------------
void GLoadPdbAsync(const std::vector<std::string>& a_Modules) {
  GModuleManager.LoadPdbAsync(a_Modules, []() { GOrbitApp->OnPdbLoaded(); });
}

//-----------------------------------------------------------------------------
void OrbitApp::ProcessTimer(const Timer& a_Timer, const std::string&) {
  CHECK(!ConnectionManager::Get().IsService());
  GCurrentTimeGraph->ProcessTimer(a_Timer);
  ++Capture::GFunctionCountMap[a_Timer.m_FunctionAddress];
}

//-----------------------------------------------------------------------------
void OrbitApp::ProcessSamplingCallStack(LinuxCallstackEvent& a_CallStack) {
  CHECK(!ConnectionManager::Get().IsService());

  Capture::GSamplingProfiler->AddCallStack(a_CallStack.m_CS);
  GEventTracer.GetEventBuffer().AddCallstackEvent(
      a_CallStack.m_time, a_CallStack.m_CS.m_Hash, a_CallStack.m_CS.m_ThreadId);
}

//-----------------------------------------------------------------------------
void OrbitApp::ProcessHashedSamplingCallStack(CallstackEvent& a_CallStack) {
  if (ConnectionManager::Get().IsService()) {
    ScopeLock lock(m_HashedSamplingCallstackMutex);
    m_HashedSamplingCallstackBuffer.push_back(a_CallStack);
  } else {
    Capture::GSamplingProfiler->AddHashedCallStack(a_CallStack);
    GEventTracer.GetEventBuffer().AddCallstackEvent(
        a_CallStack.m_Time, a_CallStack.m_Id, a_CallStack.m_TID);
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::ProcessCallStack(CallStack& a_CallStack) {
  if (ConnectionManager::Get().IsService()) {
    // Send full callstack once
    if (!Capture::GetCallstack(a_CallStack.Hash())) {
      std::string messageData = SerializeObjectHumanReadable(a_CallStack);
      GTcpServer->Send(Msg_RemoteCallStack, (void*)messageData.c_str(),
                       messageData.size());
    }
  }

  Capture::AddCallstack(a_CallStack);
}

//-----------------------------------------------------------------------------
void OrbitApp::ProcessContextSwitch(const ContextSwitch& a_ContextSwitch) {
  if (ConnectionManager::Get().IsService()) {
    ScopeLock lock(m_ContextSwitchMutex);
    m_ContextSwitchBuffer.push_back(a_ContextSwitch);
  }

  GTimerManager->Add(a_ContextSwitch);
}

//-----------------------------------------------------------------------------
void OrbitApp::AddAddressInfo(LinuxAddressInfo address_info) {
  // This should not be called for the service - make sure it doesn't
  CHECK(!ConnectionManager::Get().IsService());

  Capture::GTargetProcess->AddAddressInfo(std::move(address_info));
}
//-----------------------------------------------------------------------------
void OrbitApp::AddKeyAndString(uint64_t key, std::string_view str) {
  CHECK(!ConnectionManager::Get().IsService());
  {
    KeyAndString key_and_string;
    key_and_string.key = key;
    key_and_string.str = str;
    if (!string_manager_->Exists(key)) {
      std::string message_data = SerializeObjectBinary(key_and_string);
      GTcpServer->Send(Msg_KeyAndString, (void*)message_data.c_str(),
                       message_data.size());
      string_manager_->Add(key, str);
    }
  }

  string_manager_->Add(key, str);
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

  GModuleManager.Init();
  Capture::Init();

  // TODO(antonrohr) clean this up (it casts GOrbitApp* to void*)
  Capture::SetSamplingDoneCallback(&OrbitApp::AddSamplingReport,
                                   GOrbitApp.get());
  Capture::SetLoadPdbAsyncFunc(GLoadPdbAsync);

#ifdef _WIN32
  DiaManager::InitMsDiaDll();
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
    GTcpClient->AddMainThreadCallback(
        Msg_RemoteProcess,
        [=](const Message& a_Msg) { GOrbitApp->OnRemoteProcess(a_Msg); });
    GTcpClient->AddMainThreadCallback(
        Msg_RemoteProcessList,
        [=](const Message& a_Msg) { GOrbitApp->OnRemoteProcessList(a_Msg); });
    ConnectionManager::Get().ConnectToRemote(options_.asio_server_address);
    m_ProcessesDataView->SetIsRemote(true);
    SetIsRemote(true);
  }

  string_manager_ = std::make_shared<StringManager>();
  GCurrentTimeGraph->SetStringManager(string_manager_);

  for (std::string& arg : m_PostInitArguments) {
    if (absl::StrContains(arg, "systrace:")) {
      std::string command = Replace(arg, "systrace:", "");
      auto tokens = Tokenize(command, ",");
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

  int my_argc = 0;
  glutInit(&my_argc, NULL);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  GetDesktopResolution(GOrbitApp->m_ScreenRes[0], GOrbitApp->m_ScreenRes[1]);

  GOrbitApp->InitializeManagers();
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

  std::wfstream infile(fileName);
  if (!infile.fail()) {
    std::wstring wline;
    while (std::getline(infile, wline)) {
      std::string line = ws2s(wline);
      if (absl::StartsWith(line, "//")) continue;

      bool containsQuotes = absl::StrContains(line, "\"");

      std::vector<std::string> tokens = Tokenize(line);
      if (tokens.size() == 2 && !containsQuotes) {
        m_FileMapping[ToLower(tokens[0])] = ToLower(tokens[1]);
      } else {
        std::vector<std::string> validTokens;
        for (const std::string& token : Tokenize(line, "\"//")) {
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
  std::vector<std::string> sessionFileNames =
      Path::ListFiles(Path::GetPresetPath(), ".opr");
  std::vector<std::shared_ptr<Session>> sessions;
  for (std::string& fileName : sessionFileNames) {
    std::ifstream file(fileName, std::ios::binary);
    if (!file.fail()) {
      try {
        auto session = std::make_shared<Session>();
        cereal::BinaryInputArchive archive(file);
        archive(*session);
        file.close();
        session->m_FileName = fileName;
        sessions.push_back(session);
      } catch (std::exception& e) {
        ERROR("Loading session from \"%s\": %s", fileName.c_str(), e.what());
      }
    }
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
  // Make sure type hierarchy has been generated
  if (Type* type =
          a_Variable->m_Pdb->GetTypePtrFromId(a_Variable->m_TypeIndex)) {
    type->LoadDiaInfo();
  }

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
void OrbitApp::Disassemble(const std::string& a_FunctionName,
                           uint64_t a_VirtualAddress,
                           const uint8_t* a_MachineCode, size_t a_Size) {
  Disassembler disasm;
  disasm.LOGF(absl::StrFormat("asm: /* %s */\n", a_FunctionName.c_str()));
  const unsigned char* code = (const unsigned char*)a_MachineCode;
  disasm.Disassemble(code, a_Size, a_VirtualAddress,
                     Capture::GTargetProcess->GetIs64Bit());
  SendToUiAsync(disasm.GetResult());
}

//-----------------------------------------------------------------------------
const std::unordered_map<DWORD64, std::shared_ptr<class Rule>>*
OrbitApp::GetRules() {
  return &m_RuleEditor->GetRules();
}

//-----------------------------------------------------------------------------
void OrbitApp::SetLicense(const std::wstring& a_License) {
  m_License = a_License;
}

//-----------------------------------------------------------------------------
int OrbitApp::OnExit() {
  if (GTimerManager && GTimerManager->m_IsRecording) GOrbitApp->StopCapture();

  GParams.Save();
  GTimerManager = nullptr;

  ConnectionManager::Get().Stop();
  GTcpClient->Stop();

  if (GOrbitApp->HasTcpServer()) {
    GTcpServer->Stop();
  }

  GCoreApp = nullptr;
  GOrbitApp = nullptr;
  Orbit_ImGui_Shutdown();
  return 0;
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

  // Tick Transaction manager only from client (OrbitApp is client only);
  auto transaction_manager = GOrbitApp->GetTransactionManager();

  // Note that MainTick could be called before OrbitApp::PostInit() was complete
  // in which case translaction namager is not yet initialized - check that it
  // is not null before calling it.
  if (transaction_manager != nullptr) {
    transaction_manager->Tick();
  }

  GMainTimer.Reset();
  Capture::Update();
  GTcpServer->MainThreadTick();

  if (!Capture::GProcessToInject.empty()) {
    std::cout << "Injecting into " << Capture::GTargetProcess->GetFullPath()
              << std::endl;
    std::cout << "Orbit host: " << GOrbitApp->options_.asio_server_address
              << std::endl;
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
void OrbitApp::RegisterProcessesDataView(ProcessesDataView* a_Processes) {
  assert(m_ProcessesDataView == nullptr);
  m_ProcessesDataView = a_Processes;
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterModulesDataView(ModulesDataView* a_Modules) {
  assert(m_ModulesDataView == nullptr);
  assert(m_ProcessesDataView != nullptr);
  m_ModulesDataView = a_Modules;
  m_ProcessesDataView->SetModulesDataView(m_ModulesDataView);
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterFunctionsDataView(FunctionsDataView* a_Functions) {
  m_FunctionsDataView = a_Functions;
  m_Panels.push_back(a_Functions);
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterLiveFunctionsDataView(
    LiveFunctionsDataView* a_Functions) {
  m_LiveFunctionsDataView = a_Functions;
  m_Panels.push_back(a_Functions);
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterCallStackDataView(CallStackDataView* a_Callstack) {
  assert(m_CallStackDataView == nullptr);
  m_CallStackDataView = a_Callstack;
  m_Panels.push_back(a_Callstack);
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterTypesDataView(TypesDataView* a_Types) {
  m_TypesDataView = a_Types;
  m_Panels.push_back(a_Types);
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterGlobalsDataView(GlobalsDataView* a_Globals) {
  m_GlobalsDataView = a_Globals;
  m_Panels.push_back(a_Globals);
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterSessionsDataView(SessionsDataView* a_Sessions) {
  m_SessionsDataView = a_Sessions;
  m_Panels.push_back(a_Sessions);
  ListSessions();
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterCaptureWindow(CaptureWindow* a_Capture) {
  assert(m_CaptureWindow == nullptr);
  m_CaptureWindow = a_Capture;
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterOutputLog(LogDataView* a_Log) {
  assert(m_Log == nullptr);
  m_Log = a_Log;
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterRuleEditor(RuleEditor* a_RuleEditor) {
  assert(m_RuleEditor == nullptr);
  m_RuleEditor = a_RuleEditor;
}

//-----------------------------------------------------------------------------
void OrbitApp::NeedsRedraw() { m_CaptureWindow->NeedsUpdate(); }

//-----------------------------------------------------------------------------
void OrbitApp::AddSamplingReport(
    std::shared_ptr<SamplingProfiler>& sampling_profiler, void* app_ptr) {
  OrbitApp* app = static_cast<OrbitApp*>(app_ptr);
  auto report = std::make_shared<SamplingReport>(sampling_profiler);

  for (SamplingReportCallback& callback : app->m_SamplingReportsCallbacks) {
    callback(report);
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::AddSelectionReport(
    std::shared_ptr<SamplingProfiler>& a_SamplingProfiler) {
  auto report = std::make_shared<SamplingReport>(a_SamplingProfiler);

  for (SamplingReportCallback& callback :
       GOrbitApp->m_SelectionReportCallbacks) {
    callback(report);
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::GoToCode(DWORD64 a_Address) {
  m_CaptureWindow->FindCode(a_Address);
  SendToUiNow("gotocode");
}

//-----------------------------------------------------------------------------
void OrbitApp::GoToCallstack() { SendToUiNow("gotocallstack"); }

//-----------------------------------------------------------------------------
void OrbitApp::GoToCapture() { SendToUiNow("gotocapture"); }

//-----------------------------------------------------------------------------
void OrbitApp::OnOpenPdb(const std::string& file_name) {
  Capture::GTargetProcess = std::make_shared<Process>();
  std::shared_ptr<Module> mod = std::make_shared<Module>();

  mod->m_FullName = file_name;
  mod->m_Name = Path::GetFileName(file_name);
  mod->m_Directory = Path::GetDirectory(file_name);
  mod->m_PdbName = mod->m_FullName;
  mod->m_FoundPdb = true;
  mod->LoadDebugInfo();

  Capture::GTargetProcess->SetName(Path::StripExtension(mod->m_Name));
  Capture::GTargetProcess->AddModule(mod);

  m_ModulesDataView->SetProcess(Capture::GTargetProcess);
  Capture::SetTargetProcess(Capture::GTargetProcess);
  GOrbitApp->FireRefreshCallbacks();

  EnqueueModuleToLoad(mod);
  LoadModules();
}

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
std::wstring OrbitApp::GetCaptureFileName() {
  assert(Capture::GTargetProcess);
  time_t timestamp =
      std::chrono::system_clock::to_time_t(Capture::GCaptureTimePoint);
  std::string timestamp_string = OrbitUtils::FormatTime(timestamp);
  std::string result =
      Path::StripExtension(Capture::GTargetProcess->GetName()) + "_" +
      timestamp_string + ".orbit";

  return s2ws(result);
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
void OrbitApp::SetClipboard(const std::wstring& a_Text) {
  if (m_ClipboardCallback) m_ClipboardCallback(a_Text);
}

//-----------------------------------------------------------------------------
void OrbitApp::OnSaveSession(const std::string& file_name) {
  Capture::SaveSession(file_name);
  ListSessions();
  Refresh(DataViewType::SESSIONS);
}

//-----------------------------------------------------------------------------
bool OrbitApp::OnLoadSession(const std::string& file_name) {
  std::string file_path = file_name;

  if (Path::GetDirectory(file_name).empty()) {
    file_path = Path::GetPresetPath() + file_name;
  }

  std::ifstream file(file_path);
  if (!file.fail()) {
    try {
      auto session = std::make_shared<Session>();
      cereal::BinaryInputArchive archive(file);
      archive(*session);
      file.close();
      session->m_FileName = file_path;
      LoadSession(session);
      return true;
    } catch (std::exception& e) {
      ERROR("Loading session from \"%s\": %s", file_path.c_str(), e.what());
    }
  }

  return false;
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadSession(const std::shared_ptr<Session>& session) {
  if (SelectProcess(Path::GetFileName(session->m_ProcessFullPath))) {
    Capture::GSessionPresets = session;
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::OnSaveCapture(const std::string& file_name) {
  CaptureSerializer ar;
  ar.time_graph_ = GCurrentTimeGraph;
  ar.Save(s2ws(file_name));
}

//-----------------------------------------------------------------------------
void OrbitApp::OnLoadCapture(const std::string& file_name) {
  StopCapture();
  Capture::ClearCaptureData();
  GCurrentTimeGraph->Clear();
  if (Capture::GClearCaptureDataFunc) {
    Capture::GClearCaptureDataFunc();
  }

  CaptureSerializer ar;
  ar.time_graph_ = GCurrentTimeGraph;
  ar.Load(s2ws(file_name));
  m_ModulesDataView->SetProcess(Capture::GTargetProcess);
  StopCapture();
  DoZoom = true;  // TODO: remove global, review logic
}

//-----------------------------------------------------------------------------
void GLoadPdbAsync(const std::shared_ptr<Module>& a_Module) {
  GModuleManager.LoadPdbAsync(a_Module, []() { GOrbitApp->OnPdbLoaded(); });
}

//-----------------------------------------------------------------------------
void OrbitApp::OnDisconnect() { GTcpServer->Send(Msg_Unload); }

//-----------------------------------------------------------------------------
void OrbitApp::OnPdbLoaded() {
  FireRefreshCallbacks();

  if (m_ModulesToLoad.empty()) {
    SendToUiAsync("pdbloaded");
  } else {
    LoadModules();
  }
}

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

//-----------------------------------------------------------------------------
void OrbitApp::StartCapture() {
  // Tracing session is only needed when StartCapture is
  // running on the service side
  Capture::StartCapture(nullptr /* tracing_session */,
                        options_.asio_server_address);

  if (m_NeedsThawing) {
#ifdef _WIN32
    m_Debugger->SendThawMessage();
#endif
    m_NeedsThawing = false;
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::StopCapture() {
  Capture::StopCapture();

  FireRefreshCallbacks();
}

//-----------------------------------------------------------------------------
void OrbitApp::ToggleCapture() {
  if (GTimerManager) {
    if (GTimerManager->m_IsRecording)
      StopCapture();
    else
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
bool OrbitApp::SelectProcess(uint32_t a_ProcessID) {
  if (m_ProcessesDataView) {
    return m_ProcessesDataView->SelectProcess(a_ProcessID) != nullptr;
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
void OrbitApp::SendToUiAsync(const std::string& message) {
  GTcpServer->SendToUiAsync(message);
}

//-----------------------------------------------------------------------------
void OrbitApp::SendToUiNow(const std::string& message) {
  if (m_UiCallback) {
    m_UiCallback(message);
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::EnqueueModuleToLoad(const std::shared_ptr<Module>& a_Module) {
  m_ModulesToLoad.push_back(a_Module);
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadModules() {
  if (!m_ModulesToLoad.empty()) {
    if (Capture::IsRemote()) {
      LoadRemoteModules();
      return;
    }
#ifdef _WIN32
    for (std::shared_ptr<Module> module : m_ModulesToLoad) {
      GLoadPdbAsync(module);
    }
#else
    for (std::shared_ptr<Module> module : m_ModulesToLoad) {
      if (symbol_helper_.LoadSymbolsIncludedInBinary(module)) continue;
      if (symbol_helper_.LoadSymbolsUsingSymbolsFile(module)) continue;
      ERROR("Could not load symbols for module %s", module->m_Name.c_str());
    }
    GOrbitApp->FireRefreshCallbacks();
#endif
  }

  m_ModulesToLoad.clear();
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadRemoteModules() {
  GetSymbolsManager()->LoadSymbols(m_ModulesToLoad, Capture::GTargetProcess);
  m_ModulesToLoad.clear();
  GOrbitApp->FireRefreshCallbacks();
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
void OrbitApp::OnRemoteProcess(const Message& a_Message) {
  std::istringstream buffer(std::string(a_Message.m_Data, a_Message.m_Size));
  cereal::JSONInputArchive inputAr(buffer);
  std::shared_ptr<Process> remoteProcess = std::make_shared<Process>();
  inputAr(*remoteProcess);
  remoteProcess->SetIsRemote(true);
  PRINT_VAR(remoteProcess->GetName());
  GOrbitApp->m_ProcessesDataView->SetRemoteProcess(remoteProcess);

  // Trigger session loading if needed.
  std::shared_ptr<Session> session = Capture::GSessionPresets;
  if (session) {
    GetSymbolsManager()->LoadSymbols(session, remoteProcess);
    GParams.m_ProcessPath = session->m_ProcessFullPath;
    GParams.m_Arguments = session->m_Arguments;
    GParams.m_WorkingDirectory = session->m_WorkingDirectory;
    GCoreApp->SendToUiNow("SetProcessParams");
    Capture::GSessionPresets = nullptr;
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::ApplySession(const Session& session) {
  for (const auto& pair : session.m_Modules) {
    const std::string& name = pair.first;
    std::shared_ptr<Module> module =
        Capture::GTargetProcess->GetModuleFromName(Path::GetFileName(name));
    if (module && module->m_Pdb) module->m_Pdb->ApplyPresets(session);
  }

  FireRefreshCallbacks();
}

//-----------------------------------------------------------------------------
void OrbitApp::OnRemoteProcessList(const Message& a_Message) {
  std::istringstream buffer(std::string(a_Message.m_Data, a_Message.m_Size));
  cereal::JSONInputArchive inputAr(buffer);
  ProcessList remoteProcessList;
  inputAr(remoteProcessList);
  remoteProcessList.SetRemote(true);
  GOrbitApp->m_ProcessesDataView->SetRemoteProcessList(
      std::move(remoteProcessList));

  // Trigger session loading if needed.
  if (!Capture::GPresetToLoad.empty()) {
    GOrbitApp->OnLoadSession(Capture::GPresetToLoad);
    Capture::GPresetToLoad = "";
  }
}

//-----------------------------------------------------------------------------
void OrbitApp::OnRemoteModuleDebugInfo(const Message& a_Message) {
  std::vector<ModuleDebugInfo> remote_module_debug_infos;
  DeserializeObjectBinary(a_Message.GetData(), a_Message.GetSize(),
                          remote_module_debug_infos);
  OnRemoteModuleDebugInfo(remote_module_debug_infos);
}

//-----------------------------------------------------------------------------
void OrbitApp::OnRemoteModuleDebugInfo(
    const std::vector<ModuleDebugInfo>& remote_module_debug_infos) {
  for (const ModuleDebugInfo& module_info : remote_module_debug_infos) {
    std::shared_ptr<Module> module =
        Capture::GTargetProcess->GetModuleFromName(module_info.m_Name);

    if (!module) {
      ERROR("Could not find module %s", module_info.m_Name.c_str());
      continue;
    }

    if (module_info.m_Functions.empty()) {
      ERROR("Remote did not send any symbols for module %s",
            module_info.m_Name.c_str());
    } else {
      symbol_helper_.LoadSymbolsFromDebugInfo(module, module_info);
      LOG("Received %lu function symbols from remote collector for module %s",
          module_info.m_Functions.size(), module_info.m_Name.c_str());
    }
  }

  GOrbitApp->FireRefreshCallbacks();
}

//-----------------------------------------------------------------------------
void OrbitApp::LaunchRuleEditor(Function* a_Function) {
  m_RuleEditor->m_Window.Launch(a_Function);
  SendToUiNow("RuleEditor");
}
