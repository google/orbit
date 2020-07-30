// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbitmainwindow.h"

#include <QBuffer>
#include <QCheckBox>
#include <QClipboard>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPointer>
#include <QProgressDialog>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QToolTip>
#include <utility>

#include "App.h"
#include "MainThreadExecutorImpl.h"
#include "OrbitVersion.h"
#include "SamplingReport.h"
#include "TopDownViewItemModel.h"
#include "absl/flags/flag.h"
#include "absl/strings/match.h"
#include "absl/strings/str_format.h"
#include "orbitaboutdialog.h"
#include "orbitcodeeditor.h"
#include "orbitdisassemblydialog.h"
#include "orbitlivefunctions.h"
#include "orbitsamplingreport.h"
#include "outputdialog.h"
#include "services.pb.h"
#include "ui_orbitmainwindow.h"

#ifdef _WIN32
#include <shellapi.h>
#endif

ABSL_DECLARE_FLAG(bool, enable_stale_features);
ABSL_DECLARE_FLAG(bool, devmode);

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
  OrbitApp::Init(std::move(options), CreateMainThreadExecutor());

  DataViewFactory* data_view_factory = GOrbitApp.get();

  ui->setupUi(this);

  ui->ProcessesList->SetDataView(
      data_view_factory->GetOrCreateDataView(DataViewType::PROCESSES));

  QList<int> sizes;
  sizes.append(5000);
  sizes.append(5000);
  ui->HomeVerticalSplitter->setSizes(sizes);
  ui->HomeHorizontalSplitter->setSizes(sizes);
  ui->splitter_2->setSizes(sizes);

  GOrbitApp->SetCaptureStartedCallback([this] {
    ui->actionStart_Capture->setDisabled(true);
    ui->actionStop_Capture->setDisabled(false);
    ui->actionClear_Capture->setDisabled(true);
    ui->actionOpen_Capture->setDisabled(true);
    ui->actionSave_Capture->setDisabled(true);
    ui->actionOpen_Preset->setDisabled(true);
    ui->actionSave_Preset_As->setDisabled(true);
    ui->HomeTab->setDisabled(true);
    SetTitle({});
  });

  auto finalizing_capture_dialog = new QProgressDialog(
      "Waiting for the remaining capture data...", "OK", 0, 0, this, Qt::Tool);
  finalizing_capture_dialog->setWindowTitle("Finalizing capture");
  finalizing_capture_dialog->setModal(true);
  finalizing_capture_dialog->setWindowFlags(
      (finalizing_capture_dialog->windowFlags() | Qt::CustomizeWindowHint) &
      ~Qt::WindowCloseButtonHint & ~Qt::WindowSystemMenuHint);
  finalizing_capture_dialog->setFixedSize(finalizing_capture_dialog->size());
  finalizing_capture_dialog->close();

  GOrbitApp->SetCaptureStopRequestedCallback([this, finalizing_capture_dialog] {
    ui->actionStop_Capture->setDisabled(true);
    finalizing_capture_dialog->show();
  });
  GOrbitApp->SetCaptureStoppedCallback([this, finalizing_capture_dialog] {
    finalizing_capture_dialog->close();
    ui->actionStart_Capture->setDisabled(false);
    ui->actionOpen_Capture->setDisabled(false);
    ui->actionSave_Capture->setDisabled(false);
    ui->actionOpen_Preset->setDisabled(false);
    ui->actionSave_Preset_As->setDisabled(false);
    ui->actionClear_Capture->setDisabled(false);
    ui->HomeTab->setDisabled(false);
  });
  GOrbitApp->SetCaptureClearedCallback([this] {
    OnCaptureCleared();
  });

  GOrbitApp->SetRefreshCallback([this](DataViewType a_Type) {
    this->OnRefreshDataViewPanels(a_Type);
    this->ui->liveFunctions->OnDataChanged();
  });
  GOrbitApp->SetSamplingReportCallback(
      [this](DataView* callstack_data_view,
             std::shared_ptr<SamplingReport> report) {
        this->OnNewSamplingReport(callstack_data_view, std::move(report));
      });
  GOrbitApp->SetSelectionReportCallback(
      [this](DataView* callstack_data_view,
             std::shared_ptr<SamplingReport> report) {
        this->OnNewSelectionReport(callstack_data_view, std::move(report));
      });
  GOrbitApp->SetTopDownViewCallback(
      [this](std::unique_ptr<TopDownView> top_down_view) {
        this->OnNewTopDownView(std::move(top_down_view));
      });

  GOrbitApp->SetOpenCaptureCallback(
      [this] { on_actionOpen_Capture_triggered(); });
  GOrbitApp->SetSaveCaptureCallback(
      [this] { on_actionSave_Capture_triggered(); });
  GOrbitApp->SetSelectLiveTabCallback(
      [this] { ui->RightTabWidget->setCurrentWidget(ui->liveTab); });
  GOrbitApp->SetDisassemblyCallback(
      [this](const std::string& disassembly, const DisassemblyReport& report) {
        OpenDisassembly(disassembly, report);
      });
  GOrbitApp->SetErrorMessageCallback(
      [this](const std::string& title, const std::string& text) {
        QMessageBox::critical(this, QString::fromStdString(title),
                              QString::fromStdString(text));
      });
  GOrbitApp->SetInfoMessageCallback(
      [this](const std::string& title, const std::string& text) {
        QMessageBox::information(this, QString::fromStdString(title),
                                 QString::fromStdString(text));
      });
  GOrbitApp->SetTooltipCallback([this](const std::string& tooltip) {
    QToolTip::showText(QCursor::pos(), QString::fromStdString(tooltip), this);
  });
  GOrbitApp->SetFeedbackDialogCallback([this] { ShowFeedbackDialog(); });

  GOrbitApp->SetFindFileCallback([this](const std::string& caption,
                                        const std::string& dir,
                                        const std::string& filter) {
    return this->FindFile(caption, dir, filter);
  });
  GOrbitApp->SetSaveFileCallback([this](const std::string& extension) {
    return this->OnGetSaveFileName(extension);
  });
  GOrbitApp->SetClipboardCallback(
      [this](const std::string& text) { this->OnSetClipboard(text); });

  ParseCommandlineArguments();

  ui->DebugGLWidget->Initialize(GlPanel::DEBUG, this);
  ui->CaptureGLWidget->Initialize(GlPanel::CAPTURE, this);
  ui->VisualizeGLWidget->Initialize(GlPanel::VISUALIZE, this);

  ui->ModulesList->Initialize(
      data_view_factory->GetOrCreateDataView(DataViewType::MODULES),
      SelectionType::kExtended, FontType::kDefault);
  ui->FunctionsList->Initialize(
      data_view_factory->GetOrCreateDataView(DataViewType::FUNCTIONS),
      SelectionType::kExtended, FontType::kDefault);
  ui->CallStackView->Initialize(
      data_view_factory->GetOrCreateDataView(DataViewType::CALLSTACK),
      SelectionType::kExtended, FontType::kDefault);
  ui->SessionList->Initialize(
      data_view_factory->GetOrCreateDataView(DataViewType::PRESETS),
      SelectionType::kDefault, FontType::kDefault);

  SetupCodeView();

  if (!m_IsDev) {
    HideTab(ui->MainTabWidget, "debug");
    HideTab(ui->MainTabWidget, "visualize");
  }

  if (!absl::GetFlag(FLAGS_enable_stale_features)) {
    ui->MainTabWidget->removeTab(ui->MainTabWidget->indexOf(ui->OutputTab));

    ui->RightTabWidget->removeTab(
        ui->RightTabWidget->indexOf(ui->CallStackTab));
    ui->RightTabWidget->removeTab(ui->RightTabWidget->indexOf(ui->CodeTab));
  }

  if (!absl::GetFlag(FLAGS_devmode)) {
    ui->menuDebug->menuAction()->setVisible(false);
  }

  SetupCaptureToolbar();

  // Output window
  this->ui->plainTextEdit->SetIsOutputWindow();

  StartMainTimer();

  m_OutputDialog = new OutputDialog(this);
  m_OutputDialog->setWindowTitle("Orbit - Loading Pdb...");

  ui->liveFunctions->Initialize(SelectionType::kExtended, FontType::kDefault);

  connect(ui->liveFunctions->GetFilterLineEdit(), &QLineEdit::textChanged, this,
          [this](const QString& text) {
            OnLiveTabFunctionsFilterTextChanged(text);
          });

  SetTitle({});
  std::string iconFileName = Path::GetExecutablePath() + "orbit.ico";
  this->setWindowIcon(QIcon(iconFileName.c_str()));

  GMainWindow = this;

  GOrbitApp->PostInit();
}

namespace {
static void SetFontSize(QWidget* widget, uint32_t font_size) {
  QFont font = widget->font();
  font.setPointSize(font_size);
  widget->setFont(font);
}

static QWidget* CreateSpacer(QWidget* parent) {
  QLabel* spacer = new QLabel(parent);
  spacer->setText("    ");
  return spacer;
}

static QAction* CreateDummyAction(const QIcon& icon, QObject* parent) {
  QAction* action = new QAction(icon, "", parent);
  action->setDisabled(true);
  return action;
}

static QIcon GetIcon(const std::string& icon_name) {
  return QIcon(Path::JoinPath({Path::GetIconsPath(), icon_name}).c_str());
}
}  // namespace

void OrbitMainWindow::SetupCaptureToolbar() {
  // Sizes.
  uint32_t kFontSize = 10;
  uint32_t kIconSize = 30;
  QToolBar* toolbar = ui->capture_toolbar;
  toolbar->setIconSize(QSize(kIconSize, kIconSize));

  // Create icons.
  QIcon icon_start = GetIcon("outline_play_arrow_white_48dp.png");
  QIcon icon_stop = GetIcon("outline_stop_white_48dp.png");
  QIcon icon_clear = GetIcon("outline_clear_white_48dp.png");
  QIcon icon_open = GetIcon("outline_folder_white_48dp.png");
  QIcon icon_save = GetIcon("outline_save_alt_white_48dp.png");
  QIcon icon_help = GetIcon("outline_help_outline_white_48dp.png");
  QIcon icon_feedback = GetIcon("outline_feedback_white_48dp.png");
  QIcon icon_search = GetIcon("outline_search_white_48dp.png");
  QIcon icon_filter = GetIcon("outline_filter_list_white_48dp.png");
  QIcon icon_timer = GetIcon("outline_access_time_white_48dp.png");

  // Set action icons.
  ui->actionStart_Capture->setIcon(icon_start);
  ui->actionStop_Capture->setIcon(icon_stop);
  ui->actionClear_Capture->setIcon(icon_clear);
  ui->actionOpen_Capture->setIcon(icon_open);
  ui->actionSave_Capture->setIcon(icon_save);
  ui->actionHelp->setIcon(icon_help);
  ui->actionFeedback->setIcon(icon_feedback);
  ui->actionFilter_Functions->setIcon(icon_search);
  ui->actionFilter_Tracks->setIcon(icon_filter);

  // Add actions.
  toolbar->addAction(ui->actionStart_Capture);
  toolbar->addAction(ui->actionStop_Capture);
  toolbar->addAction(ui->actionClear_Capture);
  toolbar->addAction(ui->actionOpen_Capture);
  toolbar->addAction(ui->actionSave_Capture);
  toolbar->addAction(ui->actionHelp);
  toolbar->addAction(ui->actionFeedback);

  // Filter tracks.
  toolbar->addWidget(CreateSpacer(toolbar));
  toolbar->addAction(CreateDummyAction(icon_filter, toolbar));
  filter_tracks_line_edit_ = new QLineEdit(toolbar);
  filter_tracks_line_edit_->setClearButtonEnabled(true);
  filter_tracks_line_edit_->setPlaceholderText("filter tracks");
  SetFontSize(filter_tracks_line_edit_, kFontSize);
  toolbar->addWidget(filter_tracks_line_edit_);
  connect(filter_tracks_line_edit_, &QLineEdit::textChanged, this,
          [this](const QString& text) { OnFilterTracksTextChanged(text); });

  // Filter functions.
  toolbar->addWidget(CreateSpacer(toolbar));
  toolbar->addAction(CreateDummyAction(icon_search, toolbar));
  filter_functions_line_edit_ = new QLineEdit();
  filter_functions_line_edit_->setClearButtonEnabled(true);
  filter_functions_line_edit_->setPlaceholderText("filter functions");
  SetFontSize(filter_functions_line_edit_, kFontSize);
  toolbar->addWidget(filter_functions_line_edit_);
  connect(filter_functions_line_edit_, &QLineEdit::textChanged, this,
          [this](const QString& text) { OnFilterFunctionsTextChanged(text); });

  // Timer.
  toolbar->addWidget(CreateSpacer(toolbar));
  toolbar->addAction(CreateDummyAction(icon_timer, toolbar));
  timer_label_ = new QLabel(toolbar);
  timer_label_->setText("1s");
  SetFontSize(timer_label_, kFontSize);
  QFontMetrics fm(timer_label_->font());
  int pixel_width = fm.width("w");
  timer_label_->setMinimumWidth(5 * pixel_width);
  toolbar->addWidget(timer_label_);

  // Initial state.
  ui->actionStop_Capture->setDisabled(true);
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
void OrbitMainWindow::ShowFeedbackDialog() {
  QDialog feedback_dialog{nullptr,
                          Qt::WindowTitleHint | Qt::WindowCloseButtonHint};

  const auto layout = QPointer{new QGridLayout{&feedback_dialog}};
  const auto button_box =
      QPointer{new QDialogButtonBox{QDialogButtonBox::StandardButton::Close}};

  const QPointer<QPushButton> report_missing_feature_button =
      QPointer{new QPushButton{&feedback_dialog}};
  button_box->addButton(report_missing_feature_button,
                        QDialogButtonBox::AcceptRole);
  report_missing_feature_button->setText("Report Missing Feature");
  const QPointer<QPushButton> report_bug_button =
      QPointer{new QPushButton{&feedback_dialog}};
  button_box->addButton(report_bug_button, QDialogButtonBox::AcceptRole);
  report_bug_button->setText("Report Bug");

  layout->addWidget(button_box, 0, 0);

  QObject::connect(report_missing_feature_button, &QPushButton::clicked,
                   &feedback_dialog, [this, &feedback_dialog]() {
                     on_actionReport_Missing_Feature_triggered();
                     feedback_dialog.accept();
                   });

  QObject::connect(report_bug_button, &QPushButton::clicked, &feedback_dialog,
                   [this, &feedback_dialog]() {
                     on_actionReport_Bug_triggered();
                     feedback_dialog.accept();
                   });

  QObject::connect(button_box, &QDialogButtonBox::rejected, &feedback_dialog,
                   &QDialog::reject);

  feedback_dialog.exec();
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
std::string OrbitMainWindow::FindFile(const std::string& caption,
                                      const std::string& dir,
                                      const std::string& filter) {
  QStringList list = QFileDialog::getOpenFileNames(this, caption.c_str(),
                                                   dir.c_str(), filter.c_str());
  std::string result;
  for (auto& file : list) {
    result = file.toStdString();
    break;
  }

  return result;
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnRefreshDataViewPanels(DataViewType a_Type) {
  if (a_Type == DataViewType::ALL) {
    for (int i = 0; i < static_cast<int>(DataViewType::ALL); ++i) {
      UpdatePanel(static_cast<DataViewType>(i));
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
      ui->liveFunctions->Refresh();
      break;
    case DataViewType::MODULES:
      ui->ModulesList->Refresh();
      break;
    case DataViewType::PROCESSES:
      ui->ProcessesList->Refresh();
      break;
    case DataViewType::PRESETS:
      ui->SessionList->Refresh();
      break;
    case DataViewType::SAMPLING:
      ui->samplingReport->RefreshCallstackView();
      ui->samplingReport->RefreshTabs();
      ui->selectionReport->RefreshCallstackView();
      ui->selectionReport->RefreshTabs();
      break;
    default:
      break;
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnNewSamplingReport(
    DataView* callstack_data_view,
    std::shared_ptr<SamplingReport> sampling_report) {
  ui->samplingGridLayout->removeWidget(ui->samplingReport);
  delete ui->samplingReport;

  ui->samplingReport = new OrbitSamplingReport(ui->samplingTab);
  ui->samplingReport->Initialize(callstack_data_view,
                                 std::move(sampling_report));
  ui->samplingGridLayout->addWidget(ui->samplingReport, 0, 0, 1, 1);

  // Automatically switch to sampling tab if not already in live tab.
  if (ui->RightTabWidget->currentWidget() != ui->liveTab) {
    ui->RightTabWidget->setCurrentWidget(ui->samplingTab);
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnNewSelectionReport(
    DataView* callstack_data_view,
    std::shared_ptr<SamplingReport> sampling_report) {
  ui->selectionGridLayout->removeWidget(ui->selectionReport);
  delete ui->selectionReport;

  ui->selectionReport = new OrbitSamplingReport(ui->selectionTab);
  ui->selectionReport->Initialize(callstack_data_view,
                                  std::move(sampling_report));
  ui->selectionGridLayout->addWidget(ui->selectionReport, 0, 0, 1, 1);

  ui->RightTabWidget->setCurrentWidget(ui->selectionTab);
}

void OrbitMainWindow::OnNewTopDownView(
    std::unique_ptr<TopDownView> top_down_view) {
  auto* model =
      new TopDownViewItemModel{std::move(top_down_view), ui->topDownView};
  auto* proxy_model = new QSortFilterProxyModel{ui->topDownView};
  proxy_model->setSourceModel(model);
  proxy_model->setSortRole(Qt::EditRole);
  ui->topDownView->setModel(proxy_model);
  ui->topDownView->sortByColumn(TopDownViewItemModel::kInclusive,
                                Qt::DescendingOrder);
  for (int column = 0; column < TopDownViewItemModel::kColumnCount; ++column) {
    ui->topDownView->resizeColumnToContents(column);
  }
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
void OrbitMainWindow::OnSetClipboard(const std::string& text) {
  QApplication::clipboard()->setText(QString::fromStdString(text));
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionReport_Missing_Feature_triggered() {
  if (!QDesktopServices::openUrl(
          QUrl("https://community.stadia.dev/s/feature-requests",
               QUrl::StrictMode))) {
    QMessageBox::critical(
        this, "Error opening URL",
        "Could not open community.stadia.dev/s/feature-request");
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionReport_Bug_triggered() {
  if (!QDesktopServices::openUrl(QUrl(
          "https://community.stadia.dev/s/contactsupport", QUrl::StrictMode))) {
    QMessageBox::critical(
        this, "Error opening URL",
        "Could not open community.stadia.dev/s/contactsupport");
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionAbout_triggered() {
  OrbitQt::OrbitAboutDialog dialog{this};
  dialog.setWindowTitle(windowTitle());
  dialog.SetVersionString(QCoreApplication::applicationVersion());
  dialog.SetBuildInformation(
      QString::fromStdString(OrbitCore::GetBuildReport()));

  QFile licenseFile{
      QDir{QCoreApplication::applicationDirPath()}.filePath("NOTICE")};
  if (licenseFile.open(QIODevice::ReadOnly)) {
    dialog.SetLicenseText(licenseFile.readAll());
  }
  dialog.exec();
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

  if (timer_label_) {
    timer_label_->setText(QString::fromStdString(GOrbitApp->GetCaptureTime()));
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnHideSearch() { ui->lineEdit->hide(); }

void OrbitMainWindow::OnFilterFunctionsTextChanged(const QString& text) {
  // The toolbar and live tab filters are mirrored.
  ui->liveFunctions->SetFilter(text);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnLiveTabFunctionsFilterTextChanged(const QString& text) {
  GOrbitApp->FilterFunctions(text.toStdString());

  // Set main toolbar functions filter without triggering signals.
  filter_functions_line_edit_->blockSignals(true);
  filter_functions_line_edit_->setText(text);
  filter_functions_line_edit_->blockSignals(false);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OnFilterTracksTextChanged(const QString& text) {
  GOrbitApp->FilterTracks(text.toStdString());
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOpen_Preset_triggered() {
  QStringList list = QFileDialog::getOpenFileNames(
      this, "Select a file to open...", Path::GetPresetPath().c_str(), "*.opr");
  for (const auto& file : list) {
    ErrorMessageOr<void> result = GOrbitApp->OnLoadPreset(file.toStdString());
    if (result.has_error()) {
      QMessageBox::critical(
          this, "Error loading session",
          absl::StrFormat("Could not load session from \"%s\":\n%s.",
                          file.toStdString(), result.error().message())
              .c_str());
    }
    break;
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionQuit_triggered() {
  close();
  QApplication::quit();
}

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
void OrbitMainWindow::on_actionSave_Preset_As_triggered() {
  QString file =
      QFileDialog::getSaveFileName(this, "Specify a file to save...",
                                   Path::GetPresetPath().c_str(), "*.opr");
  if (file.isEmpty()) {
    return;
  }

  ErrorMessageOr<void> result = GOrbitApp->OnSavePreset(file.toStdString());
  if (result.has_error()) {
    QMessageBox::critical(
        this, "Error saving session",
        absl::StrFormat("Could not save session in \"%s\":\n%s.",
                        file.toStdString(), result.error().message())
            .c_str());
  }
}

void OrbitMainWindow::on_actionStart_Capture_triggered() {
  GOrbitApp->StartCapture();
}

void OrbitMainWindow::on_actionStop_Capture_triggered() {
  GOrbitApp->StopCapture();
}

void OrbitMainWindow::on_actionClear_Capture_triggered() {
  GOrbitApp->ClearCapture();
}

void OrbitMainWindow::on_actionHelp_triggered() { GOrbitApp->ToggleDrawHelp(); }

void OrbitMainWindow::on_actionFeedback_triggered() { ShowFeedbackDialog(); }

void OrbitMainWindow::ShowCaptureOnSaveWarningIfNeeded() {
  QSettings settings("The Orbit Authors", "Orbit Profiler");
  const QString skip_capture_warning("SkipCaptureVersionWarning");
  if (!settings.value(skip_capture_warning, false).toBool()) {
    QMessageBox message_box;
    message_box.setText(
        "Note: Captures saved with this version of Orbit might be incompatible "
        "with future versions. Please check release notes for more "
        "information");
    message_box.addButton(QMessageBox::Ok);
    QCheckBox check_box("Don't show this message again.");
    message_box.setCheckBox(&check_box);

    QObject::connect(&check_box, &QCheckBox::stateChanged,
                     [&settings, &skip_capture_warning](int state) {
                       settings.setValue(skip_capture_warning,
                                         static_cast<bool>(state));
                     });

    message_box.exec();
  }
}

void OrbitMainWindow::on_actionSave_Capture_triggered() {
  ShowCaptureOnSaveWarningIfNeeded();

  QString file = QFileDialog::getSaveFileName(
      this, "Save capture...",
      Path::JoinPath({Path::GetCapturePath(), GOrbitApp->GetCaptureFileName()})
          .c_str(),
      "*.orbit");
  if (file.isEmpty()) {
    return;
  }

  ErrorMessageOr<void> result = GOrbitApp->OnSaveCapture(file.toStdString());
  if (result.has_error()) {
    QMessageBox::critical(
        this, "Error saving capture",
        absl::StrFormat("Could not save capture in \"%s\":\n%s.",
                        file.toStdString(), result.error().message())
            .c_str());
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionOpen_Capture_triggered() {
  QString file = QFileDialog::getOpenFileName(
      this, "Open capture...", QString::fromStdString(Path::GetCapturePath()),
      "*.orbit");
  if (file.isEmpty()) {
    return;
  }

  (void)OpenCapture(file.toStdString());
}

outcome::result<void> OrbitMainWindow::OpenCapture(
    const std::string& filepath) {
  ErrorMessageOr<void> result = GOrbitApp->OnLoadCapture(filepath);

  if (result.has_error()) {
    SetTitle({});
    QMessageBox::critical(
        this, "Error loading capture",
        QString::fromStdString(absl::StrFormat(
            "Could not load capture from \"%s\":\n%s",
            filepath, result.error().message())));
    return std::errc::no_such_file_or_directory;
  }
  SetTitle(QString::fromStdString(filepath));
  ui->MainTabWidget->setCurrentWidget(ui->CaptureTab);
  return outcome::success();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::OpenDisassembly(
    const std::string& a_String,
    const DisassemblyReport& report) {
  auto* dialog = new OrbitDisassemblyDialog(this);
  dialog->SetText(a_String);
  dialog->SetDisassemblyReport(report);
  dialog->setWindowTitle("Orbit Disassembly");
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowMinimizeButtonHint |
                         Qt::WindowMaximizeButtonHint);
  dialog->show();
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::SetTitle(const QString& task_description) {
  if (task_description.isEmpty()) {
    setWindowTitle(QString("%1 %2").arg(QApplication::applicationName(),
                                        QApplication::applicationVersion()));
  } else {
    setWindowTitle(QString("%1 %2 - %3")
                       .arg(QApplication::applicationName(),
                            QApplication::applicationVersion(),
                            task_description));
  }
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionCheckFalse_triggered() { CHECK(false); }

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionNullPointerDereference_triggered() {
  int* null_pointer = nullptr;
  *null_pointer = 0;
}

//-----------------------------------------------------------------------------
void InfiniteRecursion(int num) {
  if (num != 1) {
    InfiniteRecursion(num);
  }
  LOG("%s", VAR_TO_STR(num).c_str());
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionStackOverflow_triggered() {
  InfiniteRecursion(0);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionServiceCheckFalse_triggered() {
  GOrbitApp->CrashOrbitService(CrashOrbitServiceRequest_CrashType_CHECK_FALSE);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionServiceNullPointerDereference_triggered() {
  GOrbitApp->CrashOrbitService(
      CrashOrbitServiceRequest_CrashType_NULL_POINTER_DEREFERENCE);
}

//-----------------------------------------------------------------------------
void OrbitMainWindow::on_actionServiceStackOverflow_triggered() {
  GOrbitApp->CrashOrbitService(
      CrashOrbitServiceRequest_CrashType_STACK_OVERFLOW);
}

void OrbitMainWindow::OnCaptureCleared() {
  ui->liveFunctions->Reset();
}
