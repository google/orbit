// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_MAIN_WINDOW_H_
#define ORBIT_QT_ORBIT_MAIN_WINDOW_H_

#include <absl/time/time.h>
#include <absl/types/span.h>
#include <stddef.h>

#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QObject>
#include <QPoint>
#include <QPushButton>
#include <QResizeEvent>
#include <QString>
#include <QStringList>
#include <QTabWidget>
#include <QTimer>
#include <QWidget>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleData.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeStatsCollection.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "CodeReport/CodeReport.h"
#include "CodeReport/DisassemblyReport.h"
#include "DataViews/DataView.h"
#include "DataViews/DataViewType.h"
#include "GrpcProtos/process.pb.h"
#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"
#include "OrbitGl/CallTreeView.h"
#include "OrbitGl/MainWindowInterface.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/SelectionData.h"
#include "OrbitGl/StaticTimeGraphLayout.h"
#include "OrbitQt/FilterPanelWidgetAction.h"
#include "OrbitQt/orbitglwidget.h"
#include "QtUtils/MainThreadExecutor.h"
#include "SessionSetup/TargetConfiguration.h"
#include "SessionSetup/TargetLabel.h"

namespace Ui {
class OrbitMainWindow;
}

class OrbitMainWindow final : public QMainWindow, public orbit_gl::MainWindowInterface {
  Q_OBJECT
  using ScopeId = orbit_client_data::ScopeId;

 public:
  static constexpr int kQuitOrbitReturnCode = 0;
  static constexpr int kEndSessionReturnCode = 1;

  explicit OrbitMainWindow(orbit_session_setup::TargetConfiguration target_configuration,
                           const QStringList& command_line_flags = QStringList());
  ~OrbitMainWindow() override;

  void OnRefreshDataViewPanels(orbit_data_views::DataViewType type);
  void UpdatePanel(orbit_data_views::DataViewType type);

  void SetSamplingReport(
      const orbit_client_data::CallstackData* callstack_data,
      const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data) override;
  void SetSelectionSamplingReport(
      orbit_data_views::DataView* callstack_data_view,
      const orbit_client_data::CallstackData* callstack_data,
      const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data) override;
  void UpdateSamplingReport(
      const orbit_client_data::CallstackData* callstack_data,
      const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data) override;
  void UpdateSelectionReport(
      const orbit_client_data::CallstackData* callstack_data,
      const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data) override;

  void SetTopDownView(std::shared_ptr<const CallTreeView> top_down_view) override;
  void SetSelectionTopDownView(
      std::shared_ptr<const CallTreeView> selection_top_down_view) override;

  void SetBottomUpView(std::shared_ptr<const CallTreeView> bottom_up_view) override;
  void SetSelectionBottomUpView(
      std::shared_ptr<const CallTreeView> selection_bottom_up_view) override;

  void OpenCapture(std::string_view filepath);
  void OnCaptureCleared() override;
  void OnSetClipboard(std::string_view text) override;
  void RefreshDataView(orbit_data_views::DataViewType type) override;
  void SelectLiveTab() override;
  void SelectTopDownTab() override;
  std::string OnGetSaveFileName(std::string_view extension) override;

  void SetErrorMessage(std::string_view title, std::string_view text) override;
  void SetWarningMessage(std::string_view title, std::string_view text) override;

  bool eventFilter(QObject* watched, QEvent* event) override;

  [[nodiscard]] orbit_session_setup::TargetConfiguration ClearTargetConfiguration();

  void ShowTooltip(std::string_view message) override;
  void ShowSourceCode(
      const std::filesystem::path& file_path, size_t line_number,
      std::optional<std::unique_ptr<orbit_code_report::CodeReport>> maybe_code_report) override;
  void ShowDisassembly(const orbit_client_data::FunctionInfo& function_info,
                       std::string_view assembly,
                       orbit_code_report::DisassemblyReport report) override;

  void AppendToCaptureLog(CaptureLogSeverity severity, absl::Duration capture_time,
                          std::string_view message) override;
  orbit_gl::MainWindowInterface::SymbolErrorHandlingResult HandleSymbolError(
      const ErrorMessage& error, const orbit_client_data::ModuleData* module) override;
  void ShowWarningWithDontShowAgainCheckboxIfNeeded(
      std::string_view title, std::string_view text,
      std::string_view dont_show_again_setting_key) override;

  void ShowHistogram(const std::vector<uint64_t>* data, std::string scope_name,
                     std::optional<ScopeId> scope_id) override;

  orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>> DownloadFileFromInstance(
      std::filesystem::path path_on_instance, std::filesystem::path local_path,
      orbit_base::StopToken stop_token) override;

  orbit_base::CanceledOr<void> DisplayStopDownloadDialog(
      const orbit_client_data::ModuleData* module) override;

  void SetSelection(const SelectionData& selection_data) override;

  bool IsConnected() override { return is_connected_; }

  [[nodiscard]] bool IsLocalTarget() const override {
    return std::holds_alternative<orbit_session_setup::LocalTarget>(target_configuration_);
  }

  void SetLiveTabScopeStatsCollection(
      std::shared_ptr<const orbit_client_data::ScopeStatsCollection> scope_collection) override;

 protected:
  void closeEvent(QCloseEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

  // TODO(https://github.com/google/orbit/issues/4589): Connect slots via code and not via UI files,
  // and remove the "public slots" specifier
 public slots:  // NOLINT(readability-redundant-access-specifiers)
  void OnFilterFunctionsTextChanged(const QString& text);
  void OnFilterTracksTextChanged(const QString& text);

  // TODO(https://github.com/google/orbit/issues/4589): Connect slots via code and not via UI files,
  // and remove the "public slots" specifier
 private slots:  // NOLINT(readability-redundant-access-specifiers)
  void on_actionOpenUserDataDirectory_triggered();
  void on_actionOpenAppDataDirectory_triggered();
  void on_actionAbout_triggered();

  void on_actionReport_Missing_Feature_triggered();
  void on_actionReport_Bug_triggered();

  void OnTimer();
  void OnLiveTabFunctionsFilterTextChanged(const QString& text);

  void on_actionOpen_Preset_triggered();
  void on_actionSave_Preset_As_triggered();

  void on_actionEnd_Session_triggered();
  void on_actionQuit_triggered();

  void on_actionToggle_Capture_triggered();
  void on_actionOpen_Capture_triggered();
  void on_actionRename_Capture_File_triggered();
  void on_actionCaptureOptions_triggered();
  void on_actionHelp_toggled(bool checked);
  void on_actionIntrospection_triggered();

  static void on_actionCheckFalse_triggered();
  static void on_actionStackOverflow_triggered();
  void on_actionServiceCheckFalse_triggered();
  void on_actionServiceStackOverflow_triggered();

  void on_actionSourcePathMappings_triggered();

  void on_actionSymbolLocationsDialog_triggered();

  void OnTimerSelectionChanged(const orbit_client_protos::TimerInfo* timer_info) override;

  // TODO(https://github.com/google/orbit/issues/4589): Remove redundant "private" once slots is not
  // needed anymore above.
 private:  // NOLINT(readability-redundant-access-specifiers)
  void UpdateFilePath(const std::filesystem::path& file_path);
  void StartMainTimer();
  void SetupCaptureToolbar();
  void SetupMainWindow();
  void SetupHintFrame();
  void SetupTargetLabel();
  void SetupStatusBarLogButton();
  void SetupTrackConfigurationUi();

  void SetupAccessibleNamesForAutomation();

  void SaveCurrentTabLayoutAsDefaultInMemory();

  void SaveMainWindowGeometry();
  void RestoreMainWindowGeometry();

  void CreateTabBarContextMenu(QTabWidget* tab_widget, int tab_index, const QPoint& pos);
  void UpdateCaptureStateDependentWidgets();
  void UpdateProcessConnectionStateDependentWidgets();
  void ClearCaptureFilters();

  void UpdateActiveTabsAfterSelection(bool selection_has_samples);

  QTabWidget* FindParentTabWidget(const QWidget* widget) const;

  void SetTarget(const orbit_session_setup::SshTarget& target);
  void SetTarget(const orbit_session_setup::LocalTarget& target);
  void SetTarget(const orbit_session_setup::FileTarget& target);

  void UpdateTargetLabelPosition();

  void OnProcessListUpdated(absl::Span<const orbit_grpc_protos::ProcessInfo> processes);

  void ExecuteSymbolLocationsDialog(std::optional<const orbit_client_data::ModuleData*> module);

  static const QString kEnableCallstackSamplingSettingKey;
  static const QString kCallstackSamplingPeriodMsSettingKey;
  static const QString kCallstackUnwindingMethodSettingKey;
  static const QString kMaxCopyRawStackSizeSettingKey;
  static const QString kCollectSchedulerInfoSettingKey;
  static const QString kCollectThreadStatesSettingKey;
  static const QString kTraceGpuSubmissionsSettingKey;
  static const QString kEnableAutoFrameTrack;
  static const QString kCollectMemoryInfoSettingKey;
  static const QString kEnableApiSettingKey;
  static const QString kEnableIntrospectionSettingKey;
  static const QString kDynamicInstrumentationMethodSettingKey;
  static const QString kMemorySamplingPeriodMsSettingKey;
  static const QString kMemoryWarningThresholdKbSettingKey;
  static const QString kLimitLocalMarkerDepthPerCommandBufferSettingsKey;
  static const QString kEnableCallStackCollectionOnThreadStateChanges;
  static const QString kThreadStateChangeCallstackMaxCopyRawStackSizeSettingKey;
  static const QString kMaxLocalMarkerDepthPerCommandBufferSettingsKey;
  static const QString kMainWindowGeometrySettingKey;
  static const QString kMainWindowStateSettingKey;
  static const QString kWineSyscallHandlingMethodSettingKey;

  void LoadCaptureOptionsIntoApp();

  [[nodiscard]] bool ConfirmExit();
  void Exit(int return_code);

  void OnConnectionError(const QString& error_message);
  void OnSshConnectionError(std::error_code error);
  void OnLocalConnectionError(const QString& error_message);

  void UpdateCaptureToolbarIconOpacity();

  std::optional<QString> LoadSourceCode(const std::filesystem::path& file_path);

  std::unique_ptr<OrbitApp> app_;
  Ui::OrbitMainWindow* ui;
  FilterPanelWidgetAction* filter_panel_action_ = nullptr;
  QTimer* main_timer_ = nullptr;
  std::unique_ptr<OrbitGLWidget> introspection_widget_ = nullptr;
  QFrame* hint_frame_ = nullptr;
  orbit_session_setup::TargetLabel* target_label_ = nullptr;
  QWidget* target_widget_ = nullptr;
  QPushButton* capture_log_button_ = nullptr;

  QStringList command_line_flags_;

  // Capture toolbar.
  QIcon icon_start_capture_;
  QIcon icon_stop_capture_;
  QIcon icon_toolbar_extension_;

  QIcon icon_keyboard_arrow_left_;
  QIcon icon_keyboard_arrow_right_;

  struct TabWidgetLayout {
    std::vector<std::pair<QWidget*, QString>> tabs_and_titles;
    int current_index;
  };
  std::map<QTabWidget*, TabWidgetLayout> default_tab_layout_;

  orbit_session_setup::TargetConfiguration target_configuration_;

  enum class TargetProcessState { kRunning, kEnded };

  TargetProcessState target_process_state_ = TargetProcessState::kEnded;

  // This value indicates whether Orbit (Ui / OrbitQt) is connected to an OrbitService. This can
  // currently be connection to a Stadia instance (ssh tunnel via ServiceDeployManager) or a
  // connection to an OrbitService running on the local machine. If Orbit displays a capture saved
  // to a file, it is not connected and this bool is false. This is also false when the connection
  // broke.
  bool is_connected_ = false;

  // Keep this at the bottom of the member list, so that it's destroyed first!
  orbit_qt_utils::MainThreadExecutor main_thread_executor_;
};

#endif  // ORBIT_QT_ORBIT_MAIN_WINDOW_H_
