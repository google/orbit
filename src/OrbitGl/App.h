// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_APP_H_
#define ORBIT_GL_APP_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/types/span.h>
#include <grpc/impl/codegen/connectivity_state.h>
#include <grpcpp/channel.h>

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <outcome.hpp>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "CallStackDataView.h"
#include "CallTreeView.h"
#include "CaptureClient/CaptureClient.h"
#include "CaptureClient/CaptureListener.h"
#include "CaptureWindow.h"
#include "DataManager.h"
#include "DataView.h"
#include "DataViewFactory.h"
#include "DataViewTypes.h"
#include "DisassemblyReport.h"
#include "FramePointerValidatorClient.h"
#include "FrameTrackOnlineProcessor.h"
#include "FunctionsDataView.h"
#include "GlCanvas.h"
#include "IntrospectionWindow.h"
#include "MainThreadExecutor.h"
#include "MainWindowInterface.h"
#include "ManualInstrumentationManager.h"
#include "MetricsUploader/MetricsUploader.h"
#include "ModulesDataView.h"
#include "OrbitBase/CrashHandler.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitClientData/Callstack.h"
#include "OrbitClientData/CallstackTypes.h"
#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ModuleManager.h"
#include "OrbitClientData/PostProcessedSamplingData.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientData/TracepointCustom.h"
#include "OrbitClientData/UserDefinedCaptureData.h"
#include "OrbitClientModel/CaptureData.h"
#include "OrbitClientServices/CrashManager.h"
#include "OrbitClientServices/ProcessManager.h"
#include "OrbitClientServices/TracepointServiceClient.h"
#include "PresetLoadState.h"
#include "PresetsDataView.h"
#include "SamplingReport.h"
#include "ScopedStatus.h"
#include "StatusListener.h"
#include "StringManager.h"
#include "SymbolHelper.h"
#include "TextBox.h"
#include "TracepointsDataView.h"
#include "capture.pb.h"
#include "capture_data.pb.h"
#include "preset.pb.h"
#include "services.pb.h"
#include "symbol.pb.h"
#include "tracepoint.pb.h"

class OrbitApp final : public DataViewFactory, public orbit_capture_client::CaptureListener {
 public:
  explicit OrbitApp(orbit_gl::MainWindowInterface* main_window,
                    MainThreadExecutor* main_thread_executor,
                    const orbit_base::CrashHandler* crash_handler,
                    orbit_metrics_uploader::MetricsUploader* metrics_uploader = nullptr);
  ~OrbitApp() override;

  static std::unique_ptr<OrbitApp> Create(
      orbit_gl::MainWindowInterface* main_window, MainThreadExecutor* main_thread_executor,
      const orbit_base::CrashHandler* crash_handler,
      orbit_metrics_uploader::MetricsUploader* metrics_uploader = nullptr);

  void PostInit(bool is_connected);
  void MainTick();

  [[nodiscard]] std::string GetCaptureTime() const;
  [[nodiscard]] std::string GetSaveFile(const std::string& extension) const;
  void SetClipboard(const std::string& text);
  ErrorMessageOr<void> OnSavePreset(const std::string& file_name);
  ErrorMessageOr<void> OnLoadPreset(const std::string& file_name);
  ErrorMessageOr<void> OnSaveCapture(const std::filesystem::path& file_name);
  orbit_base::Future<ErrorMessageOr<CaptureOutcome>> LoadCaptureFromFile(
      const std::string& file_name);
  void OnLoadCaptureCancelRequested();

  [[nodiscard]] orbit_capture_client::CaptureClient::State GetCaptureState() const;
  [[nodiscard]] bool IsCapturing() const;

  void StartCapture();
  void StopCapture();
  void AbortCapture();
  void ClearCapture();
  [[nodiscard]] bool HasCaptureData() const { return capture_data_ != nullptr; }
  [[nodiscard]] CaptureData& GetMutableCaptureData() {
    CHECK(capture_data_ != nullptr);
    return *capture_data_;
  }
  [[nodiscard]] const CaptureData& GetCaptureData() const {
    CHECK(capture_data_ != nullptr);
    return *capture_data_;
  }

  [[nodiscard]] bool HasSampleSelection() const {
    return selection_report_ != nullptr && selection_report_->HasSamples();
  }

  void ToggleDrawHelp();
  void ToggleCapture();
  void ListPresets();
  void RefreshCaptureView();
  void Disassemble(int32_t pid, const orbit_client_protos::FunctionInfo& function);
  void ShowSourceCode(const orbit_client_protos::FunctionInfo& function);

  void OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& capture_started,
                        absl::flat_hash_set<uint64_t> frame_track_function_ids) override;
  void OnCaptureFinished(const orbit_grpc_protos::CaptureFinished& capture_finished) override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  void OnKeyAndString(uint64_t key, std::string str) override;
  void OnUniqueCallStack(CallStack callstack) override;
  void OnCallstackEvent(orbit_client_protos::CallstackEvent callstack_event) override;
  void OnThreadName(int32_t thread_id, std::string thread_name) override;
  void OnThreadStateSlice(orbit_client_protos::ThreadStateSliceInfo thread_state_slice) override;
  void OnAddressInfo(orbit_client_protos::LinuxAddressInfo address_info) override;
  void OnUniqueTracepointInfo(uint64_t key,
                              orbit_grpc_protos::TracepointInfo tracepoint_info) override;
  void OnTracepointEvent(orbit_client_protos::TracepointEventInfo tracepoint_event_info) override;
  void OnModuleUpdate(uint64_t timestamp_ns, orbit_grpc_protos::ModuleInfo module_info) override;
  void OnModulesSnapshot(uint64_t timestamp_ns,
                         std::vector<orbit_grpc_protos::ModuleInfo> module_infos) override;

  enum class SystemMemoryUsageEncodingIndex {
    kTotalKb,
    kFreeKb,
    kAvailableKb,
    kBuffersKb,
    kCachedKb,
    kEnd
  };
  void OnSystemMemoryUsage(
      const orbit_grpc_protos::SystemMemoryUsage& system_memory_usage) override;

  void OnValidateFramePointers(std::vector<const ModuleData*> modules_to_validate);

  void SetCaptureWindow(CaptureWindow* capture);
  [[nodiscard]] const TimeGraph* GetTimeGraph() const {
    CHECK(capture_window_ != nullptr);
    return capture_window_->GetTimeGraph();
  }
  [[nodiscard]] TimeGraph* GetMutableTimeGraph() {
    CHECK(capture_window_ != nullptr);
    return capture_window_->GetTimeGraph();
  }
  void SetDebugCanvas(GlCanvas* debug_canvas);
  void SetIntrospectionWindow(IntrospectionWindow* canvas);
  void StopIntrospection();

  void SetSamplingReport(
      PostProcessedSamplingData post_processed_sampling_data,
      absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> unique_callstacks);
  void SetSelectionReport(
      PostProcessedSamplingData post_processed_sampling_data,
      absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> unique_callstacks,
      bool has_summary);
  void SetTopDownView(const CaptureData& capture_data);
  void ClearTopDownView();
  void SetSelectionTopDownView(const PostProcessedSamplingData& selection_post_processed_data,
                               const CaptureData& capture_data);
  void ClearSelectionTopDownView();

  void SetBottomUpView(const CaptureData& capture_data);
  void ClearBottomUpView();
  void SetSelectionBottomUpView(const PostProcessedSamplingData& selection_post_processed_data,
                                const CaptureData& capture_data);
  void ClearSelectionBottomUpView();

  // This needs to be called from the main thread.
  [[nodiscard]] bool IsCaptureConnected(const CaptureData& capture) const;

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
  using CaptureFailedCallback = std::function<void()>;
  void SetCaptureFailedCallback(CaptureFailedCallback callback) {
    capture_failed_callback_ = std::move(callback);
  }

  using CaptureClearedCallback = std::function<void()>;
  void SetCaptureClearedCallback(CaptureClearedCallback callback) {
    capture_cleared_callback_ = std::move(callback);
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
  using WarningMessageCallback = std::function<void(const std::string&, const std::string&)>;
  void SetWarningMessageCallback(WarningMessageCallback callback) {
    warning_message_callback_ = std::move(callback);
  }
  using InfoMessageCallback = std::function<void(const std::string&, const std::string&)>;
  void SetInfoMessageCallback(InfoMessageCallback callback) {
    info_message_callback_ = std::move(callback);
  }
  using RefreshCallback = std::function<void(DataViewType type)>;
  void SetRefreshCallback(RefreshCallback callback) { refresh_callback_ = std::move(callback); }

  using SamplingReportCallback =
      std::function<void(DataView*, const std::shared_ptr<SamplingReport>&)>;
  void SetSamplingReportCallback(SamplingReportCallback callback) {
    sampling_reports_callback_ = std::move(callback);
  }
  void SetSelectionReportCallback(SamplingReportCallback callback) {
    selection_report_callback_ = std::move(callback);
  }

  using CallTreeViewCallback = std::function<void(std::unique_ptr<CallTreeView>)>;
  void SetTopDownViewCallback(CallTreeViewCallback callback) {
    top_down_view_callback_ = std::move(callback);
  }
  void SetSelectionTopDownViewCallback(CallTreeViewCallback callback) {
    selection_top_down_view_callback_ = std::move(callback);
  }
  void SetBottomUpViewCallback(CallTreeViewCallback callback) {
    bottom_up_view_callback_ = std::move(callback);
  }
  void SetSelectionBottomUpViewCallback(CallTreeViewCallback callback) {
    selection_bottom_up_view_callback_ = std::move(callback);
  }
  using TimerSelectedCallback = std::function<void(const orbit_client_protos::TimerInfo*)>;
  void SetTimerSelectedCallback(TimerSelectedCallback callback) {
    timer_selected_callback_ = std::move(callback);
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
  using ShowEmptyFrameTrackWarningCallback = std::function<void(std::string_view)>;
  void SetShowEmptyFrameTrackWarningCallback(ShowEmptyFrameTrackWarningCallback callback) {
    empty_frame_track_warning_callback_ = std::move(callback);
  }

  void SetStatusListener(StatusListener* listener) { status_listener_ = listener; }

  void SendDisassemblyToUi(std::string disassembly, DisassemblyReport report);
  void SendTooltipToUi(const std::string& tooltip);
  void SendInfoToUi(const std::string& title, const std::string& text);
  void SendWarningToUi(const std::string& title, const std::string& text);
  void SendErrorToUi(const std::string& title, const std::string& text);
  void RenderImGuiDebugUI();

  // RetrieveModule retrieves a module file and returns the local file path (potentially from the
  // local cache). Only modules with a .symtab section will be considered.
  orbit_base::Future<ErrorMessageOr<std::filesystem::path>> RetrieveModule(
      const std::string& module_path, const std::string& build_id);
  orbit_base::Future<void> RetrieveModulesAndLoadSymbols(
      absl::Span<const ModuleData* const> modules);

  // RetrieveModuleAndSymbols is a helper function which first retrieves the module by calling
  // `RetrieveModule` and afterwards load the symbols by calling `LoadSymbols`.
  orbit_base::Future<ErrorMessageOr<void>> RetrieveModuleAndLoadSymbols(const ModuleData* module);
  orbit_base::Future<ErrorMessageOr<void>> RetrieveModuleAndLoadSymbols(
      const std::string& module_path, const std::string& build_id);

  // This method is pretty similar to `RetrieveModule`, but it also requires debug information to be
  // present.
  orbit_base::Future<ErrorMessageOr<std::filesystem::path>> RetrieveModuleWithDebugInfo(
      const ModuleData* module);
  orbit_base::Future<ErrorMessageOr<std::filesystem::path>> RetrieveModuleWithDebugInfo(
      const std::string& module_path, const std::string& build_id);

  void UpdateProcessAndModuleList();
  orbit_base::Future<std::vector<ErrorMessageOr<void>>> ReloadModules(
      absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos);
  void RefreshUIAfterModuleReload();

  void UpdateAfterSymbolLoading();
  void UpdateAfterCaptureCleared();

  orbit_base::Future<ErrorMessageOr<void>> LoadPresetModule(
      const std::string& module_path, const orbit_client_protos::PresetModule& preset_module);
  void LoadPreset(const std::shared_ptr<orbit_client_protos::PresetFile>& preset);
  PresetLoadState GetPresetLoadState(
      const std::shared_ptr<orbit_client_protos::PresetFile>& preset) const;
  void FilterTracks(const std::string& filter);

  void CrashOrbitService(orbit_grpc_protos::CrashOrbitServiceRequest_CrashType crash_type);

  void SetGrpcChannel(std::shared_ptr<grpc::Channel> grpc_channel) {
    CHECK(grpc_channel_ == nullptr);
    CHECK(grpc_channel != nullptr);
    grpc_channel_ = std::move(grpc_channel);
  }
  void SetProcessManager(ProcessManager* process_manager) {
    CHECK(process_manager_ == nullptr);
    CHECK(process_manager != nullptr);
    process_manager_ = process_manager;
  }
  void SetTargetProcess(ProcessData* process);
  [[nodiscard]] DataView* GetOrCreateDataView(DataViewType type) override;
  [[nodiscard]] DataView* GetOrCreateSelectionCallstackDataView();

  [[nodiscard]] StringManager* GetStringManager() { return &string_manager_; }
  [[nodiscard]] ProcessManager* GetProcessManager() { return process_manager_; }
  [[nodiscard]] ThreadPool* GetThreadPool() { return thread_pool_.get(); }
  [[nodiscard]] MainThreadExecutor* GetMainThreadExecutor() { return main_thread_executor_; }
  [[nodiscard]] ProcessData* GetMutableTargetProcess() const { return process_; }
  [[nodiscard]] const ProcessData* GetTargetProcess() const { return process_; }
  [[nodiscard]] ManualInstrumentationManager* GetManualInstrumentationManager() {
    return manual_instrumentation_manager_.get();
  }
  [[nodiscard]] ModuleData* GetMutableModuleByPathAndBuildId(const std::string& path,
                                                             const std::string& build_id) const {
    return module_manager_->GetMutableModuleByPathAndBuildId(path, build_id);
  }
  [[nodiscard]] const ModuleData* GetModuleByPathAndBuildId(const std::string& path,
                                                            const std::string& build_id) const {
    return module_manager_->GetModuleByPathAndBuildId(path, build_id);
  }

  void SetCollectThreadStates(bool collect_thread_states);
  void SetSamplesPerSecond(double samples_per_second);
  void SetUnwindingMethod(orbit_grpc_protos::UnwindingMethod unwinding_method);
  void SetMaxLocalMarkerDepthPerCommandBuffer(uint64_t max_local_marker_depth_per_command_buffer);

  void SetCollectMemoryInfo(bool collect_memory_info) {
    data_manager_->set_collect_memory_info(collect_memory_info);
  }
  [[nodiscard]] bool GetCollectMemoryInfo() const { return data_manager_->collect_memory_info(); }
  void SetMemorySamplingPeriodNs(uint64_t memory_sampling_period_ns) {
    data_manager_->set_memory_sampling_period_ns(memory_sampling_period_ns);
  }
  void SetMemoryWarningThresholdKb(uint64_t memory_warning_threshold_kb) {
    data_manager_->set_memory_warning_threshold_kb(memory_warning_threshold_kb);
  }
  [[nodiscard]] uint64_t GetMemoryWarningThresholdKb() const {
    return data_manager_->memory_warning_threshold_kb();
  }

  // TODO(kuebler): Move them to a separate controler at some point
  void SelectFunction(const orbit_client_protos::FunctionInfo& func);
  void DeselectFunction(const orbit_client_protos::FunctionInfo& func);
  [[nodiscard]] bool IsFunctionSelected(const orbit_client_protos::FunctionInfo& func) const;
  [[nodiscard]] bool IsFunctionSelected(const SampledFunction& func) const;
  [[nodiscard]] bool IsFunctionSelected(uint64_t absolute_address) const;
  [[nodiscard]] const orbit_grpc_protos::InstrumentedFunction* GetInstrumentedFunction(
      uint64_t function_id) const;

  void SetVisibleFunctionIds(absl::flat_hash_set<uint64_t> visible_functions);
  [[nodiscard]] bool IsFunctionVisible(uint64_t function_id);

  [[nodiscard]] uint64_t highlighted_function_id() const;
  void set_highlighted_function_id(uint64_t highlighted_function_id);

  [[nodiscard]] ThreadID selected_thread_id() const;
  void set_selected_thread_id(ThreadID thread_id);

  [[nodiscard]] const TextBox* selected_text_box() const;
  void SelectTextBox(const TextBox* text_box);
  void DeselectTextBox();

  [[nodiscard]] uint64_t GetFunctionIdToHighlight() const;

  void SelectCallstackEvents(
      const std::vector<orbit_client_protos::CallstackEvent>& selected_callstack_events,
      int32_t thread_id);

  void SelectTracepoint(const TracepointInfo& info);
  void DeselectTracepoint(const TracepointInfo& tracepoint);

  [[nodiscard]] bool IsTracepointSelected(const TracepointInfo& info) const;

  // Only enables the frame track in the capture settings (in DataManager) and does not
  // add a frame track to the current capture data.
  void EnableFrameTrack(const orbit_client_protos::FunctionInfo& function);

  // Only disables the frame track in the capture settings (in DataManager) and does
  // not remove the frame track from the capture data.
  void DisableFrameTrack(const orbit_client_protos::FunctionInfo& function);

  [[nodiscard]] bool IsFrameTrackEnabled(const orbit_client_protos::FunctionInfo& function) const;

  // Adds the frame track to the capture settings and also adds a frame track to the current
  // capture data, *if* the captures contains function calls to the function and the function
  // was instrumented.
  void AddFrameTrack(const orbit_client_protos::FunctionInfo& function);
  void AddFrameTrack(uint64_t instrumented_function_id);

  // Removes the frame track from the capture settings and also removes the frame track
  // (if it exists) from the capture data.
  void RemoveFrameTrack(const orbit_client_protos::FunctionInfo& function);
  void RemoveFrameTrack(uint64_t instrumented_function_id);

  [[nodiscard]] bool HasFrameTrackInCaptureData(uint64_t instrumented_function_id) const;

 private:
  void UpdateModulesAbortCaptureIfModuleWithoutBuildIdNeedsReload(
      absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos);
  void AddSymbols(const std::filesystem::path& module_file_path, const std::string& module_build_id,
                  const orbit_grpc_protos::ModuleSymbols& symbols);

  [[nodiscard]] orbit_base::Future<ErrorMessageOr<std::filesystem::path>> RetrieveModuleFromRemote(
      const std::string& module_file_path);

  void SelectFunctionsFromHashes(const ModuleData* module,
                                 absl::Span<const uint64_t> function_hashes);

  ErrorMessageOr<std::filesystem::path> FindModuleLocally(const std::filesystem::path& module_path,
                                                          const std::string& build_id);
  [[nodiscard]] orbit_base::Future<ErrorMessageOr<void>> LoadSymbols(
      const std::filesystem::path& symbols_path, const std::string& module_file_path,
      const std::string& module_build_id);

  ErrorMessageOr<orbit_client_protos::PresetInfo> ReadPresetFromFile(
      const std::filesystem::path& filename);

  ErrorMessageOr<void> SavePreset(const std::string& filename);
  [[nodiscard]] ScopedStatus CreateScopedStatus(const std::string& initial_message);

  void EnableFrameTracksFromHashes(const ModuleData* module,
                                   absl::Span<const uint64_t> function_hashes);
  void AddFrameTrackTimers(uint64_t instrumented_function_id);
  void RefreshFrameTracks();

  void OnCaptureFailed(ErrorMessage error_message);
  void OnCaptureCancelled();
  void OnCaptureComplete();

  void RequestUpdatePrimitives();

  std::atomic<bool> capture_loading_cancellation_requested_ = false;

  CaptureStartedCallback capture_started_callback_;
  CaptureStopRequestedCallback capture_stop_requested_callback_;
  CaptureStoppedCallback capture_stopped_callback_;
  CaptureFailedCallback capture_failed_callback_;
  CaptureClearedCallback capture_cleared_callback_;
  SelectLiveTabCallback select_live_tab_callback_;
  DisassemblyCallback disassembly_callback_;
  ErrorMessageCallback error_message_callback_;
  WarningMessageCallback warning_message_callback_;
  InfoMessageCallback info_message_callback_;
  RefreshCallback refresh_callback_;
  SamplingReportCallback sampling_reports_callback_;
  SamplingReportCallback selection_report_callback_;
  CallTreeViewCallback top_down_view_callback_;
  CallTreeViewCallback selection_top_down_view_callback_;
  CallTreeViewCallback bottom_up_view_callback_;
  CallTreeViewCallback selection_bottom_up_view_callback_;
  SaveFileCallback save_file_callback_;
  ClipboardCallback clipboard_callback_;
  SecureCopyCallback secure_copy_callback_;
  ShowEmptyFrameTrackWarningCallback empty_frame_track_warning_callback_;
  TimerSelectedCallback timer_selected_callback_;

  std::vector<DataView*> panels_;

  std::unique_ptr<ModulesDataView> modules_data_view_;
  std::unique_ptr<FunctionsDataView> functions_data_view_;
  std::unique_ptr<CallStackDataView> callstack_data_view_;
  std::unique_ptr<CallStackDataView> selection_callstack_data_view_;
  std::unique_ptr<PresetsDataView> presets_data_view_;
  std::unique_ptr<TracepointsDataView> tracepoints_data_view_;

  CaptureWindow* capture_window_ = nullptr;
  IntrospectionWindow* introspection_window_ = nullptr;
  GlCanvas* debug_canvas_ = nullptr;

  std::shared_ptr<SamplingReport> sampling_report_;
  std::shared_ptr<SamplingReport> selection_report_ = nullptr;

  absl::flat_hash_map<std::pair<std::string, std::string>,
                      orbit_base::Future<ErrorMessageOr<std::filesystem::path>>>
      modules_currently_loading_;
  absl::flat_hash_map<std::pair<std::string, std::string>, orbit_base::Future<ErrorMessageOr<void>>>
      symbols_currently_loading_;

  StringManager string_manager_;
  std::shared_ptr<grpc::Channel> grpc_channel_;

  orbit_gl::MainWindowInterface* main_window_ = nullptr;
  MainThreadExecutor* main_thread_executor_;
  std::thread::id main_thread_id_;
  std::unique_ptr<ThreadPool> thread_pool_;
  std::unique_ptr<orbit_capture_client::CaptureClient> capture_client_;
  ProcessManager* process_manager_ = nullptr;
  std::unique_ptr<orbit_client_data::ModuleManager> module_manager_;
  std::unique_ptr<DataManager> data_manager_;
  std::unique_ptr<CrashManager> crash_manager_;
  std::unique_ptr<ManualInstrumentationManager> manual_instrumentation_manager_;

  const SymbolHelper symbol_helper_;

  StatusListener* status_listener_ = nullptr;

  ProcessData* process_ = nullptr;

  std::unique_ptr<FramePointerValidatorClient> frame_pointer_validator_client_;

  // TODO(kuebler): This is mostly written during capture by the capture thread on the
  //  CaptureListener parts of App, but may be read also during capturing by all threads.
  //  Currently, it is not properly synchronized (and thus it can't live at DataManager).
  std::unique_ptr<CaptureData> capture_data_;

  orbit_gl::FrameTrackOnlineProcessor frame_track_online_processor_;

  const orbit_base::CrashHandler* crash_handler_;
  orbit_metrics_uploader::MetricsUploader* metrics_uploader_;
};

#endif  // ORBIT_GL_APP_H_
