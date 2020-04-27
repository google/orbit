//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "orbitmainwindow.h"

#include <QBuffer>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QTimer>
#include <QToolTip>
#include <utility>

#include "../OrbitCore/Path.h"
#include "../OrbitCore/PrintVar.h"
#include "../OrbitCore/Utils.h"
#include "../OrbitCore/Version.h"
#include "../OrbitGl/App.h"
#include "../OrbitGl/PluginManager.h"
#include "../OrbitGl/SamplingReport.h"
#include "../OrbitPlugin/OrbitSDK.h"
#include "../external/concurrentqueue/concurrentqueue.h"
#include "absl/strings/match.h"
#include "absl/strings/str_format.h"
#include "licensedialog.h"
#include "orbitdiffdialog.h"
#include "orbitdisassemblydialog.h"
#include "orbitsamplingreport.h"
#include "orbitvisualizer.h"
#include "outputdialog.h"
#include "showincludesdialog.h"
#include "ui_orbitmainwindow.h"

#ifdef _WIN32
#include <shellapi.h>
#endif

//-----------------------------------------------------------------------------
OrbitMainWindow* GMainWindow;
extern QMenu* GContextMenu;

//-----------------------------------------------------------------------------
OrbitMainWindow::OrbitMainWindow(QApplication* a_App,
                                 ApplicationOptions&& options)
    : QMainWindow(nullptr),
      m_App(a_App),
      ui(new Ui::OrbitMainWindow),
      m_Headless(false),
      m_IsDev(false) {
  OrbitApp::Init(std::move(options));

  ui->setupUi(this);
  ui->ProcessesList->SetProcessParams();

  QList<int> sizes;
  sizes.append(5000);
  sizes.append(5000);
  ui->HomeVerticalSplitter->setSizes(sizes);
  ui->HomeHorizontalSplitter->setSizes(sizes);
  ui->splitter_2->setSizes(sizes);

  GOrbitApp->AddRefreshCallback(
      [this](DataViewType a_Type) { this->OnRefreshDataViewPanels(a_Type); });
  GOrbitApp->AddSamplingReoprtCallback(
      [this](std::shared_ptr<SamplingReport> a_Report) {
        this->OnNewSamplingReport(std::move(a_Report));
      });
  GOrbitApp->AddSelectionReportCallback(
      [this](std::shared_ptr<SamplingReport> a_Report) {
        this->OnNewSelection(std::move(a_Report));
      });
  GOrbitApp->AddUiMessageCallback([this](const std::string& a_Message) {
    this->OnReceiveMessage(a_Message);
  });
  GOrbitApp->SetFindFileCallback([this](const std::wstring& a_Caption,
                                        const std::wstring& a_Dir,
                                        const std::wstring& a_Filter) {
    return this->FindFile(a_Caption, a_Dir, a_Filter);
  });
  GOrbitApp->AddWatchCallback(
      [this](const Variable* a_Variable) { this->OnAddToWatch(a_Variable); });
  GOrbitApp->SetSaveFileCallback([this](const std::string& extension) {
    return this->OnGetSaveFileName(extension);
  });
  GOrbitApp->SetClipboardCallback(
      [this](const std::wstring& a_Text) { this->OnSetClipboard(a_Text); });

  ParseCommandlineArguments();

  ui->DebugGLWidget->Initialize(GlPanel::DEBUG, this);
  ui->CaptureGLWidget->Initialize(GlPanel::CAPTURE, this);
  ui->VisualizeGLWidget->Initialize(GlPanel::VISUALIZE, this);

  ui->ModulesList->Initialize(DataViewType::MODULES);
  ui->FunctionsList->Initialize(DataViewType::FUNCTIONS);
  ui->LiveFunctionsList->Initialize(DataViewType::LIVE_FUNCTIONS);
  ui->CallStackView->Initialize(DataViewType::CALLSTACK);
  ui->TypesList->Initialize(DataViewType::TYPES);
  ui->GlobalsList->Initialize(DataViewType::GLOBALS);
  ui->SessionList->Initialize(DataViewType::SESSIONS);
  ui->OutputView->Initialize(DataViewType::LOG);

  SetupCodeView();
  SetupRuleEditor();

  if (!m_IsDev) {
    HideTab(ui->MainTabWidget, "debug");
    HideTab(ui->MainTabWidget, "visualize");
  }

  // Output window
  this->ui->plainTextEdit->SetIsOutputWindow();

  StartMainTimer();

  m_OutputDialog = new OutputDialog(this);
  m_OutputDialog->setWindowTitle("Orbit - Loading Pdb...");

  ui->actionEnable_Context_Switches->setChecked(
      GOrbitApp->GetTrackContextSwitches());
  ui->actionEnable_Unreal_Support->setChecked(
      GOrbitApp->GetUnrealSupportEnabled());
  ui->actionAllow_Unsafe_Hooking->setChecked(
      GOrbitApp->GetUnsafeHookingEnabled());
  ui->actionEnable_Sampling->setChecked(GOrbitApp->GetSamplingEnabled());
  ui->actionOutputDebugString->setChecked(
      GOrbitApp->GetOutputDebugStringEnabled());

  CreateSamplingTab();
  CreateSelectionTab();
  CreatePluginTabs();

  this->setWindowTitle("Orbit Profiler");
  std::string iconFileName = Path::GetExecutablePath() + "orbit.ico";
  this->setWindowIcon(QIcon(iconFileName.c_str()));

  GMainWindow = this;

  GOrbitApp->PostInit();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::SetupCodeView() {
  ui->CodeTextEdit->SetEditorType(OrbitCodeEditor::CODE_VIEW);
  ui->FileMappingTextEdit->SetEditorType(OrbitCodeEditor::FILE_MAPPING);
  ui->FileMappingTextEdit->SetSaveButton(ui->SaveFileMapping);
  ui->CodeTextEdit->SetFindLineEdit(ui->lineEdit);
  ui->FileMappingWidget->hide();
  OrbitCodeEditor::setFileMappingWidget(ui->FileMappingWidget);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::SetupRuleEditor() {
  m_RuleEditor = new OrbitVisualizer(this);
  m_RuleEditor->Initialize(this);
  m_RuleEditor->setWindowFlags(m_RuleEditor->windowFlags() |
                               Qt::WindowMinimizeButtonHint |
                               Qt::WindowMaximizeButtonHint);
  m_RuleEditor->hide();
}

//-----------------------------------------------------------------------------
OrbitMainWindow::~OrbitMainWindow() {
  delete m_OutputDialog;
  delete ui;
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::ParseCommandlineArguments() {
  std::vector<std::string> arguments;
  for (const auto& qt_argument : QCoreApplication::arguments()) {
    std::string argument = qt_argument.toStdString();
    if (absl::StrContains(argument, "inject:")) {
      m_Headless = true;
    } else if (argument == "dev") {
      m_IsDev = true;
    }

    arguments.push_back(std::move(argument));
  }

  GOrbitApp->SetCommandLineArguments(arguments);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::PostInit() {}

//-----------------------------------------------------------------------------
bool OrbitMainWindow::HideTab(QTabWidget* a_TabWidget, const char* a_TabName) {
  QTabWidget* tab = a_TabWidget;

  for (int i = 0; i < tab->count(); ++i) {
    std::string tabName = tab->tabText(i).toStdString();

    if (tabName == a_TabName) {
      tab->removeTab(i);
      return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------
std::wstring OrbitMainWindow::FindFile(const std::wstring& a_Caption,
                                       const std::wstring& a_Dir,
                                       const std::wstring& a_Filter) {
  QStringList list = QFileDialog::getOpenFileNames(
      this, ws2s(a_Caption).c_str(), ws2s(a_Dir).c_str(),
      ws2s(a_Filter).c_str());
  std::wstring result;
  for (auto& file : list) {
    result = file.toStdWString();
    break;
  }

  return result;
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnRefreshDataViewPanels(DataViewType a_Type) {
  if (a_Type == DataViewType::ALL) {
    for (int i = 0; i < DataViewType::ALL; ++i) {
      UpdatePanel((DataViewType)i);
    }
  } else {
    UpdatePanel(a_Type);
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::UpdatePanel(DataViewType a_Type) {
  switch (a_Type) {
    case DataViewType::CALLSTACK:
      ui->CallStackView->Refresh();
      break;
    case DataViewType::FUNCTIONS:
      ui->FunctionsList->Refresh();
      break;
    case DataViewType::LIVE_FUNCTIONS:
      ui->LiveFunctionsList->Refresh();
      break;
    case DataViewType::TYPES:
      ui->TypesList->Refresh();
      break;
    case DataViewType::GLOBALS:
      ui->GlobalsList->Refresh();
      break;
    case DataViewType::MODULES:
      ui->ModulesList->Refresh();
      break;
    case DataViewType::LOG:
      ui->OutputView->Refresh();
      break;
    case DataViewType::SESSIONS:
      ui->SessionList->Refresh();
      break;
    default:
      break;
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::CreateSamplingTab() {
  m_SamplingTab = new QWidget();
  m_SamplingLayout = new QGridLayout(m_SamplingTab);
  m_SamplingLayout->setSpacing(6);
  m_SamplingLayout->setContentsMargins(11, 11, 11, 11);
  m_OrbitSamplingReport = new OrbitSamplingReport(m_SamplingTab);
  m_SamplingLayout->addWidget(m_OrbitSamplingReport, 0, 0, 1, 1);

  ui->RightTabWidget->addTab(m_SamplingTab, QString("sampling"));
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::CreatePluginTabs() {
  for (Orbit::Plugin* plugin : GPluginManager.m_Plugins) {
    QWidget* widget = new QWidget();
    QGridLayout* layout = new QGridLayout(widget);
    layout->setSpacing(6);
    layout->setContentsMargins(11, 11, 11, 11);
    OrbitGLWidget* glWidget = new OrbitGLWidget(widget);
    layout->addWidget(glWidget, 0, 0, 1, 1);
    ui->RightTabWidget->addTab(widget, plugin->GetName());

    glWidget->Initialize(GlPanel::PLUGIN, this, (void*)plugin);
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnNewSamplingReport(
    std::shared_ptr<SamplingReport> a_SamplingReport) {
  m_SamplingLayout->removeWidget(m_OrbitSamplingReport);
  delete m_OrbitSamplingReport;

  m_OrbitSamplingReport = new OrbitSamplingReport(m_SamplingTab);
  m_OrbitSamplingReport->Initialize(a_SamplingReport);
  m_SamplingLayout->addWidget(m_OrbitSamplingReport, 0, 0, 1, 1);

  // Automatically switch to sampling tab if not already in live tab.
  if (ui->RightTabWidget->currentWidget() != ui->LiveTab) {
    ui->RightTabWidget->setCurrentWidget(m_SamplingTab);
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::CreateSelectionTab() {
  m_SelectionTab = new QWidget();
  m_SelectionLayout = new QGridLayout(m_SelectionTab);
  m_SelectionLayout->setSpacing(6);
  m_SelectionLayout->setContentsMargins(11, 11, 11, 11);
  m_SelectionReport = new OrbitSamplingReport(m_SelectionTab);
  m_SelectionLayout->addWidget(m_SelectionReport, 0, 0, 1, 1);
  ui->RightTabWidget->addTab(m_SelectionTab, QString("selection"));
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnNewSelection(
    std::shared_ptr<class SamplingReport> a_SamplingReport) {
  m_SelectionLayout->removeWidget(m_SelectionReport);
  delete m_SelectionReport;

  m_SelectionReport = new OrbitSamplingReport(m_SelectionTab);
  m_SelectionReport->Initialize(a_SamplingReport);
  m_SelectionLayout->addWidget(m_SelectionReport, 0, 0, 1, 1);

  ui->RightTabWidget->setCurrentWidget(m_SelectionTab);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnReceiveMessage(const std::string& a_Message) {
  if (a_Message == "ScreenShot") {
    QFile file("sshot.png");
    file.open(QIODevice::WriteOnly);
    QPixmap pixMap = this->grab();
    pixMap.save(&file, "PNG");

    std::wstring fileName = file.fileName().toStdWString();

#ifdef _WIN32
    ShellExecute(0, 0, fileName.c_str(), 0, 0, SW_SHOW);
#endif
  } else if (absl::StartsWith(a_Message, "code")) {
    ui->FileMappingWidget->hide();

    bool success = ui->CodeTextEdit->loadCode(a_Message);

    if (!success) {
      ui->FileMappingTextEdit->loadFileMap();
      ui->FileMappingWidget->show();
    }
  } else if (absl::StartsWith(a_Message, "tooltip:")) {
    QToolTip::showText(QCursor::pos(),
                       Replace(a_Message, "tooltip:", "").c_str(), this);
  } else if (absl::StartsWith(a_Message, "output")) {
    ui->MainTabWidget->setCurrentWidget(ui->OutputTab);
  } else if (absl::StartsWith(a_Message, "gotocode")) {
    ui->RightTabWidget->setCurrentWidget(ui->CodeTab);
  } else if (absl::StartsWith(a_Message, "gotocallstack")) {
    ui->RightTabWidget->setCurrentWidget(ui->CallStackTab);
  } else if (absl::StartsWith(a_Message, "startcapture")) {
    SetTitle("");
  } else if (absl::StartsWith(a_Message, "gotolive")) {
    ui->RightTabWidget->setCurrentWidget(ui->LiveTab);
  } else if (absl::StartsWith(a_Message, "gotocapture")) {
    ui->MainTabWidget->setCurrentWidget(ui->CaptureTab);
  } else if (absl::StartsWith(a_Message, "pdb:")) {
    if (GOrbitApp->IsLoading()) {
      m_CurrentPdbName = Replace(a_Message, "pdb:", "");
      m_OutputDialog->Reset();
      m_OutputDialog->SetStatus(m_CurrentPdbName);
      m_OutputDialog->show();
      this->setDisabled(true);
    }
  } else if (absl::StartsWith(a_Message, "pdbloaded")) {
    this->setDisabled(false);
    m_OutputDialog->hide();
  } else if (absl::StartsWith(a_Message, "status:")) {
    m_OutputDialog->SetStatus(Replace(a_Message, "status:", ""));
  } else if (absl::StartsWith(a_Message, "log:")) {
    m_OutputDialog->AddLog(Replace(a_Message, "log:", ""));
  } else if (absl::StartsWith(a_Message, "asm:")) {
    OpenDisassembly(a_Message);
  } else if (absl::StartsWith(a_Message, "RuleEditor")) {
    m_RuleEditor->show();
  } else if (absl::StartsWith(a_Message, "UpdateProcessParams")) {
    ui->ProcessesList->UpdateProcessParams();
  } else if (absl::StartsWith(a_Message, "SetProcessParams")) {
    ui->ProcessesList->SetProcessParams();
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnAddToWatch(const class Variable* a_Variable) {
  ui->WatchWidget->AddToWatch(a_Variable);
  ui->MainTabWidget->setCurrentWidget(ui->WatchTab);
}

//-----------------------------------------------------------------------------
std::string OrbitMainWindow::OnGetSaveFileName(const std::string& extension) {
  std::string filename =
      QFileDialog::getSaveFileName(this, "Specify a file to save...", nullptr,
                                   extension.c_str())
          .toStdString();
  if (!filename.empty() && !absl::EndsWith(filename, extension)) {
    filename += extension;
  }
  return filename;
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnSetClipboard(const std::wstring& a_Text) {
  QApplication::clipboard()->setText(QString::fromStdWString(a_Text));
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionAbout_triggered() {
  std::string title = "Orbit Profiler";
  title += " | Version " + GOrbitApp->GetVersion();
  std::string text = "Copyright (c) 2013-2018 - Pierric Gimmig\n Qt:";
  text += qVersion();

  QMessageBox::about(this, title.c_str(), text.c_str());
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::StartMainTimer() {
  m_MainTimer = new QTimer(this);
  connect(m_MainTimer, SIGNAL(timeout()), this, SLOT(OnTimer()));

  // Update period set to 16ms (~60FPS)
  int msec = 16;
  m_MainTimer->start(msec);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnTimer() {
  OrbitApp::MainTick();

  for (OrbitGLWidget* glWidget : m_GlWidgets) {
    glWidget->update();
  }

  // Output window
  this->ui->plainTextEdit->OnTimer();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnHideSearch() { ui->lineEdit->hide(); }

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOpen_Capture_triggered() {}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionSave_Session_triggered() {
  std::string sessionName = GOrbitApp->GetSessionFileName();
  if (!sessionName.empty()) {
    GOrbitApp->OnSaveSession(sessionName);
  } else {
    on_actionSave_Session_As_triggered();
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOpen_Session_triggered() {
  QStringList list = QFileDialog::getOpenFileNames(
      this, "Select a file to open...", Path::GetPresetPath().c_str(), "*.opr");
  for (const auto& file : list) {
    bool loaded = GOrbitApp->OnLoadSession(file.toStdString());
    if (!loaded) {
      QMessageBox::critical(
          this, "Error loading session",
          absl::StrFormat("Could not load session from \"%s\".",
                          file.toStdString())
              .c_str());
    }
    break;
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOpen_PDB_triggered() {
  QStringList list = QFileDialog::getOpenFileNames(
      this, "Select a pdb file to open...", "", "*.pdb");
  for (auto& file : list) {
    GOrbitApp->OnOpenPdb(file.toStdString());
    break;
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionDisconnect_triggered() {
  GOrbitApp->OnDisconnect();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionQuit_triggered() { QApplication::quit(); }

//-----------------------------------------------------------------------------
QPixmap QtGrab(OrbitMainWindow* a_Window) {
  QPixmap pixMap = a_Window->grab();
  if (GContextMenu) {
    QPixmap menuPixMap = GContextMenu->grab();
    pixMap.copy();
  }
  return pixMap;
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionToogleDevMode_toggled(bool a_Toggle) {
  UNUSED(a_Toggle);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionSave_Session_As_triggered() {
  QString file =
      QFileDialog::getSaveFileName(this, "Specify a file to save...",
                                   Path::GetPresetPath().c_str(), "*.opr");
  if (!file.isEmpty()) {
    printf("filename: %s\n", file.toStdString().c_str());
    GOrbitApp->OnSaveSession(file.toStdString());
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionEnable_Context_Switches_triggered() {}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionEnable_Context_Switches_triggered(bool checked) {
  GOrbitApp->SetTrackContextSwitches(checked);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionLaunch_Process_triggered() {
  QStringList list = QFileDialog::getOpenFileNames(
      this, "Select an executable to launch...", "", "*.exe");
  for (auto& file : list) {
    GOrbitApp->OnLaunchProcess(file.toStdString(), "", "");
    break;
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionEnable_Unreal_Support_triggered(bool checked) {
  GOrbitApp->EnableUnrealSupport(checked);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionAllow_Unsafe_Hooking_triggered(bool checked) {
  GOrbitApp->EnableUnsafeHooking(checked);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionEnable_Sampling_triggered(bool checked) {
  GOrbitApp->EnableSampling(checked);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionEnable_Sampling_toggled(bool arg1) {
  UNUSED(arg1);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionSave_Capture_triggered() {
  QString file = QFileDialog::getSaveFileName(
      this, "Save capture...",
      (Path::GetCapturePath() + ws2s(GOrbitApp->GetCaptureFileName())).c_str(),
      "*.orbit");
  GOrbitApp->OnSaveCapture(file.toStdString());
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOpen_Capture_2_triggered() {
  QStringList list = QFileDialog::getOpenFileNames(
      this, "Open capture...", Path::GetCapturePath().c_str(), "*.orbit");
  for (auto& file : list) {
    GOrbitApp->OnLoadCapture(file.toStdString());
    SetTitle(file.toStdString());
    ui->MainTabWidget->setCurrentWidget(ui->CaptureTab);
    break;
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionShow_Includes_Util_triggered() {
  ShowIncludesDialog* dialog = new ShowIncludesDialog(this);
  dialog->setWindowTitle("Orbit Show Includes Utility");
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowMinimizeButtonHint |
                         Qt::WindowMaximizeButtonHint);
  dialog->show();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionDiff_triggered() {
  OrbitDiffDialog* dialog = new OrbitDiffDialog(this);
  dialog->setWindowTitle("Orbit Diff Utility");
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowMinimizeButtonHint |
                         Qt::WindowMaximizeButtonHint);
  dialog->show();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OpenDisassembly(const std::string& a_String) {
  OrbitDisassemblyDialog* dialog = new OrbitDisassemblyDialog(this);
  dialog->SetText(a_String);
  dialog->setWindowTitle("Orbit Disassembly");
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowMinimizeButtonHint |
                         Qt::WindowMaximizeButtonHint);
  dialog->show();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::SetTitle(const std::string& a_Title) {
  std::string title = "Orbit Profiler";
  if (a_Title != "") {
    title += " - ";
    title += a_Title;
  }
  this->setWindowTitle(title.c_str());
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOutputDebugString_triggered(bool checked) {
  GOrbitApp->EnableOutputDebugString(checked);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionRule_Editor_triggered() { m_RuleEditor->show(); }
