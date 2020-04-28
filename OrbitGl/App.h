//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <utility>

#include "ApplicationOptions.h"
#include "ContextSwitch.h"
#include "CoreApp.h"
#include "DataViewTypes.h"
#include "Message.h"
#include "StringManager.h"
#include "SymbolHelper.h"
#include "Threading.h"

#if defined(_WIN32)
#include "Debugger.h"
#endif

struct CallStack;
class Process;

//-----------------------------------------------------------------------------
class OrbitApp : public CoreApp {
 public:
  explicit OrbitApp(ApplicationOptions&& options);
  ~OrbitApp() override;

  static bool Init(ApplicationOptions&& options);
  void PostInit();
  static int OnExit();
  static void MainTick();
  void CheckLicense();
  void SetLicense(const std::wstring& a_License);
  std::string GetVersion();
  void CheckDebugger();

  std::wstring GetCaptureFileName();
  std::string GetSessionFileName();
  std::string GetSaveFile(const std::string& extension);
  void SetClipboard(const std::wstring& a_Text);
  void OnSaveSession(const std::string& file_name);
  bool OnLoadSession(const std::string& file_name);
  void OnSaveCapture(const std::string& file_name);
  void OnLoadCapture(const std::string& file_name);
  void OnOpenPdb(const std::string& file_name);
  void OnLaunchProcess(const std::string& process_name,
                       const std::string& working_dir, const std::string& args);
  void Inject(const std::string& file_name);
  void StartCapture();
  void StopCapture();
  void ToggleCapture();
  void OnDisconnect();
  void OnPdbLoaded();
  void SetCallStack(std::shared_ptr<CallStack> a_CallStack);
  void LoadFileMapping();
  void LoadSystrace(const std::string& a_FileName);
  void AppendSystrace(const std::string& a_FileName, uint64_t a_TimeOffset);
  void ListSessions();
  void RefreshCaptureView() override;
  void RequestRemoteModules(const std::vector<std::string>& a_Modules);
  void AddWatchedVariable(Variable* a_Variable);
  void UpdateVariable(Variable* a_Variable) override;
  void ClearWatchedVariables();
  void RefreshWatch();
  void Disassemble(const std::string& a_FunctionName, uint64_t a_VirtualAddress,
                   const uint8_t* a_MachineCode, size_t a_Size) override;
  void ProcessTimer(const Timer& a_Timer,
                    const std::string& a_FunctionName) override;
  void ProcessSamplingCallStack(LinuxCallstackEvent& a_CallStack) override;
  void ProcessHashedSamplingCallStack(CallstackEvent& a_CallStack) override;
  void ProcessCallStack(CallStack& a_CallStack) override;
  void ProcessContextSwitch(const ContextSwitch& a_ContextSwitch) override;
  void AddAddressInfo(LinuxAddressInfo address_info) override;
  void AddKeyAndString(uint64_t key, std::string_view str) override;

  int* GetScreenRes() { return m_ScreenRes; }

  void RegisterProcessesDataView(class ProcessesDataView* a_Processes);
  void RegisterModulesDataView(class ModulesDataView* a_Modules);
  void RegisterFunctionsDataView(class FunctionsDataView* a_Functions);
  void RegisterLiveFunctionsDataView(class LiveFunctionsDataView* a_Functions);
  void RegisterCallStackDataView(class CallStackDataView* a_Callstack);
  void RegisterTypesDataView(class TypesDataView* a_Types);
  void RegisterGlobalsDataView(class GlobalsDataView* a_Globals);
  void RegisterSessionsDataView(class SessionsDataView* a_Sessions);
  void RegisterCaptureWindow(class CaptureWindow* a_Capture);
  void RegisterOutputLog(class LogDataView* a_Log);
  void RegisterRuleEditor(class RuleEditor* a_RuleEditor);

  void Unregister(class DataView* a_Model);
  bool SelectProcess(const std::string& a_Process);
  bool SelectProcess(uint32_t a_ProcessID);
  bool Inject(unsigned long a_ProcessId);
  static void AddSamplingReport(
      std::shared_ptr<class SamplingProfiler>& sampling_profiler,
      void* app_ptr);

  static void AddSelectionReport(
      std::shared_ptr<SamplingProfiler>& a_SamplingProfiler);

  void GoToCode(DWORD64 a_Address);
  void GoToCallstack();
  void GoToCapture();

  // Callbacks
  typedef std::function<void(DataViewType a_Type)> RefreshCallback;
  void AddRefreshCallback(RefreshCallback a_Callback) {
    m_RefreshCallbacks.emplace_back(std::move(a_Callback));
  }
  typedef std::function<void(std::shared_ptr<class SamplingReport>)>
      SamplingReportCallback;
  void AddSamplingReoprtCallback(SamplingReportCallback a_Callback) {
    m_SamplingReportsCallbacks.emplace_back(std::move(a_Callback));
  }
  void AddSelectionReportCallback(SamplingReportCallback a_Callback) {
    m_SelectionReportCallbacks.emplace_back(std::move(a_Callback));
  }
  typedef std::function<void(Variable* a_Variable)> WatchCallback;
  void AddWatchCallback(WatchCallback a_Callback) {
    m_AddToWatchCallbacks.emplace_back(std::move(a_Callback));
  }
  typedef std::function<std::string(const std::string& a_Extension)>
      SaveFileCallback;
  void SetSaveFileCallback(SaveFileCallback a_Callback) {
    m_SaveFileCallback = std::move(a_Callback);
  }
  void AddUpdateWatchCallback(WatchCallback a_Callback) {
    m_UpdateWatchCallbacks.emplace_back(std::move(a_Callback));
  }
  void FireRefreshCallbacks(DataViewType a_Type = DataViewType::ALL);
  void Refresh(DataViewType a_Type = DataViewType::ALL) {
    FireRefreshCallbacks(a_Type);
  }
  void AddUiMessageCallback(std::function<void(const std::string&)> a_Callback);
  typedef std::function<std::wstring(const std::wstring& a_Caption,
                                     const std::wstring& a_Dir,
                                     const std::wstring& a_Filter)>
      FindFileCallback;
  void SetFindFileCallback(FindFileCallback a_Callback) {
    m_FindFileCallback = std::move(a_Callback);
  }
  std::wstring FindFile(const std::wstring& a_Caption,
                        const std::wstring& a_Dir,
                        const std::wstring& a_Filter);
  typedef std::function<void(const std::wstring&)> ClipboardCallback;
  void SetClipboardCallback(ClipboardCallback a_Callback) {
    m_ClipboardCallback = std::move(a_Callback);
  }

  void SetCommandLineArguments(const std::vector<std::string>& a_Args);

  // TODO(antonrohr) check whether this is still used
  const std::vector<std::string>& GetCommandLineArguments() {
    return m_Arguments;
  }

  void SendToUiAsync(const std::string& message) override;
  void SendToUiNow(const std::string& message) override;
  void NeedsRedraw();

  const std::map<std::string, std::string>& GetFileMapping() {
    return m_FileMapping;
  }

  void EnqueueModuleToLoad(const std::shared_ptr<struct Module>& a_Module);
  void LoadModules();
  void LoadRemoteModules();
  bool IsLoading();
  void SetTrackContextSwitches(bool a_Value);
  bool GetTrackContextSwitches();

  void EnableUnrealSupport(bool a_Value);
  bool GetUnrealSupportEnabled() override;

  void EnableSampling(bool a_Value);
  bool GetSamplingEnabled() override;

  void EnableUnsafeHooking(bool a_Value);
  bool GetUnsafeHookingEnabled() override;

  void EnableOutputDebugString(bool a_Value);
  bool GetOutputDebugStringEnabled() override;

  void RequestThaw() { m_NeedsThawing = true; }
  void OnRemoteProcess(const Message& a_Message);
  void OnRemoteProcessList(const Message& a_Message);
  void OnRemoteModuleDebugInfo(const Message& a_Message);
  void OnRemoteModuleDebugInfo(const std::vector<ModuleDebugInfo>&) override;
  void ApplySession(const Session& session) override;
  void LoadSession(const std::shared_ptr<Session>& session);
  void LaunchRuleEditor(class Function* a_Function);
  void SetIsRemote(bool a_IsRemote) { m_IsRemote = a_IsRemote; }
  bool IsRemote() const { return m_IsRemote; }
  bool HasTcpServer() const { return !IsRemote(); }

  RuleEditor* GetRuleEditor() { return m_RuleEditor; }
  const std::unordered_map<DWORD64, std::shared_ptr<class Rule> >* GetRules()
      override;

 private:
  ApplicationOptions options_;

  std::vector<std::string> m_Arguments;
  std::vector<RefreshCallback> m_RefreshCallbacks;
  std::vector<WatchCallback> m_AddToWatchCallbacks;
  std::vector<WatchCallback> m_UpdateWatchCallbacks;
  std::vector<SamplingReportCallback> m_SamplingReportsCallbacks;
  std::vector<SamplingReportCallback> m_SelectionReportCallbacks;
  std::vector<class DataView*> m_Panels;
  FindFileCallback m_FindFileCallback;
  SaveFileCallback m_SaveFileCallback;
  ClipboardCallback m_ClipboardCallback;
  bool m_IsRemote = false;

  ProcessesDataView* m_ProcessesDataView = nullptr;
  ModulesDataView* m_ModulesDataView = nullptr;
  FunctionsDataView* m_FunctionsDataView = nullptr;
  LiveFunctionsDataView* m_LiveFunctionsDataView = nullptr;
  CallStackDataView* m_CallStackDataView = nullptr;
  TypesDataView* m_TypesDataView = nullptr;
  GlobalsDataView* m_GlobalsDataView = nullptr;
  SessionsDataView* m_SessionsDataView = nullptr;
  CaptureWindow* m_CaptureWindow = nullptr;
  LogDataView* m_Log = nullptr;
  RuleEditor* m_RuleEditor = nullptr;
  int m_ScreenRes[2];
  bool m_HasPromptedForUpdate = false;
  bool m_NeedsThawing = false;
  bool m_UnrealEnabled = false;

  std::vector<std::shared_ptr<class SamplingReport> > m_SamplingReports;
  std::map<std::string, std::string> m_FileMapping;
  std::vector<std::string> m_SymbolDirectories;
  std::function<void(const std::string&)> m_UiCallback;

  // buffering data to send large messages instead of small ones:
  std::shared_ptr<std::thread> m_MessageBufferThread = nullptr;
  std::vector<ContextSwitch> m_ContextSwitchBuffer;
  Mutex m_ContextSwitchMutex;
  std::vector<Timer> m_TimerBuffer;
  Mutex m_TimerMutex;
  std::vector<LinuxCallstackEvent> m_SamplingCallstackBuffer;
  Mutex m_SamplingCallstackMutex;
  std::vector<CallstackEvent> m_HashedSamplingCallstackBuffer;
  Mutex m_HashedSamplingCallstackMutex;

  std::wstring m_User;
  std::wstring m_License;

  std::vector<std::shared_ptr<struct Module> > m_ModulesToLoad;
  std::vector<std::string> m_PostInitArguments;

  int m_NumTicks = 0;

  std::shared_ptr<StringManager> string_manager_ = nullptr;

  const SymbolHelper symbol_helper_;
#if defined(_WIN32)
  std::unique_ptr<Debugger> m_Debugger;
#else
  std::shared_ptr<class BpfTrace> m_BpfTrace;
#endif
};

//-----------------------------------------------------------------------------
extern std::unique_ptr<OrbitApp> GOrbitApp;
