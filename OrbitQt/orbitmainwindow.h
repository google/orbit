//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <QMainWindow>
#include <atomic>
#include <memory>
#include <thread>

#include "ApplicationOptions.h"
#include "DataViewTypes.h"

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
      std::shared_ptr<class SamplingReport> a_SamplingReport);
  void CreateSamplingTab();
  void CreateSelectionTab();
  void CreatePluginTabs();
  void OnNewSelection(std::shared_ptr<class SamplingReport> a_SamplingReport);
  void OnReceiveMessage(const std::string& message);
  void OnAddToWatch(const class Variable* a_Variable);
  std::string OnGetSaveFileName(const std::string& extension);
  void OnSetClipboard(const std::wstring& a_Text);
  void ParseCommandlineArguments();
  bool IsHeadless() { return m_Headless; }
  void PostInit();
  bool HideTab(QTabWidget* a_TabWidget, const char* a_TabName);
  std::wstring FindFile(const std::wstring& a_Caption,
                        const std::wstring& a_Dir,
                        const std::wstring& a_Filter);
  void OpenDisassembly(const std::string& a_String);
  void SetTitle(const std::string& a_Title);

 private slots:
  void on_actionAbout_triggered();
  void OnTimer();
  void OnHideSearch();

  void on_actionOpen_Capture_triggered();
  void on_actionSave_Session_triggered();
  void on_actionOpen_Session_triggered();
  void on_actionOpen_PDB_triggered();
  void on_actionDisconnect_triggered();
  void on_actionQuit_triggered();
  void on_actionLaunch_Process_triggered();

  void on_actionToogleDevMode_toggled(bool arg1);

  void on_actionSave_Session_As_triggered();

  void on_actionEnable_Context_Switches_triggered();

  void on_actionEnable_Context_Switches_triggered(bool checked);

  void on_actionEnable_Unreal_Support_triggered(bool checked);

  void on_actionAllow_Unsafe_Hooking_triggered(bool checked);

  void on_actionEnable_Sampling_triggered(bool checked);

  void on_actionEnable_Sampling_toggled(bool arg1);

  void on_actionSave_Capture_triggered();

  void on_actionOpen_Capture_2_triggered();

  void on_actionShow_Includes_Util_triggered();

  void on_actionDiff_triggered();

  void on_actionOutputDebugString_triggered(bool checked);

  void on_actionRule_Editor_triggered();

 private:
  void StartMainTimer();
  void GetLicense();

  void SetupCodeView();
  void SetupRuleEditor();

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

  // Rule editor
  class OrbitVisualizer* m_RuleEditor;

  class OutputDialog* m_OutputDialog;
  std::string m_CurrentPdbName;
  bool m_IsDev;
};
