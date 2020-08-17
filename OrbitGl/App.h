// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_APP_H_
#define ORBIT_GL_APP_H_

#include <functional>
#include <map>
#include <memory>
#include <outcome.hpp>
#include <queue>
#include <string>
#include <utility>

#include "ApplicationOptions.h"
#include "CallStackDataView.h"
#include "Callstack.h"
#include "CaptureWindow.h"
#include "DataManager.h"
#include "DataViewFactory.h"
#include "DataViewTypes.h"
#include "DisassemblyReport.h"
#include "FramePointerValidatorClient.h"
#include "FunctionsDataView.h"
#include "LiveFunctionsDataView.h"
#include "MainThreadExecutor.h"
#include "ModulesDataView.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitCaptureClient/CaptureClient.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "OrbitClientServices/CrashManager.h"
#include "OrbitClientServices/ProcessManager.h"
#include "PresetsDataView.h"
#include "ProcessesDataView.h"
#include "SamplingReportDataView.h"
#include "ScopedStatus.h"
#include "StatusListener.h"
#include "StringManager.h"
#include "SymbolHelper.h"
#include "Threading.h"
#include "TopDownView.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "capture_data.pb.h"
#include "grpcpp/grpcpp.h"
#include "preset.pb.h"
#include "services.grpc.pb.h"
#include "services.pb.h"

class Process;

class OrbitApp final : public DataViewFactory, public CaptureListener {
 public:
  OrbitApp(ApplicationOptions&& options, std::unique_ptr<MainThreadExecutor> main_thread_executor);
  ~OrbitApp() override;

  static bool Init(ApplicationOptions&& options,
                   std::unique_ptr<MainThreadExecutor> main_thread_executor);
  void PostInit();
  void OnExit();
  static void MainTick();

  std::string GetCaptureFileName();
  std::string GetCaptureTime();
  std::string GetSaveFile(const std::string& extension);
  void SetClipboard(const std::string& text);
  ErrorMessageOr<void> OnSavePreset(const std::string& file_name);
  ErrorMessageOr<void> OnLoadPreset(const std::string& file_name);
  ErrorMessageOr<void> OnSaveCapture(const std::string& file_name);
  ErrorMessageOr<void> OnLoadCapture(const std::string& file_name);
  bool IsCapturing() const;
  bool StartCapture();
  void StopCapture();
  void ClearCapture();
  void ToggleDrawHelp();
  void ToggleCapture();
  void LoadFileMapping();
  void ListPresets();
  void RefreshCaptureView();
  void Disassemble(int32_t pid, const orbit_client_protos::FunctionInfo& function);

  void OnCaptureStarted() override;
  void OnCaptureComplete() override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  void OnKeyAndString(uint64_t key, std::string str) override;
  void OnCallstack(CallStack callstack) override;
  void OnCallstackEvent(orbit_client_protos::CallstackEvent callstack_event) override;
  void OnThreadName(int32_t thread_id, std::string thread_name) override;
  void OnAddressInfo(orbit_client_protos::LinuxAddressInfo address_info) override;

  void OnValidateFramePointers(std::vector<std::shared_ptr<Module>> modules_to_validate);

  void RegisterCaptureWindow(CaptureWindow* capture);

  void AddSamplingReport(std::shared_ptr<SamplingProfiler> sampling_profiler);
  void AddSelectionReport(std::shared_ptr<SamplingProfiler> sampling_profiler);
  void AddTopDownView(const SamplingProfiler& sampling_profiler);

  bool SelectProcess(const std::string& process);

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

  using CaptureClearedCallback = std::function<void()>;
  void SetCaptureClearedCallback(CaptureClearedCallback callback) {
    capture_cleared_callback_ = std::move(callback);
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
  using DisassemblyCallback = std::function<void(std::string, DisassemblyReport)>;
  void SetDisassemblyCallback(DisassemblyCallback callback) {
    disassembly_callback_ = std::move(callback);
  }
  using ErrorMessageCallback = std::function<void(const std::string&, const std::string&)>;
  void SetErrorMessageCallback(ErrorMessageCallback callback) {
    error_message_callback_ = std::move(callback);
  }
  using InfoMessageCallback = std::function<void(const std::string&, const std::string&)>;
  void SetInfoMessageCallback(InfoMessageCallback callback) {
    info_message_callback_ = std::move(callback);
  }
  using TooltipCallback = std::function<void(const std::string&)>;
  void SetTooltipCallback(TooltipCallback callback) { tooltip_callback_ = std::move(callback); }
  using RefreshCallback = std::function<void(DataViewType type)>;
  void SetRefreshCallback(RefreshCallback callback) { refresh_callback_ = std::move(callback); }
  using SamplingReportCallback = std::function<void(DataView*, std::shared_ptr<SamplingReport>)>;
  void SetSamplingReportCallback(SamplingReportCallback callback) {
    sampling_reports_callback_ = std::move(callback);
  }
  void SetSelectionReportCallback(SamplingReportCallback callback) {
    selection_report_callback_ = std::move(callback);
  }
  using TopDownViewCallback = std::function<void(std::unique_ptr<TopDownView>)>;
  void SetTopDownViewCallback(TopDownViewCallback callback) {
    top_down_view_callback_ = std::move(callback);
  }
  using SaveFileCallback = std::function<std::string(const std::string& extension)>;
  void SetSaveFileCallback(SaveFileCallback callback) { save_file_callback_ = std::move(callback); }
  void FireRefreshCallbacks(DataViewType type = DataViewType::kAll);
  void Refresh(DataViewType type = DataViewType::kAll) { FireRefreshCallbacks(type); }
  using ClipboardCallback = std::function<void(const std::string&)>;
  void SetClipboardCallback(ClipboardCallback callback) {
    clipboard_callback_ = std::move(callback);
  }

  using SecureCopyCallback =
      std::function<ErrorMessageOr<void>(std::string_view, std::string_view)>;
  void SetSecureCopyCallback(SecureCopyCallback callback) {
    secure_copy_callback_ = std::move(callback);
  }

  void SetStatusListener(StatusListener* listener) { status_listener_ = listener; }

  void SendDisassemblyToUi(std::string disassembly, DisassemblyReport report);
  void SendTooltipToUi(const std::string& tooltip);
  void SendInfoToUi(const std::string& title, const std::string& text);
  void SendErrorToUi(const std::string& title, const std::string& text);
  void NeedsRedraw();

  void LoadModules(int32_t process_id, const std::vector<std::shared_ptr<Module>>& modules,
                   const std::shared_ptr<orbit_client_protos::PresetFile>& preset = nullptr);
  void LoadModulesFromPreset(const std::shared_ptr<Process>& process,
                             const std::shared_ptr<orbit_client_protos::PresetFile>& preset);
  void UpdateProcessAndModuleList(int32_t pid);

  void UpdateSamplingReport();
  void LoadPreset(const std::shared_ptr<orbit_client_protos::PresetFile>& session);
  void FilterTracks(const std::string& filter);

  void CrashOrbitService(orbit_grpc_protos::CrashOrbitServiceRequest_CrashType crash_type);

  DataView* GetOrCreateDataView(DataViewType type) override;

  [[nodiscard]] ProcessManager* GetProcessManager() { return process_manager_.get(); }
  [[nodiscard]] ThreadPool* GetThreadPool() { return thread_pool_.get(); }
  [[nodiscard]] MainThreadExecutor* GetMainThreadExecutor() { return main_thread_executor_.get(); }
  [[nodiscard]] std::shared_ptr<Process> FindProcessByPid(int32_t pid);

  // TODO(kuebler): Move them to a separate controler at some point
  void SelectFunction(const orbit_client_protos::FunctionInfo& func);
  void DeselectFunction(const orbit_client_protos::FunctionInfo& func);
  void ClearSelectedFunctions();
  [[nodiscard]] bool IsFunctionSelected(const orbit_client_protos::FunctionInfo& func) const;
  [[nodiscard]] bool IsFunctionSelected(const SampledFunction& func) const;

  void SetVisibleFunctions(absl::flat_hash_set<uint64_t> visible_functions);
  [[nodiscard]] bool IsFunctionVisible(uint64_t function_address);

  [[nodiscard]] ThreadID selected_thread_id() const;
  void set_selected_thread_id(ThreadID thread_id);

 private:
  void LoadModuleOnRemote(int32_t process_id, const std::shared_ptr<Module>& module,
                          const std::shared_ptr<orbit_client_protos::PresetFile>& preset);
  void SymbolLoadingFinished(uint32_t process_id, const std::shared_ptr<Module>& module,
                             const std::shared_ptr<orbit_client_protos::PresetFile>& preset);

  ErrorMessageOr<orbit_client_protos::PresetInfo> ReadPresetFromFile(const std::string& filename);

  [[nodiscard]] absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo>
  GetSelectedFunctionsAndOrbitFunctions() const;
  ErrorMessageOr<void> SavePreset(const std::string& filename);
  [[nodiscard]] ScopedStatus CreateScopedStatus(const std::string& initial_message);

  ApplicationOptions options_;

  absl::Mutex process_map_mutex_;
  absl::flat_hash_map<uint32_t, std::shared_ptr<Process>> process_map_;

  CaptureStartedCallback capture_started_callback_;
  CaptureStopRequestedCallback capture_stop_requested_callback_;
  CaptureStoppedCallback capture_stopped_callback_;
  CaptureClearedCallback capture_cleared_callback_;
  OpenCaptureCallback open_capture_callback_;
  SaveCaptureCallback save_capture_callback_;
  SelectLiveTabCallback select_live_tab_callback_;
  DisassemblyCallback disassembly_callback_;
  ErrorMessageCallback error_message_callback_;
  InfoMessageCallback info_message_callback_;
  TooltipCallback tooltip_callback_;
  RefreshCallback refresh_callback_;
  SamplingReportCallback sampling_reports_callback_;
  SamplingReportCallback selection_report_callback_;
  TopDownViewCallback top_down_view_callback_;
  std::vector<class DataView*> m_Panels;
  SaveFileCallback save_file_callback_;
  ClipboardCallback clipboard_callback_;
  SecureCopyCallback secure_copy_callback_;

  std::unique_ptr<ProcessesDataView> processes_data_view_;
  std::unique_ptr<ModulesDataView> modules_data_view_;
  std::unique_ptr<FunctionsDataView> functions_data_view_;
  std::unique_ptr<LiveFunctionsDataView> live_functions_data_view_;
  std::unique_ptr<CallStackDataView> callstack_data_view_;
  std::unique_ptr<PresetsDataView> presets_data_view_;

  CaptureWindow* capture_window_ = nullptr;

  std::shared_ptr<SamplingReport> sampling_report_;
  std::shared_ptr<SamplingReport> selection_report_;
  std::map<std::string, std::string> file_mapping_;

  absl::flat_hash_set<std::string> modules_currently_loading_;

  std::shared_ptr<StringManager> string_manager_;
  std::shared_ptr<grpc::Channel> grpc_channel_;

  std::unique_ptr<MainThreadExecutor> main_thread_executor_;
  std::thread::id main_thread_id_;
  std::unique_ptr<ThreadPool> thread_pool_;
  std::unique_ptr<CaptureClient> capture_client_;
  std::unique_ptr<ProcessManager> process_manager_;
  std::unique_ptr<DataManager> data_manager_;
  std::unique_ptr<CrashManager> crash_manager_;

  const SymbolHelper symbol_helper_;

  std::unique_ptr<FramePointerValidatorClient> frame_pointer_validator_client_;

  // Temporary objects used by CaptureListener implementation
  absl::flat_hash_map<uint64_t, orbit_client_protos::LinuxAddressInfo> captured_address_infos_;
  StatusListener* status_listener_ = nullptr;
};

extern std::unique_ptr<OrbitApp> GOrbitApp;

#endif  // ORBIT_GL_APP_H_
