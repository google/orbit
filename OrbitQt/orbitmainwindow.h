// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_MAIN_WINDOW_H_
#define ORBIT_QT_ORBIT_MAIN_WINDOW_H_

#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QObject>
#include <QPoint>
#include <QString>
#include <QTabWidget>
#include <QTimer>
#include <QWidget>
#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "App.h"
#include "CallTreeView.h"
#include "DataView.h"
#include "DataViewTypes.h"
#include "DisassemblyReport.h"
#include "MainThreadExecutor.h"
#include "OrbitClientServices/ProcessManager.h"
#include "StatusListener.h"
#include "TargetConfiguration.h"
#include "capture_data.pb.h"
#include "orbitglwidget.h"
#include "servicedeploymanager.h"
#include "ui_orbitmainwindow.h"

namespace Ui {
class OrbitMainWindow;
}

class OrbitMainWindow : public QMainWindow {
  Q_OBJECT

 public:
  static constexpr int kEndSessionReturnCode = 1;

  // TODO (170468590) remove when not needed anymore
  explicit OrbitMainWindow(orbit_qt::ServiceDeployManager* service_deploy_manager,
                           std::string grpc_server_address, uint32_t font_size);

  explicit OrbitMainWindow(orbit_qt::TargetConfiguration target_configuration, uint32_t font_size);
  ~OrbitMainWindow() override;

  void RegisterGlWidget(OrbitGLWidget* widget) { gl_widgets_.push_back(widget); }
  void UnregisterGlWidget(OrbitGLWidget* widget) {
    const auto it = std::find(gl_widgets_.begin(), gl_widgets_.end(), widget);
    if (it != gl_widgets_.end()) {
      gl_widgets_.erase(it);
    }
  }
  void OnRefreshDataViewPanels(DataViewType a_Type);
  void UpdatePanel(DataViewType a_Type);

  void OnNewSamplingReport(DataView* callstack_data_view,
                           std::shared_ptr<class SamplingReport> sampling_report);
  void OnNewSelectionReport(DataView* callstack_data_view,
                            std::shared_ptr<class SamplingReport> sampling_report);

  void OnNewTopDownView(std::unique_ptr<CallTreeView> top_down_view);
  void OnNewSelectionTopDownView(std::unique_ptr<CallTreeView> selection_top_down_view);

  void OnNewBottomUpView(std::unique_ptr<CallTreeView> bottom_up_view);
  void OnNewSelectionBottomUpView(std::unique_ptr<CallTreeView> selection_bottom_up_view);

  std::string OnGetSaveFileName(const std::string& extension);
  void OnSetClipboard(const std::string& text);
  void OpenDisassembly(std::string a_String, DisassemblyReport report);
  void OpenCapture(const std::string& filepath);
  void OnCaptureCleared();

  Ui::OrbitMainWindow* GetUi() { return ui; }

  bool eventFilter(QObject* watched, QEvent* event) override;

  void RestoreDefaultTabLayout();

  // TODO(170468590): [ui beta] When out of ui beta, this can return TargetConfiguration (without
  // std::optional)
  std::optional<orbit_qt::TargetConfiguration> ClearTargetConfiguration() {
    std::optional<orbit_qt::TargetConfiguration> result = std::move(target_configuration_);
    target_configuration_ = std::nullopt;
    return result;
  }

 protected:
  void closeEvent(QCloseEvent* event) override;

 private slots:
  void on_actionAbout_triggered();

  void on_actionReport_Missing_Feature_triggered();
  void on_actionReport_Bug_triggered();

  void OnTimer();
  void OnLiveTabFunctionsFilterTextChanged(const QString& text);
  void OnFilterFunctionsTextChanged(const QString& text);
  void OnFilterTracksTextChanged(const QString& text);

  void on_actionOpen_Preset_triggered();
  void on_actionEnd_Session_triggered();
  void on_actionQuit_triggered();
  void on_actionSave_Preset_As_triggered();

  void on_actionToggle_Capture_triggered();
  void on_actionSave_Capture_triggered();
  void on_actionOpen_Capture_triggered();
  void on_actionClear_Capture_triggered();
  void on_actionHelp_triggered();
  void on_actionIntrospection_triggered();

  void on_actionCheckFalse_triggered();
  void on_actionNullPointerDereference_triggered();
  void on_actionStackOverflow_triggered();
  void on_actionServiceCheckFalse_triggered();
  void on_actionServiceNullPointerDereference_triggered();
  void on_actionServiceStackOverflow_triggered();

  void ShowCaptureOnSaveWarningIfNeeded();
  void OnTimerSelectionChanged(const orbit_client_protos::TimerInfo* timer_info);
  void ShowEmptyFrameTrackWarningIfNeeded(std::string_view function);

 private:
  void StartMainTimer();
  void SetupCaptureToolbar();
  void SetupCodeView();
  void SetupMainWindow(uint32_t font_size);
  void SetupTargetLabel();

  void SaveCurrentTabLayoutAsDefaultInMemory();

  void CreateTabBarContextMenu(QTabWidget* tab_widget, int tab_index, const QPoint pos);
  void UpdateCaptureStateDependentWidgets();

  void UpdateActiveTabsAfterSelection(bool selection_has_samples);

  QTabWidget* FindParentTabWidget(const QWidget* widget) const;

  void SetTarget(const orbit_qt::StadiaTarget& target);
  void SetTarget(const orbit_qt::LocalTarget& target);
  void SetTarget(const orbit_qt::FileTarget& target);

  // TODO(170468590): [ui beta] When out of ui beta, this is not needed anymore (is done by
  // ProfilingTargetDialog)
  void SetupGrpcAndProcessManager(std::string grpc_server_address);

 private:
  std::unique_ptr<MainThreadExecutor> main_thread_executor_;
  std::unique_ptr<OrbitApp> app_;
  Ui::OrbitMainWindow* ui;
  QTimer* m_MainTimer = nullptr;
  std::vector<OrbitGLWidget*> gl_widgets_;
  OrbitGLWidget* introspection_widget_ = nullptr;
  QLabel* target_label_ = nullptr;

  // Capture toolbar.
  QIcon icon_start_capture_;
  QIcon icon_stop_capture_;

  QIcon icon_keyboard_arrow_left_;
  QIcon icon_keyboard_arrow_right_;

  // Status listener
  std::unique_ptr<StatusListener> status_listener_;

  struct TabWidgetLayout {
    std::vector<std::pair<QWidget*, QString>> tabs_and_titles;
    int current_index;
  };
  std::map<QTabWidget*, TabWidgetLayout> default_tab_layout_;

  // TODO(170468590): [ui beta] When out of ui beta, this process_manager_ is not needed anymore,
  // since the one from the Target is used;
  std::unique_ptr<ProcessManager> process_manager_;

  // TODO(170468590): [ui beta] When out of ui beta, this does not need to be an optional anymore
  std::optional<orbit_qt::TargetConfiguration> target_configuration_;
};

#endif  // ORBIT_QT_ORBIT_MAIN_WINDOW_H_
