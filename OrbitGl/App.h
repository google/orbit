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
#include "CaptureClient.h"
#include "CaptureListener.h"
#include "ContextSwitch.h"
#include "CoreApp.h"
#include "CrashManager.h"
#include "DataManager.h"
#include "DataViewFactory.h"
#include "DataViewTypes.h"
#include "FramePointerValidatorClient.h"
#include "FunctionsDataView.h"
#include "GlobalsDataView.h"
#include "LiveFunctionsDataView.h"
#include "Message.h"
#include "ModulesDataView.h"
#include "OrbitBase/MainThreadExecutor.h"
#include "OrbitBase/ThreadPool.h"
#include "ProcessManager.h"
#include "ProcessesDataView.h"
#include "SamplingReportDataView.h"
#include "SessionsDataView.h"
#include "StringManager.h"
#include "SymbolHelper.h"
#include "Threading.h"
#include "TypesDataView.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "grpcpp/grpcpp.h"
#include "services.grpc.pb.h"
#include "services.pb.h"

struct CallStack;
class Process;

//-----------------------------------------------------------------------------
class OrbitApp final : public CoreApp,
                       public DataViewFactory,
                       public CaptureListener {
 public:
  explicit OrbitApp(ApplicationOptions&& options);
  ~OrbitApp() override;

  static bool Init(ApplicationOptions&& options);
  void PostInit();
  void OnExit();
  static void MainTick();

  std::string GetCaptureFileName();
  std::string GetSaveFile(const std::string& extension);
  void SetClipboard(const std::string& text);
  outcome::result<void, std::string> OnSavePreset(const std::string& file_name);
  outcome::result<void, std::string> OnLoadPreset(const std::string& file_name);
  outcome::result<void, std::string> OnSaveCapture(
      const std::string& file_name);
  outcome::result<void, std::string> OnLoadCapture(
      const std::string& file_name);
  bool StartCapture();
  void StopCapture();
  void OnCaptureStopped() override;
  void ToggleCapture();
  void SetCallStack(std::shared_ptr<CallStack> a_CallStack);
  void LoadFileMapping();
  void ListPresets();
  void RefreshCaptureView() override;
  void ClearWatchedVariables();
  void Disassemble(int32_t pid, const Function& function);

  void ProcessTimer(const Timer& timer) override;
  void ProcessSamplingCallStack(LinuxCallstackEvent& a_CallStack) override;
  void ProcessHashedSamplingCallStack(CallstackEvent& a_CallStack) override;
  void AddAddressInfo(LinuxAddressInfo address_info) override;
  void AddKeyAndString(uint64_t key, std::string_view str) override;
  void UpdateThreadName(int32_t thread_id,
                        const std::string& thread_name) override;

  void OnTimer(Timer timer) override;
  void OnKeyAndString(uint64_t key, std::string str) override;
  void OnCallstack(CallStack callstack) override;
  void OnCallstackEvent(CallstackEvent callstack_event) override;
  void OnThreadName(int32_t thread_id, std::string thread_name) override;
  void OnAddressInfo(LinuxAddressInfo address_info) override;

  void OnValidateFramePointers(
      std::vector<std::shared_ptr<Module>> modules_to_validate);

  void RegisterCaptureWindow(class CaptureWindow* a_Capture);

  void OnProcessSelected(int32_t pid);

  void AddSamplingReport(
      std::shared_ptr<class SamplingProfiler>& sampling_profiler);
  void AddSelectionReport(
      std::shared_ptr<SamplingProfiler>& a_SamplingProfiler);

  bool SelectProcess(const std::string& a_Process);
  bool SelectProcess(int32_t a_ProcessID);
  bool Inject(unsigned long a_ProcessId);

  // Callbacks
  using CaptureStartedCallback = std::function<void()>;
  void SetCaptureStartedCallback(CaptureStartedCallback callback) {
    capture_started_callback_ = std::move(callback);
  }
  using CaptureStopRequestedCallback = std::function<void()>;
  void SetCaptureStopRequestedCallback(CaptureStopRequestedCallback callback) {
    capture_stop_requested_callback_ = std::move(callback);
  }
  using CaptureStoppedCallback = std::function<void()>;
  void SetCaptureStoppedCallback(CaptureStoppedCallback callback) {
    capture_stopped_callback_ = std::move(callback);
  }

  using OpenCaptureCallback = std::function<void()>;
  void SetOpenCaptureCallback(OpenCaptureCallback callback) {
    open_capture_callback_ = std::move(callback);
  }
  using SaveCaptureCallback = std::function<void()>;
  void SetSaveCaptureCallback(SaveCaptureCallback callback) {
    save_capture_callback_ = std::move(callback);
  }
  using SelectLiveTabCallback = std::function<void()>;
  void SetSelectLiveTabCallback(SelectLiveTabCallback callback) {
    select_live_tab_callback_ = std::move(callback);
  }
  using DisassemblyCallback = std::function<void(const std::string&)>;
  void SetDisassemblyCallback(DisassemblyCallback callback) {
    disassembly_callback_ = std::move(callback);
  }
  using ErrorMessageCallback =
      std::function<void(const std::string&, const std::string&)>;
  void SetErrorMessageCallback(ErrorMessageCallback callback) {
    error_message_callback_ = std::move(callback);
  }
  using InfoMessageCallback =
      std::function<void(const std::string&, const std::string&)>;
  void SetInfoMessageCallback(InfoMessageCallback callback) {
    info_message_callback_ = std::move(callback);
  }
  using TooltipCallback = std::function<void(const std::string&)>;
  void SetTooltipCallback(TooltipCallback callback) {
    tooltip_callback_ = std::move(callback);
  }
  using FeedbackDialogCallback = std::function<void()>;
  void SetFeedbackDialogCallback(FeedbackDialogCallback callback) {
    feedback_dialog_callback_ = std::move(callback);
  }

  using RefreshCallback = std::function<void(DataViewType type)>;
  void SetRefreshCallback(RefreshCallback callback) {
    refresh_callback_ = std::move(callback);
  }
  using SamplingReportCallback =
      std::function<void(DataView*, std::shared_ptr<SamplingReport>)>;
  void SetSamplingReportCallback(SamplingReportCallback callback) {
    sampling_reports_callback_ = std::move(callback);
  }
  void SetSelectionReportCallback(SamplingReportCallback callback) {
    selection_report_callback_ = std::move(callback);
  }
  using SaveFileCallback =
      std::function<std::string(const std::string& extension)>;
  void SetSaveFileCallback(SaveFileCallback callback) {
    save_file_callback_ = std::move(callback);
  }
  void FireRefreshCallbacks(DataViewType type = DataViewType::ALL);
  void Refresh(DataViewType a_Type = DataViewType::ALL) {
    FireRefreshCallbacks(a_Type);
  }
  typedef std::function<std::string(const std::string& caption,
                                    const std::string& dir,
                                    const std::string& filter)>
      FindFileCallback;
  void SetFindFileCallback(FindFileCallback callback) {
    find_file_callback_ = std::move(callback);
  }
  std::string FindFile(const std::string& caption, const std::string& dir,
                       const std::string& filter);
  typedef std::function<void(const std::string&)> ClipboardCallback;
  void SetClipboardCallback(ClipboardCallback callback) {
    clipboard_callback_ = std::move(callback);
  }

  void SetCommandLineArguments(const std::vector<std::string>& a_Args);

  void RequestOpenCaptureToUi();
  void RequestSaveCaptureToUi();
  void SendDisassemblyToUi(const std::string& disassembly);
  void SendTooltipToUi(const std::string& tooltip);
  void RequestFeedbackDialogToUi();
  void SendInfoToUi(const std::string& title, const std::string& text);
  void SendErrorToUi(const std::string& title, const std::string& text);
  void NeedsRedraw();

  const std::map<std::string, std::string>& GetFileMapping() {
    return m_FileMapping;
  }

  void LoadModules(int32_t process_id,
                   const std::vector<std::shared_ptr<Module>>& modules,
                   const std::shared_ptr<Preset>& preset = nullptr);
  void LoadModulesFromPreset(const std::shared_ptr<Process>& process,
                             const std::shared_ptr<Preset>& preset);
  bool IsLoading();

  void UpdateSamplingReport();
  void LoadPreset(const std::shared_ptr<Preset>& session);
  void FilterFunctions(const std::string& filter);

  void CrashOrbitService(CrashOrbitServiceRequest_CrashType crash_type);

  DataView* GetOrCreateDataView(DataViewType type) override;

 private:
  void LoadModuleOnRemote(int32_t process_id,
                          const std::shared_ptr<Module>& module,
                          const std::shared_ptr<Preset>& preset);
  void SymbolLoadingFinished(uint32_t process_id,
                             const std::shared_ptr<Module>& module,
                             const std::shared_ptr<Preset>& preset);
  std::shared_ptr<Process> FindProcessByPid(int32_t pid);

  outcome::result<void, std::string> ReadPresetFromFile(
      const std::string& filename, Preset* preset);

  ApplicationOptions options_;

  absl::Mutex process_map_mutex_;
  absl::flat_hash_map<uint32_t, std::shared_ptr<Process>> process_map_;

  CaptureStartedCallback capture_started_callback_;
  CaptureStopRequestedCallback capture_stop_requested_callback_;
  CaptureStoppedCallback capture_stopped_callback_;
  OpenCaptureCallback open_capture_callback_;
  SaveCaptureCallback save_capture_callback_;
  SelectLiveTabCallback select_live_tab_callback_;
  DisassemblyCallback disassembly_callback_;
  ErrorMessageCallback error_message_callback_;
  InfoMessageCallback info_message_callback_;
  TooltipCallback tooltip_callback_;
  FeedbackDialogCallback feedback_dialog_callback_;
  RefreshCallback refresh_callback_;
  SamplingReportCallback sampling_reports_callback_;
  SamplingReportCallback selection_report_callback_;
  std::vector<class DataView*> m_Panels;
  FindFileCallback find_file_callback_;
  SaveFileCallback save_file_callback_;
  ClipboardCallback clipboard_callback_;

  std::unique_ptr<ProcessesDataView> m_ProcessesDataView;
  std::unique_ptr<ModulesDataView> m_ModulesDataView;
  std::unique_ptr<FunctionsDataView> m_FunctionsDataView;
  std::unique_ptr<LiveFunctionsDataView> m_LiveFunctionsDataView;
  std::unique_ptr<CallStackDataView> m_CallStackDataView;
  std::unique_ptr<TypesDataView> m_TypesDataView;
  std::unique_ptr<GlobalsDataView> m_GlobalsDataView;
  std::unique_ptr<PresetsDataView> m_PresetsDataView;

  CaptureWindow* m_CaptureWindow = nullptr;

  std::shared_ptr<class SamplingReport> sampling_report_;
  std::shared_ptr<class SamplingReport> selection_report_;
  std::map<std::string, std::string> m_FileMapping;

  int m_NumTicks = 0;

  absl::flat_hash_set<std::string> modules_currently_loading_;

  std::shared_ptr<StringManager> string_manager_;
  std::shared_ptr<grpc::Channel> grpc_channel_;

  std::unique_ptr<MainThreadExecutor> main_thread_executor_;
  std::unique_ptr<ThreadPool> thread_pool_;
  std::unique_ptr<CaptureClient> capture_client_;
  std::unique_ptr<ProcessManager> process_manager_;
  std::unique_ptr<DataManager> data_manager_;
  std::unique_ptr<CrashManager> crash_manager_;

  const SymbolHelper symbol_helper_;

  std::unique_ptr<FramePointerValidatorClient> frame_pointer_validator_client_;
};

//-----------------------------------------------------------------------------
extern std::unique_ptr<OrbitApp> GOrbitApp;
