// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_APP_H_
#define ORBIT_GL_APP_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/time/time.h>
#include <absl/types/span.h>
#include <grpc/impl/codegen/connectivity_state.h>
#include <grpcpp/channel.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "CaptureClient/AbstractCaptureListener.h"
#include "CaptureClient/AppInterface.h"
#include "CaptureClient/CaptureClient.h"
#include "CaptureClient/CaptureListener.h"
#include "CaptureFileInfo/Manager.h"
#include "ClientData/ApiStringEvent.h"
#include "ClientData/ApiTrackValue.h"
#include "ClientData/CallstackData.h"
#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackType.h"
#include "ClientData/CaptureData.h"
#include "ClientData/CgroupAndProcessMemoryInfo.h"
#include "ClientData/DataManager.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleIdentifier.h"
#include "ClientData/ModuleIdentifierProvider.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ModulePathAndBuildId.h"
#include "ClientData/PageFaultsInfo.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientData/ProcessData.h"
#include "ClientData/ScopeId.h"
#include "ClientData/SystemMemoryInfo.h"
#include "ClientData/ThreadStateSliceInfo.h"
#include "ClientData/TimerChain.h"
#include "ClientData/WineSyscallHandlingMethod.h"
#include "ClientProtos/capture_data.pb.h"
#include "ClientProtos/preset.pb.h"
#include "ClientServices/CrashManager.h"
#include "ClientServices/ProcessManager.h"
#include "CodeReport/DisassemblyReport.h"
#include "DataViews/AppInterface.h"
#include "DataViews/CallstackDataView.h"
#include "DataViews/DataView.h"
#include "DataViews/DataViewType.h"
#include "DataViews/FunctionsDataView.h"
#include "DataViews/ModulesDataView.h"
#include "DataViews/PresetLoadState.h"
#include "DataViews/PresetsDataView.h"
#include "DataViews/SymbolLoadingState.h"
#include "DataViews/TracepointsDataView.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/process.pb.h"
#include "GrpcProtos/services.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Executor.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitGl/CaptureWindow.h"
#include "OrbitGl/DataViewFactory.h"
#include "OrbitGl/FrameTrackOnlineProcessor.h"
#include "OrbitGl/IntrospectionWindow.h"
#include "OrbitGl/MainWindowInterface.h"
#include "OrbitGl/ManualInstrumentationManager.h"
#include "OrbitGl/SelectionData.h"
#include "OrbitGl/SymbolLoader.h"
#include "OrbitGl/TimeGraph.h"
#include "PresetFile/PresetFile.h"
#include "QtUtils/Throttle.h"
#include "Statistics/BinomialConfidenceInterval.h"
#include "Statistics/Histogram.h"
#include "StringManager/StringManager.h"

class OrbitApp final : public DataViewFactory,
                       public orbit_capture_client::AbstractCaptureListener<OrbitApp>,
                       public orbit_data_views::AppInterface,
                       public orbit_capture_client::CaptureControlInterface,
                       public orbit_gl::SymbolLoader::AppInterface {
  using ScopeId = orbit_client_data::ScopeId;

 public:
  explicit OrbitApp(orbit_gl::MainWindowInterface* main_window,
                    orbit_base::Executor* main_thread_executor);
  ~OrbitApp() override;

  static std::unique_ptr<OrbitApp> Create(orbit_gl::MainWindowInterface* main_window,
                                          orbit_base::Executor* main_thread_executor);

  void PostInit(bool is_connected);
  void MainTick();

  [[nodiscard]] absl::Duration GetCaptureTime() const;
  [[nodiscard]] absl::Duration GetCaptureTimeAt(uint64_t timestamp_ns) const;

  [[nodiscard]] std::string GetSaveFile(std::string_view extension) const override;
  void SetClipboard(std::string_view text) override;

  ErrorMessageOr<void> OnSavePreset(std::string_view file_name);
  ErrorMessageOr<void> OnLoadPreset(std::string_view file_name);
  orbit_base::Future<ErrorMessageOr<CaptureOutcome>> LoadCaptureFromFile(
      const std::filesystem::path& file_path);

  orbit_base::Future<ErrorMessageOr<void>> MoveCaptureFile(const std::filesystem::path& src,
                                                           const std::filesystem::path& dest);
  void OnLoadCaptureCancelRequested();

  // --------- orbit_capture_client::CaptureControlInterface  ----------
  [[nodiscard]] orbit_capture_client::CaptureClient::State GetCaptureState() const override;
  [[nodiscard]] bool IsCapturing() const override;

  void StartCapture() override;
  void StopCapture() override;
  void AbortCapture() override;
  void ToggleCapture() override;
  // ---------------------------------------------------------

  void ClearCapture();

  [[nodiscard]] bool IsLoadingCapture() const;

  [[nodiscard]] const orbit_client_data::ModuleManager* GetModuleManager() const override {
    ORBIT_CHECK(module_manager_ != nullptr);
    return module_manager_.get();
  }

  [[nodiscard]] orbit_client_data::ModuleManager* GetMutableModuleManager() override {
    ORBIT_CHECK(module_manager_ != nullptr);
    return module_manager_.get();
  }

  void ListPresets();
  void RefreshCaptureView();
  void Disassemble(uint32_t pid, const orbit_client_data::FunctionInfo& function) override;
  void ShowSourceCode(const orbit_client_data::FunctionInfo& function) override;

  void OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& capture_started,
                        std::optional<std::filesystem::path> file_path,
                        absl::flat_hash_set<uint64_t> frame_track_function_ids) override;
  void OnCaptureFinished(const orbit_grpc_protos::CaptureFinished& capture_finished) override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  void OnCgroupAndProcessMemoryInfo(
      const orbit_client_data::CgroupAndProcessMemoryInfo& cgroup_and_process_memory_info) override;
  void OnPageFaultsInfo(const orbit_client_data::PageFaultsInfo& page_faults_info) override;
  void OnSystemMemoryInfo(const orbit_client_data::SystemMemoryInfo& system_memory_info) override;
  void OnKeyAndString(uint64_t key, std::string str) override;

  void OnModuleUpdate(uint64_t timestamp_ns, orbit_grpc_protos::ModuleInfo module_info) override;
  void OnModulesSnapshot(uint64_t timestamp_ns,
                         std::vector<orbit_grpc_protos::ModuleInfo> module_infos) override;
  void OnPresentEvent(const orbit_grpc_protos::PresentEvent& present_event) override;
  void OnApiStringEvent(const orbit_client_data::ApiStringEvent& api_string_event) override;
  void OnApiTrackValue(const orbit_client_data::ApiTrackValue& api_track_value) override;
  void OnWarningEvent(orbit_grpc_protos::WarningEvent warning_event) override;
  void OnClockResolutionEvent(
      orbit_grpc_protos::ClockResolutionEvent clock_resolution_event) override;
  void OnErrorsWithPerfEventOpenEvent(
      orbit_grpc_protos::ErrorsWithPerfEventOpenEvent errors_with_perf_event_open_event) override;
  void OnWarningInstrumentingWithUprobesEvent(
      orbit_grpc_protos::WarningInstrumentingWithUprobesEvent
          warning_instrumenting_with_uprobes_event) override;
  void OnErrorEnablingOrbitApiEvent(
      orbit_grpc_protos::ErrorEnablingOrbitApiEvent error_enabling_orbit_api_event) override;
  void OnErrorEnablingUserSpaceInstrumentationEvent(
      orbit_grpc_protos::ErrorEnablingUserSpaceInstrumentationEvent error_event) override;
  void OnWarningInstrumentingWithUserSpaceInstrumentationEvent(
      orbit_grpc_protos::WarningInstrumentingWithUserSpaceInstrumentationEvent warning_event)
      override;
  void OnLostPerfRecordsEvent(
      orbit_grpc_protos::LostPerfRecordsEvent lost_perf_records_event) override;
  void OnOutOfOrderEventsDiscardedEvent(orbit_grpc_protos::OutOfOrderEventsDiscardedEvent
                                            out_of_order_events_discarded_event) override;

  void SetCaptureWindow(CaptureWindow* capture);
  [[nodiscard]] const TimeGraph* GetTimeGraph() const {
    ORBIT_CHECK(capture_window_ != nullptr);
    return capture_window_->GetTimeGraph();
  }
  [[nodiscard]] TimeGraph* GetMutableTimeGraph() {
    ORBIT_CHECK(capture_window_ != nullptr);
    return capture_window_->GetTimeGraph();
  }
  void SetIntrospectionWindow(IntrospectionWindow* introspection_window);
  void StopIntrospection();

  void SetSamplingReport(
      const orbit_client_data::CallstackData* callstack_data,
      const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data);
  void ClearSamplingReport();
  void SetSelectionReport(
      const orbit_client_data::CallstackData* selection_callstack_data,
      const orbit_client_data::PostProcessedSamplingData* selection_post_processed_sampling_data);
  void ClearSelectionReport();
  void SetTopDownView(const orbit_client_data::PostProcessedSamplingData& post_processed_data);
  void ClearTopDownView();
  void SetSelectionTopDownView(
      const orbit_client_data::PostProcessedSamplingData& selection_post_processed_data,
      const orbit_client_data::CaptureData* capture_data);
  void ClearSelectionTopDownView();

  void SetBottomUpView(const orbit_client_data::PostProcessedSamplingData& post_processed_data);
  void ClearBottomUpView();
  void SetSelectionBottomUpView(
      const orbit_client_data::PostProcessedSamplingData& selection_post_processed_data,
      const orbit_client_data::CaptureData* capture_data);
  void ClearSelectionBottomUpView();

  // This needs to be called from the main thread.
  [[nodiscard]] bool IsCaptureConnected(
      const orbit_client_data::CaptureData& capture) const override;

  [[nodiscard]] static bool IsDevMode();

  // Callbacks
  using CaptureStartedCallback = std::function<void(const std::optional<std::filesystem::path>&)>;
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

  void FireRefreshCallbacks(
      orbit_data_views::DataViewType type = orbit_data_views::DataViewType::kAll);
  void OnModuleListUpdated() override {
    FireRefreshCallbacks(orbit_data_views::DataViewType::kModules);
  }
  void SendDisassemblyToUi(const orbit_client_data::FunctionInfo& function_info,
                           std::string disassembly, orbit_code_report::DisassemblyReport report);
  void SendTooltipToUi(std::string tooltip);
  void SendWarningToUi(std::string title, std::string text);
  void SendErrorToUi(std::string title, std::string text) override;

  orbit_base::Future<void> LoadSymbolsManually(
      absl::Span<const orbit_client_data::ModuleData* const> modules) override;

  orbit_base::Future<ErrorMessageOr<std::filesystem::path>> RetrieveModuleWithDebugInfo(
      const orbit_client_data::ModulePathAndBuildId& module_path_and_build_id);

  orbit_base::Future<ErrorMessageOr<void>> UpdateProcessAndModuleList() override;
  orbit_base::Future<std::vector<ErrorMessageOr<void>>> ReloadModules(
      absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos);
  void RefreshUIAfterModuleReload();

  void UpdateAfterSymbolLoading();
  void UpdateAfterSymbolLoadingThrottled();
  void ClearSamplingRelatedViews();

  // Load the functions and add frame tracks from a particular module of a preset file.
  orbit_base::Future<ErrorMessageOr<void>> LoadPresetModule(
      const std::filesystem::path& module_path, const orbit_preset_file::PresetFile& preset_file);
  orbit_base::Future<ErrorMessageOr<void>> LoadPreset(
      const orbit_preset_file::PresetFile& preset) override;
  [[nodiscard]] orbit_data_views::PresetLoadState GetPresetLoadState(
      const orbit_preset_file::PresetFile& preset) const override;
  void ShowPresetInExplorer(const orbit_preset_file::PresetFile& preset) override;
  void FilterTracks(std::string_view filter);

  void CrashOrbitService(orbit_grpc_protos::CrashOrbitServiceRequest_CrashType crash_type);

  void SetGrpcChannel(std::shared_ptr<grpc::Channel> grpc_channel) {
    ORBIT_CHECK(grpc_channel_ == nullptr);
    ORBIT_CHECK(grpc_channel != nullptr);
    grpc_channel_ = std::move(grpc_channel);
  }
  void SetProcessManager(orbit_client_services::ProcessManager* process_manager) {
    ORBIT_CHECK(process_manager_ == nullptr);
    ORBIT_CHECK(process_manager != nullptr);
    process_manager_ = process_manager;
  }
  void SetTargetProcess(orbit_grpc_protos::ProcessInfo process);
  [[nodiscard]] orbit_data_views::DataView* GetOrCreateDataView(
      orbit_data_views::DataViewType type) override;
  [[nodiscard]] orbit_data_views::DataView* GetOrCreateSelectionCallstackDataView();

  [[nodiscard]] orbit_string_manager::StringManager* GetStringManager() { return &string_manager_; }
  [[nodiscard]] orbit_client_data::ProcessData* GetMutableTargetProcess() const {
    return process_.get();
  }
  [[nodiscard]] const orbit_client_data::ProcessData* GetTargetProcess() const override {
    return process_.get();
  }
  [[nodiscard]] const orbit_client_data::ModuleIdentifierProvider* GetModuleIdentifierProvider()
      const {
    return &module_identifier_provider_;
  }
  [[nodiscard]] ManualInstrumentationManager* GetManualInstrumentationManager() {
    return manual_instrumentation_manager_.get();
  }
  [[nodiscard]] orbit_client_data::ModuleData* GetMutableModuleByModuleIdentifier(
      orbit_client_data::ModuleIdentifier module_id) override {
    return module_manager_->GetMutableModuleByModuleIdentifier(module_id);
  }
  [[nodiscard]] const orbit_client_data::ModuleData* GetModuleByModuleIdentifier(
      orbit_client_data::ModuleIdentifier module_id) const override {
    return module_manager_->GetModuleByModuleIdentifier(module_id);
  }
  [[nodiscard]] orbit_client_data::ModuleData* GetMutableModuleByModulePathAndBuildId(
      const orbit_client_data::ModulePathAndBuildId& module_path_and_build_id) override {
    return module_manager_->GetMutableModuleByModulePathAndBuildId(module_path_and_build_id);
  }
  [[nodiscard]] const orbit_client_data::ModuleData* GetModuleByModulePathAndBuildId(
      const orbit_client_data::ModulePathAndBuildId& module_path_and_build_id) const override {
    return module_manager_->GetModuleByModulePathAndBuildId(module_path_and_build_id);
  }
  [[nodiscard]] const orbit_client_data::ProcessData& GetConnectedOrLoadedProcess() const;

  void SetCollectSchedulerInfo(bool collect_scheduler_info);
  void SetCollectThreadStates(bool collect_thread_states);
  void SetTraceGpuSubmissions(bool trace_gpu_submissions);
  void SetEnableApi(bool enable_api);
  void SetEnableIntrospection(bool enable_introspection);
  void SetDynamicInstrumentationMethod(
      orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod method);
  void SetWineSyscallHandlingMethod(orbit_client_data::WineSyscallHandlingMethod method);
  void SetSamplesPerSecond(double samples_per_second);
  void SetStackDumpSize(uint16_t stack_dump_size);
  void SetThreadStateChangeCallstackStackDumpSize(uint16_t stack_dump_size);
  void SetUnwindingMethod(orbit_grpc_protos::CaptureOptions::UnwindingMethod unwinding_method);
  void SetMaxLocalMarkerDepthPerCommandBuffer(uint64_t max_local_marker_depth_per_command_buffer);
  void SetEnableAutoFrameTrack(bool enable_auto_frame_track);
  void SetCollectMemoryInfo(bool collect_memory_info) {
    data_manager_->set_collect_memory_info(collect_memory_info);
  }
  void SetMemorySamplingPeriodMs(uint64_t memory_sampling_period_ms) {
    data_manager_->set_memory_sampling_period_ms(memory_sampling_period_ms);
  }
  [[nodiscard]] uint64_t GetMemorySamplingPeriodMs() const {
    return GetCaptureData().GetMemorySamplingPeriodNs() / 1'000'000;
  }
  void SetMemoryWarningThresholdKb(uint64_t memory_warning_threshold_kb) {
    data_manager_->set_memory_warning_threshold_kb(memory_warning_threshold_kb);
  }
  [[nodiscard]] uint64_t GetMemoryWarningThresholdKb() const {
    return GetCaptureData().memory_warning_threshold_kb();
  }
  void SetThreadStateChangeCallstackCollection(
      orbit_grpc_protos::CaptureOptions::ThreadStateChangeCallStackCollection
          thread_state_change_callstack_collection) {
    data_manager_->set_thread_state_change_callstack_collection(
        thread_state_change_callstack_collection);
  }

  // TODO(kuebler): Move them to a separate controller at some point
  void SelectFunction(const orbit_client_data::FunctionInfo& func) override;
  void DeselectFunction(const orbit_client_data::FunctionInfo& func) override;
  [[nodiscard]] bool IsFunctionSelected(const orbit_client_data::FunctionInfo& func) const override;
  [[nodiscard]] bool IsFunctionSelected(
      const orbit_client_data::SampledFunction& func) const override;
  [[nodiscard]] bool IsFunctionSelected(uint64_t absolute_address) const;

  void SetVisibleScopeIds(absl::flat_hash_set<ScopeId> visible_scope_ids) override;
  [[nodiscard]] bool IsTimerActive(const orbit_client_protos::TimerInfo& timer) const;
  // Returns the time range for which thread_id is active. If the entire thread is inactive, it will
  // return nullopt.
  std::optional<orbit_client_data::TimeRange> GetActiveTimeRangeForTid(
      orbit_client_data::ThreadID thread_id) const;

  [[nodiscard]] std::optional<ScopeId> GetHighlightedScopeId() const override;
  void SetHighlightedScopeId(std::optional<ScopeId> highlighted_scope_id) override;

  [[nodiscard]] orbit_client_data::ThreadID selected_thread_id() const;
  void set_selected_thread_id(orbit_client_data::ThreadID thread_id);

  [[nodiscard]] std::optional<orbit_client_data::ThreadStateSliceInfo> selected_thread_state_slice()
      const;
  void set_selected_thread_state_slice(
      std::optional<orbit_client_data::ThreadStateSliceInfo> thread_state_slice);

  [[nodiscard]] std::optional<orbit_client_data::ThreadStateSliceInfo> hovered_thread_state_slice()
      const;
  void set_hovered_thread_state_slice(
      std::optional<orbit_client_data::ThreadStateSliceInfo> thread_state_slice);

  [[nodiscard]] const orbit_client_protos::TimerInfo* selected_timer() const;
  void SelectTimer(const orbit_client_protos::TimerInfo* timer_info);
  void DeselectTimer() override;

  [[nodiscard]] std::optional<ScopeId> GetScopeIdToHighlight() const;
  [[nodiscard]] uint64_t GetGroupIdToHighlight() const;

  // origin_is_multiple_threads defines if the selection is specific to a single thread,
  // or spans across multiple threads.
  void SelectCallstackEvents(
      absl::Span<const orbit_client_data::CallstackEvent> selected_callstack_events);
  void InspectCallstackEvents(
      absl::Span<const orbit_client_data::CallstackEvent> selected_callstack_events);
  void ClearInspection();
  void ClearSelectionTabs();

  const orbit_client_data::CallstackData& GetSelectedCallstackData() const;

  void SelectTracepoint(const orbit_grpc_protos::TracepointInfo& tracepoint) override;
  void DeselectTracepoint(const orbit_grpc_protos::TracepointInfo& tracepoint) override;

  [[nodiscard]] bool IsTracepointSelected(
      const orbit_grpc_protos::TracepointInfo& info) const override;

  // Only enables the frame track in the capture settings (in DataManager) and does not
  // add a frame track to the current capture data.
  void EnableFrameTrack(const orbit_client_data::FunctionInfo& function) override;

  // Only disables the frame track in the capture settings (in DataManager) and does
  // not remove the frame track from the capture data.
  void DisableFrameTrack(const orbit_client_data::FunctionInfo& function) override;

  [[nodiscard]] bool IsFrameTrackEnabled(
      const orbit_client_data::FunctionInfo& function) const override;

  // Adds a frame track to the current capture data if the captures contains function calls to
  // the function and the function was instrumented.
  void AddFrameTrack(const orbit_client_data::FunctionInfo& function) override;

  // Removes the frame track (if it exists) from the capture data.
  void RemoveFrameTrack(const orbit_client_data::FunctionInfo& function) override;

  [[nodiscard]] bool HasFrameTrackInCaptureData(uint64_t instrumented_function_id) const override;

  void JumpToTimerAndZoom(ScopeId scope_id, JumpToTimerMode selection_mode) override;
  [[nodiscard]] std::vector<const orbit_client_data::TimerChain*> GetAllThreadTimerChains()
      const override;

  [[nodiscard]] const orbit_statistics::BinomialConfidenceIntervalEstimator&
  GetConfidenceIntervalEstimator() const override;

  void SetHistogramSelectionRange(std::optional<orbit_statistics::HistogramSelectionRange> range) {
    histogram_selection_range_ = range;
    RequestUpdatePrimitives();
  }

  [[nodiscard]] std::optional<orbit_statistics::HistogramSelectionRange>
  GetHistogramSelectionRange() const {
    return histogram_selection_range_;
  }

  [[nodiscard]] bool IsConnected() const override { return main_window_->IsConnected(); }
  [[nodiscard]] bool IsLocalTarget() const override { return main_window_->IsLocalTarget(); }

  orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>> DownloadFileFromInstance(
      std::filesystem::path path_on_instance, std::filesystem::path local_path,
      orbit_base::StopToken stop_token) override;
  void AddSymbols(const orbit_client_data::ModulePathAndBuildId& module_path_and_build_id,
                  const orbit_grpc_protos::ModuleSymbols& module_symbols) override;
  void AddFallbackSymbols(const orbit_client_data::ModulePathAndBuildId& module_path_and_build_id,
                          const orbit_grpc_protos::ModuleSymbols& fallback_symbols) override;

  [[nodiscard]] bool IsModuleDownloading(
      const orbit_client_data::ModuleData* module) const override;
  [[nodiscard]] orbit_data_views::SymbolLoadingState GetSymbolLoadingStateForModule(
      const orbit_client_data::ModuleData* module) const override;
  [[nodiscard]] bool IsSymbolLoadingInProgressForModule(
      const orbit_client_data::ModuleData* module) const override;
  void RequestSymbolDownloadStop(
      absl::Span<const orbit_client_data::ModuleData* const> modules) override;
  void DisableDownloadForModule(std::string_view module_file_path);

  // Triggers symbol loading for all modules in ModuleManager that are not loaded yet. This is done
  // with a simple prioritization. The module `ggpvlk.so` is queued to be loaded first, the "main
  // module" (binary of the process) is queued to be loaded second. All other modules are queued in
  // no particular order.
  orbit_base::Future<std::vector<ErrorMessageOr<orbit_base::CanceledOr<void>>>> LoadAllSymbols();

  // Automatically add a default Frame Track. It will choose only one frame track from an internal
  // list of auto-loadable presets.
  void AddDefaultFrameTrackOrLogError();

  void OnTimeRangeSelection(orbit_client_data::TimeRange time_range);
  void ClearTimeRangeSelection();
  void OnThreadOrTimeRangeSelectionChange();
  void ClearThreadAndTimeRangeSelection();

 private:
  void UpdateModulesAbortCaptureIfModuleWithoutBuildIdNeedsReload(
      absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos);
  [[nodiscard]] ErrorMessageOr<std::vector<const orbit_client_data::ModuleData*>>
  GetLoadedModulesByPath(const std::filesystem::path& module_path) const;

  void SelectFunctionsFromHashes(const orbit_client_data::ModuleData* module,
                                 absl::Span<const uint64_t> function_hashes);
  void SelectFunctionsByName(const orbit_client_data::ModuleData* module,
                             absl::Span<const std::string> function_names);

  enum class SymbolLoadingAndErrorHandlingResult {
    kSymbolsLoadedSuccessfully,
    kCanceled,
  };
  // RetrieveModuleAndLoadSymbolsAndHandleError attempts to retrieve the module and loads the
  // symbols via RetrieveModuleAndLoadSymbols, and when that fails it handles the error with
  // symbol_loading_error_callback_. This might result in another loading attempt (another call to
  // RetrieveModuleAndLoadSymbolsAndHandleError).
  orbit_base::Future<SymbolLoadingAndErrorHandlingResult>
  RetrieveModuleAndLoadSymbolsAndHandleError(const orbit_client_data::ModuleData* module);

  void RequestSymbolDownloadStop(absl::Span<const orbit_client_data::ModuleData* const> modules,
                                 bool show_dialog);

  static ErrorMessageOr<orbit_preset_file::PresetFile> ReadPresetFromFile(
      const std::filesystem::path& filename);
  ErrorMessageOr<void> ConvertPresetToNewFormatIfNecessary(
      const orbit_preset_file::PresetFile& preset_file);
  ErrorMessageOr<void> SavePreset(std::string_view filename);

  void AddFrameTrack(uint64_t instrumented_function_id);
  void RemoveFrameTrack(uint64_t instrumented_function_id);
  void EnableFrameTracksFromHashes(const orbit_client_data::ModuleData* module,
                                   absl::Span<const uint64_t> function_hashes);
  void EnableFrameTracksByName(const orbit_client_data::ModuleData* module,
                               absl::Span<const std::string> function_names);
  void AddFrameTrackTimers(uint64_t instrumented_function_id);
  void RefreshFrameTracks();
  void TrySaveUserDefinedCaptureInfo();

  orbit_base::Future<void> OnCaptureFailed(ErrorMessage error_message);
  orbit_base::Future<void> OnCaptureCancelled();
  orbit_base::Future<void> OnCaptureComplete();

  void RequestUpdatePrimitives();

  void ShowHistogram(const std::vector<uint64_t>* data, std::string scope_name,
                     std::optional<ScopeId> scope_id) override;

  // Sets CaptureData's selection_callstack_data and selection_post_processed_sampling_data.
  void SetCaptureDataSelectionFields(
      absl::Span<const orbit_client_data::CallstackEvent> selected_callstack_events);

  std::atomic<bool> capture_loading_cancellation_requested_ = false;
  std::atomic<orbit_client_data::CaptureData::DataSource> data_source_{
      orbit_client_data::CaptureData::DataSource::kLiveCapture};

  CaptureStartedCallback capture_started_callback_;
  CaptureStopRequestedCallback capture_stop_requested_callback_;
  CaptureStoppedCallback capture_stopped_callback_;
  CaptureFailedCallback capture_failed_callback_;

  std::vector<orbit_data_views::DataView*> panels_;

  std::unique_ptr<orbit_data_views::ModulesDataView> modules_data_view_;
  std::unique_ptr<orbit_data_views::FunctionsDataView> functions_data_view_;
  std::unique_ptr<orbit_data_views::CallstackDataView> callstack_data_view_;
  std::unique_ptr<orbit_data_views::CallstackDataView> selection_callstack_data_view_;
  std::unique_ptr<orbit_data_views::PresetsDataView> presets_data_view_;
  std::unique_ptr<orbit_data_views::TracepointsDataView> tracepoints_data_view_;

  CaptureWindow* capture_window_ = nullptr;
  IntrospectionWindow* introspection_window_ = nullptr;

  // A boolean information about if the default Frame Track was added in the current session.
  bool default_frame_track_was_added_ = false;

  orbit_string_manager::StringManager string_manager_;
  std::shared_ptr<grpc::Channel> grpc_channel_;

  orbit_gl::MainWindowInterface* main_window_ = nullptr;
  orbit_base::Executor* main_thread_executor_;
  std::thread::id main_thread_id_;
  std::shared_ptr<orbit_base::ThreadPool> thread_pool_;
  std::unique_ptr<orbit_capture_client::CaptureClient> capture_client_;
  orbit_client_services::ProcessManager* process_manager_ = nullptr;
  std::unique_ptr<orbit_client_data::ModuleManager> module_manager_;
  orbit_client_data::ModuleIdentifierProvider module_identifier_provider_{};
  std::unique_ptr<orbit_client_data::DataManager> data_manager_;
  std::unique_ptr<orbit_client_services::CrashManager> crash_manager_;
  std::unique_ptr<ManualInstrumentationManager> manual_instrumentation_manager_;

  std::unique_ptr<orbit_client_data::ProcessData> process_ = nullptr;

  orbit_gl::FrameTrackOnlineProcessor frame_track_online_processor_;

  orbit_capture_file_info::Manager capture_file_info_manager_{};

  const orbit_statistics::WilsonBinomialConfidenceIntervalEstimator confidence_interval_estimator_;

  std::optional<orbit_statistics::HistogramSelectionRange> histogram_selection_range_;

  std::optional<orbit_gl::SymbolLoader> symbol_loader_;
  static constexpr std::chrono::milliseconds kMaxPostProcessingInterval{1000};
  orbit_qt_utils::Throttle update_after_symbol_loading_throttle_{kMaxPostProcessingInterval};

  std::unique_ptr<SelectionData> full_capture_selection_;
  std::unique_ptr<SelectionData> time_range_thread_selection_;
  std::unique_ptr<SelectionData> inspection_selection_;
};

#endif  // ORBIT_GL_APP_H_
