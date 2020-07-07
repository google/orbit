// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QApplication>
#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <vector>

#include "ApplicationOptions.h"
#include "CallStackDataView.h"

namespace Ui {
class OrbitMainWindow;
}

class OrbitMainWindow : public QMainWindow {
  Q_OBJECT

 public:
  OrbitMainWindow(QApplication* a_App, ApplicationOptions&& options);
  ~OrbitMainWindow() override;

  void RegisterGlWidget(class OrbitGLWidget* a_GlWidget) {
    m_GlWidgets.push_back(a_GlWidget);
  }
  void OnRefreshDataViewPanels(DataViewType a_Type);
  void UpdatePanel(DataViewType a_Type);
  void OnNewSamplingReport(
      DataView* callstack_data_view,
      std::shared_ptr<class SamplingReport> sampling_report);
  void CreateSamplingTab();
  void CreateSelectionTab();
  void OnNewSelectionReport(
      DataView* callstack_data_view,
      std::shared_ptr<class SamplingReport> sampling_report);
  std::string OnGetSaveFileName(const std::string& extension);
  void OnSetClipboard(const std::string& text);
  void ParseCommandlineArguments();
  bool IsHeadless() { return m_Headless; }
  void PostInit();
  bool HideTab(QTabWidget* a_TabWidget, const char* a_TabName);
  std::string FindFile(const std::string& caption, const std::string& dir,
                       const std::string& filter);
  void OpenDisassembly(const std::string& a_String);
  void SetTitle(const QString& task_description);
  outcome::result<void> OpenCapture(const std::string& filepath);

 private slots:
  void on_actionAbout_triggered();

  void on_actionReport_Missing_Feature_triggered();
  void on_actionReport_Bug_triggered();

  void OnTimer();
  void OnHideSearch();

  void on_actionOpen_Preset_triggered();
  void on_actionQuit_triggered();

  void on_actionToogleDevMode_toggled(bool arg1);

  void on_actionSave_Preset_As_triggered();

  void on_actionSave_Capture_triggered();

  void on_actionOpen_Capture_triggered();

  void on_actionShow_Includes_Util_triggered();

  void on_actionCheckFalse_triggered();
  void on_actionNullPointerDereference_triggered();
  void on_actionStackOverflow_triggered();
  void on_actionServiceCheckFalse_triggered();
  void on_actionServiceNullPointerDereference_triggered();
  void on_actionServiceStackOverflow_triggered();

 private:
  void StartMainTimer();
  void SetupCodeView();
  void ShowFeedbackDialog();

 private:
  QApplication* m_App;
  Ui::OrbitMainWindow* ui;
  QTimer* m_MainTimer;
  std::vector<OrbitGLWidget*> m_GlWidgets;
  bool m_Headless;

  // sampling tab
  class QWidget* m_SamplingTab;
  class OrbitSamplingReport* m_OrbitSamplingReport;
  class QGridLayout* m_SamplingLayout;

  // selection tab
  class QWidget* m_SelectionTab;
  class OrbitSamplingReport* m_SelectionReport;
  class QGridLayout* m_SelectionLayout;

  class OutputDialog* m_OutputDialog;
  std::string m_CurrentPdbName;
  bool m_IsDev;
};
