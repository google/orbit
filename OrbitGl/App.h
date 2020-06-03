// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <outcome.hpp>
#include <queue>
#include <string>
#include <utility>

#include "ApplicationOptions.h"
#include "CallStackDataView.h"
#include "ContextSwitch.h"
#include "CoreApp.h"
#include "DataViewFactory.h"
#include "DataViewTypes.h"
#include "FramePointerValidatorClient.h"
#include "FunctionsDataView.h"
#include "GlobalsDataView.h"
#include "LiveFunctionsDataView.h"
#include "LogDataView.h"
#include "Message.h"
#include "ModulesDataView.h"
#include "OrbitBase/MainThreadExecutor.h"
#include "ProcessListManager.h"
#include "ProcessMemoryClient.h"
#include "ProcessesDataView.h"
#include "SamplingReportDataView.h"
#include "SessionsDataView.h"
#include "StringManager.h"
#include "SymbolHelper.h"
#include "SymbolsClient.h"
#include "Threading.h"
#include "TransactionClient.h"
#include "TypesDataView.h"
#include "absl/container/flat_hash_map.h"
#include "grpcpp/grpcpp.h"

#if defined(_WIN32)
#include "Debugger.h"
#endif

struct CallStack;
class Process;

//-----------------------------------------------------------------------------
class OrbitApp final : public CoreApp, public DataViewFactory {
 public:
  explicit OrbitApp(ApplicationOptions&& options);
  ~OrbitApp() override;

  static bool Init(ApplicationOptions&& options);
  void PostInit();
  void OnExit();
  static void MainTick();
  void SetLicense(const std::wstring& a_License);
  std::string GetVersion();

  std::string GetCaptureFileName();
  std::string GetSessionFileName();
  std::string GetSaveFile(const std::string& extension);
  void SetClipboard(const std::wstring& a_Text);
  outcome::result<void, std::string> OnSaveSession(
      const std::string& file_name);
  outcome::result<void, std::string> OnLoadSession(
      const std::string& file_name);
  outcome::result<void, std::string> OnSaveCapture(
      const std::string& file_name);
  outcome::result<void, std::string> OnLoadCapture(
      const std::string& file_name);
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
  void ProcessContextSwitch(const ContextSwitch& a_ContextSwitch) override;
  void AddAddressInfo(LinuxAddressInfo address_info) override;
  void AddKeyAndString(uint64_t key, std::string_view str) override;
  void UpdateThreadName(uint32_t thread_id,
                        const std::string& thread_name) override;

  int* GetScreenRes() { return m_ScreenRes; }

  void RegisterCaptureWindow(class CaptureWindow* a_Capture);

  void OnProcessSelected(uint32_t pid);

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
  using CaptureStartedCallback = std::function<void()>;
  void AddCaptureStartedCallback(CaptureStartedCallback callback) {
    capture_started_callbacks_.emplace_back(std::move(callback));
  }
  using CaptureStoppedCallback = std::function<void()>;
  void AddCaptureStoppedCallback(CaptureStoppedCallback callback) {
    capture_stopped_callbacks_.emplace_back(std::move(callback));
  }

  typedef std::function<void(DataViewType a_Type)> RefreshCallback;
  void AddRefreshCallback(RefreshCallback a_Callback) {
    m_RefreshCallbacks.emplace_back(std::move(a_Callback));
  }
  typedef std::function<void(DataView*, std::shared_ptr<class SamplingReport>)>
      SamplingReportCallback;
  void AddSamplingReportCallback(SamplingReportCallback a_Callback) {
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
  void SendInfoToUi(const std::string& title, const std::string& text);
  void SendErrorToUi(const std::string& title, const std::string& text);
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

  void EnableUploadDumpsToServer(bool a_Value);
  bool GetUploadDumpsToServerEnabled() const override;

  void RequestThaw() { m_NeedsThawing = true; }
  void OnRemoteProcess(const Message& a_Message);
  void OnRemoteModuleDebugInfo(const Message& a_Message);
  void OnRemoteModuleDebugInfo(const std::vector<ModuleDebugInfo>&) override;
  void UpdateSamplingReport();
  void ApplySession(const Session& session) override;
  void LoadSession(const std::shared_ptr<Session>& session);
  void SetIsRemote(bool a_IsRemote) { m_IsRemote = a_IsRemote; }
  bool IsRemote() const { return m_IsRemote; }
  bool HasTcpServer() const { return !IsRemote(); }
  void FilterFunctions(const std::string& filter);

  DataView* GetOrCreateDataView(DataViewType type) override;

  void InitializeClientTransactions();
  TransactionClient* GetTransactionClient() {
    return transaction_client_.get();
  }
  SymbolsClient* GetSymbolsClient() { return symbols_client_.get(); }
  FramePointerValidatorClient* GetFramePointerValidatorClient() {
    return frame_pointer_validator_client_.get();
  }
  void GetRemoteMemory(uint32_t pid, uint64_t address, uint64_t size,
                       const ProcessMemoryCallback& callback) override {
    process_memory_client_->GetRemoteMemory(pid, address, size, callback);
  }

 private:
  // TODO(dimitry): Move this to process manager
  std::shared_ptr<Process> FindProcessByPid(uint32_t pid);
  void UpdateProcess(const std::shared_ptr<Process>& process);

  ApplicationOptions options_;

  absl::Mutex process_map_mutex_;
  absl::flat_hash_map<uint32_t, std::shared_ptr<Process>> process_map_;

  std::vector<std::string> m_Arguments;
  std::vector<CaptureStartedCallback> capture_started_callbacks_;
  std::vector<CaptureStoppedCallback> capture_stopped_callbacks_;
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

  std::unique_ptr<ProcessesDataView> m_ProcessesDataView;
  std::unique_ptr<ModulesDataView> m_ModulesDataView;
  std::unique_ptr<FunctionsDataView> m_FunctionsDataView;
  std::unique_ptr<LiveFunctionsDataView> m_LiveFunctionsDataView;
  std::unique_ptr<CallStackDataView> m_CallStackDataView;
  std::unique_ptr<TypesDataView> m_TypesDataView;
  std::unique_ptr<GlobalsDataView> m_GlobalsDataView;
  std::unique_ptr<SessionsDataView> m_SessionsDataView;
  std::unique_ptr<LogDataView> m_LogDataView;

  CaptureWindow* m_CaptureWindow = nullptr;
  int m_ScreenRes[2];
  bool m_HasPromptedForUpdate = false;
  bool m_NeedsThawing = false;
  bool m_UnrealEnabled = false;

  std::shared_ptr<class SamplingReport> sampling_report_;
  std::shared_ptr<class SamplingReport> selection_report_;
  std::map<std::string, std::string> m_FileMapping;
  std::vector<std::string> m_SymbolDirectories;
  std::function<void(const std::string&)> m_UiCallback;

  std::wstring m_User;
  std::wstring m_License;

  std::vector<std::shared_ptr<struct Module>> m_ModulesToLoad;
  std::vector<std::string> m_PostInitArguments;

  int m_NumTicks = 0;

  std::shared_ptr<StringManager> string_manager_;
  std::shared_ptr<grpc::Channel> grpc_channel_;

  std::unique_ptr<MainThreadExecutor> main_thread_executor_;
  std::unique_ptr<ProcessListManager> process_list_manager_;

  const SymbolHelper symbol_helper_;
#if defined(_WIN32)
  std::unique_ptr<Debugger> m_Debugger;
#endif

 private:
  std::unique_ptr<TransactionClient> transaction_client_;
  std::unique_ptr<SymbolsClient> symbols_client_;
  std::unique_ptr<FramePointerValidatorClient> frame_pointer_validator_client_;
  std::unique_ptr<ProcessMemoryClient> process_memory_client_;
};

//-----------------------------------------------------------------------------
extern std::unique_ptr<OrbitApp> GOrbitApp;
