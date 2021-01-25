// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbitmainwindow.h"

#include <absl/flags/declare.h>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QCoreApplication>
#include <QCursor>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFlags>
#include <QFontMetrics>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIODevice>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaObject>
#include <QMouseEvent>
#include <QPixmap>
#include <QPointer>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QStringList>
#include <QTabBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QToolTip>
#include <QUrl>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>
#include <Qt>
#include <array>
#include <filesystem>
#include <initializer_list>
#include <memory>
#include <optional>
#include <utility>
#include <variant>

#include "App.h"
#include "CallTreeWidget.h"
#include "CaptureOptionsDialog.h"
#include "CodeViewer/Dialog.h"
#include "CodeViewer/FontSizeInEm.h"
#include "Connections.h"
#include "DataViewFactory.h"
#include "GlCanvas.h"
#include "LiveFunctionsController.h"
#include "LiveFunctionsDataView.h"
#include "MainThreadExecutorImpl.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Tracing.h"
#include "OrbitCaptureClient/CaptureClient.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientModel/CaptureData.h"
#include "OrbitClientModel/CaptureSerializer.h"
#include "OrbitClientServices/ProcessManager.h"
#include "OrbitGgp/Instance.h"
#include "OrbitVersion/OrbitVersion.h"
#include "Path.h"
#include "SamplingReport.h"
#include "StatusListenerImpl.h"
#include "SyntaxHighlighter/X86Assembly.h"
#include "TargetConfiguration.h"
#include "TutorialContent.h"
#include "absl/flags/flag.h"
#include "absl/strings/match.h"
#include "absl/strings/str_format.h"
#include "orbitaboutdialog.h"
#include "orbitdataviewpanel.h"
#include "orbitglwidget.h"
#include "orbitlivefunctions.h"
#include "orbitsamplingreport.h"
#include "servicedeploymanager.h"
#include "services.pb.h"
#include "types.h"
#include "ui_orbitmainwindow.h"

ABSL_DECLARE_FLAG(bool, devmode);
ABSL_DECLARE_FLAG(bool, enable_tracepoint_feature);
ABSL_DECLARE_FLAG(bool, enable_tutorials_feature);

using orbit_grpc_protos::CrashOrbitServiceRequest_CrashType;
using orbit_grpc_protos::CrashOrbitServiceRequest_CrashType_CHECK_FALSE;
using orbit_grpc_protos::CrashOrbitServiceRequest_CrashType_NULL_POINTER_DEREFERENCE;
using orbit_grpc_protos::CrashOrbitServiceRequest_CrashType_STACK_OVERFLOW;

extern QMenu* GContextMenu;

namespace {
const QString kLightGrayColor = "rgb(117, 117, 117)";
const QString kMediumGrayColor = "rgb(68, 68, 68)";
const QString kGreenColor = "rgb(41, 218, 130)";
constexpr int kHintFramePosX = 21;
constexpr int kHintFramePosY = 47;
constexpr int kHintFrameWidth = 140;
constexpr int kHintFrameHeight = 45;

const QString kTargetLabelDefaultStyleSheet = "#TargetLabel { color: %1; }";
const QString kTargetLabelColorConnected = "#66BB6A";
const QString kTargetLabelColorFileTarget = "#BDBDBD";
const QString kTargetLabelColorTargetProcessDied = "orange";

void OpenDisassembly(const std::string& assembly, const DisassemblyReport& report) {
  orbit_code_viewer::Dialog dialog{};
  dialog.setWindowTitle("Orbit Disassembly");
  dialog.SetEnableLineNumbers(true);

  auto syntax_highlighter = std::make_unique<orbit_syntax_highlighter::X86Assembly>();
  dialog.SetSourceCode(QString::fromStdString(assembly), std::move(syntax_highlighter));

  constexpr orbit_code_viewer::FontSizeInEm kHeatmapAreaWidth{1.3f};
  dialog.SetHeatmap(kHeatmapAreaWidth, [&report](int line_number) {
    if (report.GetNumSamples() == 0) return 0.0f;

    return static_cast<float>(report.GetNumSamplesAtLine(line_number)) /
           static_cast<float>(report.GetNumSamples());
  });

  dialog.exec();
}
}  // namespace

OrbitMainWindow::OrbitMainWindow(orbit_qt::TargetConfiguration target_configuration,
                                 uint32_t font_size,
                                 orbit_metrics_uploader::MetricsUploader* metrics_uploader)
    : QMainWindow(nullptr),
      main_thread_executor_{orbit_qt::MainThreadExecutorImpl::Create()},
      app_{OrbitApp::Create(main_thread_executor_.get(), metrics_uploader)},
      ui(new Ui::OrbitMainWindow),
      target_configuration_(std::move(target_configuration)) {
  SetupMainWindow(font_size);

  SetupTargetLabel();
  SetupHintFrame();

  ui->RightTabWidget->setTabText(ui->RightTabWidget->indexOf(ui->FunctionsTab), "Symbols");

  // TODO (177304549): This next line is still here from the old UI. Remove ui->HomeTab completely
  ui->MainTabWidget->removeTab(ui->MainTabWidget->indexOf(ui->HomeTab));

  // remove Functions list from position 0,0
  ui->functionsTabLayout->removeItem(ui->functionsTabLayout->itemAtPosition(0, 0));

  auto* symbols_vertical_splitter = new QSplitter(Qt::Vertical);
  auto* symbols_horizontal_splitter = new QSplitter(Qt::Horizontal);

  ui->functionsTabLayout->addWidget(symbols_vertical_splitter, 0, 0);

  symbols_vertical_splitter->addWidget(symbols_horizontal_splitter);
  symbols_vertical_splitter->addWidget(ui->FunctionsList);

  symbols_horizontal_splitter->addWidget(ui->SessionList);
  symbols_horizontal_splitter->addWidget(ui->ModulesList);

  // Make the splitters take 50% of the space.
  symbols_vertical_splitter->setSizes({5000, 5000});
  symbols_horizontal_splitter->setSizes({5000, 5000});

  DataViewFactory* data_view_factory = app_.get();
  ui->ModulesList->Initialize(data_view_factory->GetOrCreateDataView(DataViewType::kModules),
                              SelectionType::kExtended, FontType::kDefault);
  ui->FunctionsList->Initialize(data_view_factory->GetOrCreateDataView(DataViewType::kFunctions),
                                SelectionType::kExtended, FontType::kDefault);
  ui->SessionList->Initialize(data_view_factory->GetOrCreateDataView(DataViewType::kPresets),
                              SelectionType::kDefault, FontType::kDefault,
                              /*is_main_instance=*/true, /*uniform_row_height=*/false,
                              /*text_alignment=*/Qt::AlignTop | Qt::AlignLeft);

  std::visit([this](const auto& target) { SetTarget(target); }, target_configuration_);

  app_->PostInit();

  SaveCurrentTabLayoutAsDefaultInMemory();

  // TODO(177304549): This is still the way it is because of the old UI. Refactor:
  // As soon as PostInit() is cleaned up (see todo comments in PostInit), it should be called before
  // std::visit and then OpenCapture can be called inside SetTarget(FileTarget).
  std::string file_path_to_open;
  if (std::holds_alternative<orbit_qt::FileTarget>(target_configuration_)) {
    OpenCapture(
        std::get<orbit_qt::FileTarget>(target_configuration_).GetCaptureFilePath().string());
  }

  UpdateCaptureStateDependentWidgets();

  LoadCaptureOptionsIntoApp();
}

void OrbitMainWindow::SetupMainWindow(uint32_t font_size) {
  DataViewFactory* data_view_factory = app_.get();

  ui->setupUi(this);

  QList<int> sizes;
  sizes.append(5000);
  sizes.append(5000);
  ui->HomeVerticalSplitter->setSizes(sizes);
  ui->HomeHorizontalSplitter->setSizes(sizes);
  ui->splitter_2->setSizes(sizes);

  status_listener_ = StatusListenerImpl::Create(statusBar());

  app_->SetStatusListener(status_listener_.get());

  app_->SetCaptureStartedCallback([this] {
    UpdateCaptureStateDependentWidgets();
    setWindowTitle({});
  });

  constexpr const char* kFinalizingCaptureMessage =
      "<div align=\"left\">"
      "Please wait while the capture is being finalized..."
      "<ul>"
      "<li>Waiting for the remaining capture data</li>"
      "<li>Processing callstacks</li>"
      "<li>Cleaning up dynamic instrumentation</li>"
      "</ul>"
      "</div>";
  auto finalizing_capture_dialog =
      new QProgressDialog(kFinalizingCaptureMessage, "OK", 0, 0, this, Qt::Tool);
  finalizing_capture_dialog->setWindowTitle("Finalizing capture");
  finalizing_capture_dialog->setModal(true);
  finalizing_capture_dialog->setWindowFlags(
      (finalizing_capture_dialog->windowFlags() | Qt::CustomizeWindowHint) &
      ~Qt::WindowCloseButtonHint & ~Qt::WindowSystemMenuHint);
  finalizing_capture_dialog->setFixedSize(finalizing_capture_dialog->size());
  finalizing_capture_dialog->close();

  app_->SetCaptureStopRequestedCallback([this, finalizing_capture_dialog] {
    finalizing_capture_dialog->show();
    UpdateCaptureStateDependentWidgets();
  });
  auto capture_finished_callback = [this, finalizing_capture_dialog] {
    finalizing_capture_dialog->close();
    UpdateCaptureStateDependentWidgets();
  };
  app_->SetCaptureStoppedCallback(capture_finished_callback);
  app_->SetCaptureFailedCallback(capture_finished_callback);
  app_->SetCaptureClearedCallback([this] { OnCaptureCleared(); });

  auto loading_capture_dialog =
      new QProgressDialog("Waiting for the capture to be loaded...", nullptr, 0, 0, this, Qt::Tool);
  loading_capture_dialog->setWindowTitle("Loading capture");
  loading_capture_dialog->setModal(true);
  loading_capture_dialog->setWindowFlags(
      (loading_capture_dialog->windowFlags() | Qt::CustomizeWindowHint) &
      ~Qt::WindowCloseButtonHint & ~Qt::WindowSystemMenuHint);
  loading_capture_dialog->setFixedSize(loading_capture_dialog->size());

  auto loading_capture_cancel_button = QPointer{new QPushButton{this}};
  loading_capture_cancel_button->setText("Cancel");
  QObject::connect(loading_capture_cancel_button, &QPushButton::clicked, this,
                   [this, loading_capture_dialog]() {
                     app_->OnLoadCaptureCancelRequested();
                     loading_capture_dialog->close();
                   });
  loading_capture_dialog->setCancelButton(loading_capture_cancel_button);

  loading_capture_dialog->close();

  app_->SetOpenCaptureCallback([this, loading_capture_dialog] {
    setWindowTitle({});
    loading_capture_dialog->show();
  });
  app_->SetOpenCaptureFailedCallback([this, loading_capture_dialog] {
    setWindowTitle({});
    loading_capture_dialog->close();
    UpdateCaptureStateDependentWidgets();
  });
  app_->SetOpenCaptureFinishedCallback([this, loading_capture_dialog] {
    loading_capture_dialog->close();
    UpdateCaptureStateDependentWidgets();
  });

  app_->SetRefreshCallback([this](DataViewType type) {
    if (type == DataViewType::kAll || type == DataViewType::kLiveFunctions) {
      this->ui->liveFunctions->OnDataChanged();
    }
    this->OnRefreshDataViewPanels(type);
  });

  app_->SetSamplingReportCallback(
      [this](DataView* callstack_data_view, const std::shared_ptr<SamplingReport>& report) {
        this->OnNewSamplingReport(callstack_data_view, report);
      });

  app_->SetSelectionReportCallback(
      [this](DataView* callstack_data_view, const std::shared_ptr<SamplingReport>& report) {
        this->OnNewSelectionReport(callstack_data_view, report);
      });

  app_->SetTopDownViewCallback([this](std::unique_ptr<CallTreeView> top_down_view) {
    this->OnNewTopDownView(std::move(top_down_view));
  });

  app_->SetSelectionTopDownViewCallback(
      [this](std::unique_ptr<CallTreeView> selection_top_down_view) {
        this->OnNewSelectionTopDownView(std::move(selection_top_down_view));
      });

  app_->SetBottomUpViewCallback([this](std::unique_ptr<CallTreeView> bottom_up_view) {
    this->OnNewBottomUpView(std::move(bottom_up_view));
  });

  app_->SetSelectionBottomUpViewCallback(
      [this](std::unique_ptr<CallTreeView> selection_bottom_up_view) {
        this->OnNewSelectionBottomUpView(std::move(selection_bottom_up_view));
      });

  app_->SetSelectLiveTabCallback([this] { ui->RightTabWidget->setCurrentWidget(ui->liveTab); });
  app_->SetDisassemblyCallback(&OpenDisassembly);
  app_->SetErrorMessageCallback([this](const std::string& title, const std::string& text) {
    QMessageBox::critical(this, QString::fromStdString(title), QString::fromStdString(text));
  });
  app_->SetWarningMessageCallback([this](const std::string& title, const std::string& text) {
    QMessageBox::warning(this, QString::fromStdString(title), QString::fromStdString(text));
  });
  app_->SetInfoMessageCallback([this](const std::string& title, const std::string& text) {
    QMessageBox::information(this, QString::fromStdString(title), QString::fromStdString(text));
  });
  app_->SetTooltipCallback([this](const std::string& tooltip) {
    QToolTip::showText(QCursor::pos(), QString::fromStdString(tooltip), this);
  });
  app_->SetSaveFileCallback(
      [this](const std::string& extension) { return this->OnGetSaveFileName(extension); });
  app_->SetClipboardCallback([this](const std::string& text) { this->OnSetClipboard(text); });

  app_->SetShowEmptyFrameTrackWarningCallback(
      [this](std::string_view function) { this->ShowEmptyFrameTrackWarningIfNeeded(function); });

  ui->CaptureGLWidget->Initialize(GlCanvas::CanvasType::kCaptureWindow, this, font_size,
                                  app_.get());

  app_->SetTimerSelectedCallback([this](const orbit_client_protos::TimerInfo* timer_info) {
    OnTimerSelectionChanged(timer_info);
  });

  if (absl::GetFlag(FLAGS_devmode)) {
    ui->debugOpenGLWidget->Initialize(GlCanvas::CanvasType::kDebug, this, font_size, app_.get());
    app_->SetDebugCanvas(ui->debugOpenGLWidget->GetCanvas());
  } else {
    ui->RightTabWidget->removeTab(ui->RightTabWidget->indexOf(ui->debugTab));
  }

  ui->TracepointsList->Initialize(
      data_view_factory->GetOrCreateDataView(DataViewType::kTracepoints), SelectionType::kExtended,
      FontType::kDefault);

  if (!absl::GetFlag(FLAGS_enable_tracepoint_feature)) {
    ui->RightTabWidget->removeTab(ui->RightTabWidget->indexOf(ui->tracepointsTab));
  }

  if (!absl::GetFlag(FLAGS_devmode)) {
    ui->menuDebug->menuAction()->setVisible(false);
  }

  if (absl::GetFlag(FLAGS_enable_tutorials_feature)) {
    InitTutorials(this);
  }

  SetupCaptureToolbar();

  icon_keyboard_arrow_left_ = QIcon(":/actions/keyboard_arrow_left");
  icon_keyboard_arrow_right_ = QIcon(":/actions/keyboard_arrow_right");

  StartMainTimer();

  ui->liveFunctions->Initialize(app_.get(), SelectionType::kExtended, FontType::kDefault);

  connect(ui->liveFunctions->GetFilterLineEdit(), &QLineEdit::textChanged, this,
          [this](const QString& text) { OnLiveTabFunctionsFilterTextChanged(text); });

  ui->topDownWidget->Initialize(app_.get());
  ui->selectionTopDownWidget->Initialize(app_.get());
  ui->bottomUpWidget->Initialize(app_.get());
  ui->selectionBottomUpWidget->Initialize(app_.get());

  ui->MainTabWidget->tabBar()->installEventFilter(this);
  ui->RightTabWidget->tabBar()->installEventFilter(this);

  SetupAccessibleNamesForAutomation();

  setWindowTitle({});
  std::filesystem::path icon_file_name = (orbit_base::GetExecutableDir() / "orbit.ico");
  this->setWindowIcon(QIcon(QString::fromStdString(icon_file_name.string())));

  if (!absl::GetFlag(FLAGS_devmode)) {
    ui->actionIntrospection->setVisible(false);
  }
}

static QWidget* CreateSpacer(QWidget* parent) {
  auto* spacer = new QLabel(parent);
  spacer->setText("    ");
  return spacer;
}

void OrbitMainWindow::SetupCaptureToolbar() {
  // Sizes.
  QToolBar* toolbar = ui->capture_toolbar;

  // Create missing icons
  icon_start_capture_ = QIcon(":/actions/play_arrow");
  icon_stop_capture_ = QIcon(":/actions/stop");
  icon_toolbar_extension_ = QIcon(":/actions/double_arrows");

  // Attach the filter panel to the toolbar
  toolbar->addWidget(CreateSpacer(toolbar));
  filter_panel_action_ = new FilterPanelWidgetAction(toolbar);
  connect(filter_panel_action_, &FilterPanelWidgetAction::FilterTracksTextChanged, this,
          &OrbitMainWindow::OnFilterTracksTextChanged);
  connect(filter_panel_action_, &FilterPanelWidgetAction::FilterFunctionsTextChanged, this,
          &OrbitMainWindow::OnFilterFunctionsTextChanged);
  toolbar->addAction(filter_panel_action_);
  toolbar->findChild<QToolButton*>("qt_toolbar_ext_button")->setIcon(icon_toolbar_extension_);
}

void OrbitMainWindow::SetupHintFrame() {
  hint_frame_ = new QFrame();
  hint_frame_->setStyleSheet("background: transparent");
  auto* hint_layout = new QVBoxLayout();
  hint_layout->setSpacing(0);
  hint_layout->setMargin(0);
  hint_frame_->setLayout(hint_layout);
  auto* hint_arrow = new QLabel();
  hint_arrow->setPixmap(QPixmap(":/images/tutorial/grey_arrow_up.png").scaledToHeight(12));
  hint_layout->addWidget(hint_arrow);
  auto* hint_message = new QLabel("Start a capture here");
  hint_message->setAlignment(Qt::AlignCenter);
  hint_layout->addWidget(hint_message);
  hint_message->setStyleSheet(QString("background-color: %1;"
                                      "border-top-left-radius: 1px;"
                                      "border-top-right-radius: 4px;"
                                      "border-bottom-right-radius: 4px;"
                                      "border-bottom-left-radius: 4px;")
                                  .arg(kLightGrayColor));
  hint_layout->setStretchFactor(hint_message, 1);
  hint_frame_->setParent(ui->CaptureTab);

  hint_frame_->move(kHintFramePosX, kHintFramePosY);
  hint_frame_->resize(kHintFrameWidth, kHintFrameHeight);
}

void OrbitMainWindow::SetupTargetLabel() {
  auto* target_widget = new QWidget();
  target_widget->setStyleSheet(QString("background-color: %1").arg(kMediumGrayColor));
  target_label_ = new QLabel();
  target_label_->setContentsMargins(6, 0, 0, 0);
  target_label_->setObjectName("TargetLabel");
  auto* disconnect_target_button = new QPushButton("End Session");
  auto* target_layout = new QHBoxLayout();
  target_layout->addWidget(target_label_);
  target_layout->addWidget(disconnect_target_button);
  target_layout->setMargin(0);
  target_widget->setLayout(target_layout);

  ui->menuBar->setCornerWidget(target_widget, Qt::TopRightCorner);

  QObject::connect(disconnect_target_button, &QPushButton::clicked, this, [this]() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, QApplication::applicationName(),
        "This discards any unsaved progress. Are you sure you want to continue?");

    if (reply == QMessageBox::Yes) {
      QApplication::exit(kEndSessionReturnCode);
    }
  });
}

void OrbitMainWindow::SetupAccessibleNamesForAutomation() {
  for (QTabWidget* tab_widget : {ui->MainTabWidget, ui->RightTabWidget}) {
    for (int i = 0; i < tab_widget->count(); ++i) {
      tab_widget->widget(i)->setAccessibleName(tab_widget->widget(i)->objectName());
    }
  }
}

void OrbitMainWindow::SaveCurrentTabLayoutAsDefaultInMemory() {
  default_tab_layout_.clear();
  std::array<QTabWidget*, 2> tab_widgets = {ui->MainTabWidget, ui->RightTabWidget};
  for (QTabWidget* tab_widget : tab_widgets) {
    TabWidgetLayout layout = {};
    for (int i = 0; i < tab_widget->count(); ++i) {
      layout.tabs_and_titles.emplace_back(tab_widget->widget(i), tab_widget->tabText(i));
    }
    layout.current_index = tab_widget->currentIndex();
    default_tab_layout_[tab_widget] = layout;
  }
}

void OrbitMainWindow::CreateTabBarContextMenu(QTabWidget* tab_widget, int tab_index,
                                              const QPoint pos) {
  QMenu context_menu(this);
  context_menu.setAccessibleName("TabBarContextMenu");
  QAction move_action;
  QTabWidget* other_widget;

  if (tab_widget == ui->MainTabWidget) {
    move_action.setIcon(icon_keyboard_arrow_right_);
    move_action.setText(QString("Move \"") + tab_widget->tabText(tab_index) + "\" to right pane");
    other_widget = ui->RightTabWidget;
  } else if (tab_widget == ui->RightTabWidget) {
    move_action.setIcon(icon_keyboard_arrow_left_);
    move_action.setText(QString("Move \"") + tab_widget->tabText(tab_index) + "\" to left pane");
    other_widget = ui->MainTabWidget;
  } else {
    UNREACHABLE();
  }

  move_action.setEnabled(tab_widget->count() > 0);

  QObject::connect(&move_action, &QAction::triggered, [this, tab_widget, other_widget, tab_index] {
    QWidget* tab = tab_widget->widget(tab_index);
    QString text = tab_widget->tabText(tab_index);
    tab_widget->removeTab(tab_index);
    other_widget->addTab(tab, text);
    UpdateCaptureStateDependentWidgets();
    if (tab->isEnabled()) {
      other_widget->setCurrentWidget(tab);
    }
  });
  context_menu.addAction(&move_action);
  context_menu.exec(pos);
}

void OrbitMainWindow::UpdateCaptureStateDependentWidgets() {
  auto set_tab_enabled = [this](QWidget* widget, bool enabled) -> void {
    QTabWidget* tab_widget = FindParentTabWidget(widget);
    CHECK(tab_widget != nullptr);
    tab_widget->setTabEnabled(tab_widget->indexOf(widget), enabled);
  };

  const bool has_data = app_->HasCaptureData();
  const bool has_selection = has_data && app_->HasSampleSelection();
  const bool is_connected = app_->IsConnectedToInstance();
  CaptureClient::State capture_state = app_->GetCaptureState();
  const bool is_capturing = capture_state != CaptureClient::State::kStopped;
  const bool is_target_process_running = target_process_state_ == TargetProcessState::kRunning;

  set_tab_enabled(ui->FunctionsTab, true);
  set_tab_enabled(ui->CaptureTab, true);
  set_tab_enabled(ui->liveTab, has_data);
  set_tab_enabled(ui->samplingTab, has_data && !is_capturing);
  set_tab_enabled(ui->topDownTab, has_data && !is_capturing);
  set_tab_enabled(ui->bottomUpTab, has_data && !is_capturing);
  set_tab_enabled(ui->selectionSamplingTab, has_selection);
  set_tab_enabled(ui->selectionTopDownTab, has_selection);
  set_tab_enabled(ui->selectionBottomUpTab, has_selection);

  ui->actionToggle_Capture->setEnabled(
      capture_state == CaptureClient::State::kStarted ||
      (capture_state == CaptureClient::State::kStopped && is_target_process_running));
  ui->actionToggle_Capture->setIcon(is_capturing ? icon_stop_capture_ : icon_start_capture_);
  ui->actionClear_Capture->setEnabled(!is_capturing && has_data);
  ui->actionCaptureOptions->setEnabled(!is_capturing);
  ui->actionOpen_Capture->setEnabled(!is_capturing);
  ui->actionSave_Capture->setEnabled(!is_capturing && has_data);
  ui->actionOpen_Preset->setEnabled(!is_capturing && is_connected);
  ui->actionSave_Preset_As->setEnabled(!is_capturing);

  hint_frame_->setVisible(!has_data);

  // Gray out disabled actions on the capture toolbar.
  for (QAction* action : ui->capture_toolbar->actions()) {
    // setGraphicsEffect(effect) transfers the ownership of effect to the QWidget. If the effect is
    // installed on a different item, setGraphicsEffect() will remove the effect from the original
    // item and install it on this item.
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect;
    effect->setOpacity(action->isEnabled() ? 1 : 0.3);
    ui->capture_toolbar->widgetForAction(action)->setGraphicsEffect(effect);
  }
}

void OrbitMainWindow::UpdateProcessConnectionStateDependentWidgets() {
  CaptureClient::State capture_state = app_->GetCaptureState();
  const bool is_capturing = capture_state != CaptureClient::State::kStopped;
  const bool is_connected = app_->IsConnectedToInstance();
  const bool is_target_process_running = target_process_state_ == TargetProcessState::kRunning;

  ui->actionToggle_Capture->setEnabled(
      capture_state == CaptureClient::State::kStarted ||
      (capture_state == CaptureClient::State::kStopped && is_target_process_running));
  ui->actionOpen_Preset->setEnabled(!is_capturing && is_connected);
}

void OrbitMainWindow::UpdateActiveTabsAfterSelection(bool selection_has_samples) {
  const QTabWidget* capture_parent = FindParentTabWidget(ui->CaptureTab);

  // Automatically switch between (complete capture) report and selection report tabs
  // if applicable
  auto show_corresponding_selection_tab = [this, capture_parent, selection_has_samples](
                                              const std::vector<QWidget*>& report_tabs,
                                              QWidget* selection_tab) {
    QTabWidget* selection_parent = FindParentTabWidget(selection_tab);

    // If the capture window is in the same tab widget as the selection, do not change anything
    if (selection_parent == capture_parent) {
      return;
    }

    if (selection_has_samples) {
      // Non-empty selection: If one of the corresponding complete reports was visible,
      // show the selection tab instead
      if (std::find(report_tabs.begin(), report_tabs.end(), selection_parent->currentWidget()) !=
          report_tabs.end()) {
        selection_parent->setCurrentWidget(selection_tab);
      }
    } else {
      // Empty selection: If the selection tab was visible, switch back to the first complete report
      // that is in the same tab widget
      if (selection_parent->currentWidget() == selection_tab) {
        for (auto& report_tab : report_tabs) {
          QTabWidget* report_parent = FindParentTabWidget(report_tab);
          if (selection_parent == report_parent &&
              report_parent->isTabEnabled(report_parent->indexOf(report_tab))) {
            selection_parent->setCurrentWidget(report_tab);
            break;
          }
        }
      }
    }
  };

  show_corresponding_selection_tab({ui->samplingTab, ui->liveTab, ui->FunctionsTab},
                                   ui->selectionSamplingTab);
  show_corresponding_selection_tab({ui->topDownTab, ui->liveTab, ui->FunctionsTab},
                                   ui->selectionTopDownTab);
  show_corresponding_selection_tab({ui->bottomUpTab, ui->liveTab, ui->FunctionsTab},
                                   ui->selectionBottomUpTab);
}

QTabWidget* OrbitMainWindow::FindParentTabWidget(const QWidget* widget) const {
  std::array<QTabWidget*, 2> potential_parents = {ui->MainTabWidget, ui->RightTabWidget};
  for (QTabWidget* tab_widget : potential_parents) {
    for (int i = 0; i < tab_widget->count(); ++i) {
      if (tab_widget->widget(i) == widget) {
        return tab_widget;
      }
    }
  }

  return nullptr;
}

OrbitMainWindow::~OrbitMainWindow() {
  DeinitTutorials();

  ui->selectionBottomUpWidget->Deinitialize();
  ui->bottomUpWidget->Deinitialize();
  ui->selectionTopDownWidget->Deinitialize();
  ui->topDownWidget->Deinitialize();
  ui->TracepointsList->Deinitialize();
  ui->liveFunctions->Deinitialize();

  ui->samplingReport->Deinitialize();
  ui->selectionReport->Deinitialize();

  if (absl::GetFlag(FLAGS_devmode)) {
    ui->debugOpenGLWidget->Deinitialize(this);
  }

  ui->CaptureGLWidget->Deinitialize(this);
  ui->SessionList->Deinitialize();
  ui->FunctionsList->Deinitialize();
  ui->ModulesList->Deinitialize();

  delete ui;
}

void OrbitMainWindow::OnRefreshDataViewPanels(DataViewType a_Type) {
  if (a_Type == DataViewType::kAll) {
    for (int i = 0; i < static_cast<int>(DataViewType::kAll); ++i) {
      UpdatePanel(static_cast<DataViewType>(i));
    }
  } else {
    UpdatePanel(a_Type);
  }
}

void OrbitMainWindow::UpdatePanel(DataViewType a_Type) {
  switch (a_Type) {
    case DataViewType::kFunctions:
      ui->FunctionsList->Refresh();
      break;
    case DataViewType::kLiveFunctions:
      ui->liveFunctions->Refresh();
      break;
    case DataViewType::kModules:
      ui->ModulesList->Refresh();
      break;
    case DataViewType::kPresets:
      ui->SessionList->Refresh();
      break;
    case DataViewType::kSampling:
      ui->samplingReport->RefreshCallstackView();
      ui->samplingReport->RefreshTabs();
      ui->selectionReport->RefreshCallstackView();
      ui->selectionReport->RefreshTabs();
      break;
    default:
      break;
  }
}

void OrbitMainWindow::OnNewSamplingReport(DataView* callstack_data_view,
                                          const std::shared_ptr<SamplingReport>& sampling_report) {
  ui->samplingGridLayout->removeWidget(ui->samplingReport);
  delete ui->samplingReport;

  ui->samplingReport = new OrbitSamplingReport(ui->samplingTab);
  ui->samplingReport->Initialize(callstack_data_view, sampling_report);
  ui->samplingGridLayout->addWidget(ui->samplingReport, 0, 0, 1, 1);

  UpdateCaptureStateDependentWidgets();

  // Switch to sampling tab if:
  //  * Report is non-empty
  //  * Sampling-tab is not in the same widget as the capture tab
  //  * Live-tab isn't selected in the same widget as the sampling tab
  QTabWidget* sampling_tab_parent = FindParentTabWidget(ui->samplingTab);
  if (sampling_report->HasSamples() &&
      (FindParentTabWidget(ui->CaptureTab) != sampling_tab_parent) &&
      (sampling_tab_parent->currentWidget() != ui->liveTab)) {
    sampling_tab_parent->setCurrentWidget(ui->samplingTab);
  }
}

void OrbitMainWindow::OnNewSelectionReport(DataView* callstack_data_view,
                                           const std::shared_ptr<SamplingReport>& sampling_report) {
  ui->selectionGridLayout->removeWidget(ui->selectionReport);
  delete ui->selectionReport;
  bool has_samples = sampling_report->HasSamples();

  ui->selectionReport = new OrbitSamplingReport(ui->selectionSamplingTab);
  ui->selectionReport->Initialize(callstack_data_view, sampling_report);
  ui->selectionGridLayout->addWidget(ui->selectionReport, 0, 0, 1, 1);

  UpdateActiveTabsAfterSelection(has_samples);
  UpdateCaptureStateDependentWidgets();
}

void OrbitMainWindow::OnNewTopDownView(std::unique_ptr<CallTreeView> top_down_view) {
  ui->topDownWidget->SetTopDownView(std::move(top_down_view));
}

void OrbitMainWindow::OnNewSelectionTopDownView(
    std::unique_ptr<CallTreeView> selection_top_down_view) {
  ui->selectionTopDownWidget->SetTopDownView(std::move(selection_top_down_view));
}

void OrbitMainWindow::OnNewBottomUpView(std::unique_ptr<CallTreeView> bottom_up_view) {
  ui->bottomUpWidget->SetBottomUpView(std::move(bottom_up_view));
}

void OrbitMainWindow::OnNewSelectionBottomUpView(
    std::unique_ptr<CallTreeView> selection_bottom_up_view) {
  ui->selectionBottomUpWidget->SetBottomUpView(std::move(selection_bottom_up_view));
}

std::string OrbitMainWindow::OnGetSaveFileName(const std::string& extension) {
  std::string filename =
      QFileDialog::getSaveFileName(this, "Specify a file to save...", nullptr, extension.c_str())
          .toStdString();
  if (!filename.empty() && !absl::EndsWith(filename, extension)) {
    filename += extension;
  }
  return filename;
}

void OrbitMainWindow::OnSetClipboard(const std::string& text) {
  QApplication::clipboard()->setText(QString::fromStdString(text));
}

void OrbitMainWindow::on_actionReport_Missing_Feature_triggered() {
  if (!QDesktopServices::openUrl(
          QUrl("https://community.stadia.dev/s/feature-requests", QUrl::StrictMode))) {
    QMessageBox::critical(this, "Error opening URL",
                          "Could not open community.stadia.dev/s/feature-request");
  }
}

void OrbitMainWindow::on_actionReport_Bug_triggered() {
  if (!QDesktopServices::openUrl(
          QUrl("https://community.stadia.dev/s/contactsupport", QUrl::StrictMode))) {
    QMessageBox::critical(this, "Error opening URL",
                          "Could not open community.stadia.dev/s/contactsupport");
  }
}

void OrbitMainWindow::on_actionOpenUserDataDirectory_triggered() {
  std::string app_data_dir = Path::CreateOrGetOrbitAppDataDir().string();
  QUrl app_data_url = QUrl::fromLocalFile(QString::fromStdString(app_data_dir));
  if (!QDesktopServices::openUrl(app_data_url)) {
    QMessageBox::critical(this, "Error opening directory",
                          "Could not open Orbit user data directory");
  }
}

void OrbitMainWindow::on_actionAbout_triggered() {
  orbit_qt::OrbitAboutDialog dialog{this};
  dialog.setWindowTitle("About");
  dialog.SetVersionString(QCoreApplication::applicationVersion());
  dialog.SetBuildInformation(QString::fromStdString(orbit_core::GetBuildReport()));

  QFile licenseFile{QDir{QCoreApplication::applicationDirPath()}.filePath("NOTICE")};
  if (licenseFile.open(QIODevice::ReadOnly)) {
    dialog.SetLicenseText(licenseFile.readAll());
  }
  dialog.exec();
}

void OrbitMainWindow::StartMainTimer() {
  m_MainTimer = new QTimer(this);
  connect(m_MainTimer, SIGNAL(timeout()), this, SLOT(OnTimer()));

  // Update period set to 16ms (~60FPS)
  int msec = 16;
  m_MainTimer->start(msec);
}

void OrbitMainWindow::OnTimer() {
  ORBIT_SCOPE("OrbitMainWindow::OnTimer");
  app_->MainTick();

  for (OrbitGLWidget* gl_widget : gl_widgets_) {
    gl_widget->update();
  }

  filter_panel_action_->SetTimerLabelText(QString::fromStdString(app_->GetCaptureTime()));
}

void OrbitMainWindow::OnFilterFunctionsTextChanged(const QString& text) {
  // The toolbar and live tab filters are mirrored.
  ui->liveFunctions->SetFilter(text);
}

void OrbitMainWindow::OnLiveTabFunctionsFilterTextChanged(const QString& text) {
  // Set main toolbar functions filter without triggering signals.
  filter_panel_action_->SetFilterFunctionsText(text);
}

void OrbitMainWindow::OnFilterTracksTextChanged(const QString& text) {
  app_->FilterTracks(text.toStdString());
}

void OrbitMainWindow::on_actionOpen_Preset_triggered() {
  QStringList list = QFileDialog::getOpenFileNames(
      this, "Select a file to open...",
      QString::fromStdString(Path::CreateOrGetPresetDir().string()), "*.opr");
  for (const auto& file : list) {
    ErrorMessageOr<void> result = app_->OnLoadPreset(file.toStdString());
    if (result.has_error()) {
      QMessageBox::critical(this, "Error loading session",
                            absl::StrFormat("Could not load session from \"%s\":\n%s.",
                                            file.toStdString(), result.error().message())
                                .c_str());
    }
    break;
  }
}

void OrbitMainWindow::on_actionEnd_Session_triggered() {
  close();
  QApplication::exit(kEndSessionReturnCode);
}

void OrbitMainWindow::on_actionQuit_triggered() {
  close();
  QApplication::quit();
}

void OrbitMainWindow::on_actionSave_Preset_As_triggered() {
  QString file = QFileDialog::getSaveFileName(
      this, "Specify a file to save...",
      QString::fromStdString(Path::CreateOrGetPresetDir().string()), "*.opr");
  if (file.isEmpty()) {
    return;
  }

  ErrorMessageOr<void> result = app_->OnSavePreset(file.toStdString());
  if (result.has_error()) {
    QMessageBox::critical(this, "Error saving session",
                          absl::StrFormat("Could not save session in \"%s\":\n%s.",
                                          file.toStdString(), result.error().message())
                              .c_str());
  }
}

void OrbitMainWindow::on_actionToggle_Capture_triggered() { app_->ToggleCapture(); }

void OrbitMainWindow::on_actionClear_Capture_triggered() { app_->ClearCapture(); }

const QString OrbitMainWindow::kCollectThreadStatesSettingKey{"CollectThreadStates"};

void OrbitMainWindow::LoadCaptureOptionsIntoApp() {
  QSettings settings;
  app_->SetCollectThreadStates(settings.value(kCollectThreadStatesSettingKey, false).toBool());
}

void OrbitMainWindow::on_actionCaptureOptions_triggered() {
  QSettings settings;

  orbit_qt::CaptureOptionsDialog dialog{this};
  dialog.SetCollectThreadStates(settings.value(kCollectThreadStatesSettingKey, false).toBool());

  int result = dialog.exec();
  if (result != QDialog::Accepted) {
    return;
  }

  settings.setValue(kCollectThreadStatesSettingKey, dialog.GetCollectThreadStates());
  LoadCaptureOptionsIntoApp();
}

void OrbitMainWindow::on_actionHelp_triggered() { app_->ToggleDrawHelp(); }

void OrbitMainWindow::on_actionIntrospection_triggered() {
  if (introspection_widget_ == nullptr) {
    introspection_widget_ = new OrbitGLWidget();
    introspection_widget_->setWindowFlags(Qt::WindowStaysOnTopHint);
    introspection_widget_->Initialize(GlCanvas::CanvasType::kIntrospectionWindow, this, 14,
                                      app_.get());
    introspection_widget_->installEventFilter(this);
  }

  introspection_widget_->show();
}

void OrbitMainWindow::ShowCaptureOnSaveWarningIfNeeded() {
  QSettings settings;
  const QString skip_capture_warning("SkipCaptureVersionWarning");
  if (!settings.value(skip_capture_warning, false).toBool()) {
    QMessageBox message_box(this);
    message_box.setText(
        "Note: Captures saved with this version of Orbit might be incompatible "
        "with future versions. Please check release notes for more "
        "information");
    message_box.addButton(QMessageBox::Ok);
    QCheckBox check_box("Don't show this message again.");
    message_box.setCheckBox(&check_box);

    QObject::connect(&check_box, &QCheckBox::stateChanged,
                     [&settings, &skip_capture_warning](int state) {
                       settings.setValue(skip_capture_warning, static_cast<bool>(state));
                     });

    message_box.exec();
  }
}

void OrbitMainWindow::ShowEmptyFrameTrackWarningIfNeeded(std::string_view function) {
  QSettings settings;
  const QString empty_frame_track_warning("EmptyFrameTrackWarning");
  std::string message = absl::StrFormat(
      "Frame track enabled for function %s, but since the function "
      "does not have any hits in the current capture, a frame track "
      "was not added to the capture.",
      function);
  if (!settings.value(empty_frame_track_warning, false).toBool()) {
    QMessageBox message_box(this);
    message_box.setText(message.c_str());
    message_box.addButton(QMessageBox::Ok);
    QCheckBox check_box("Don't show this message again.");
    message_box.setCheckBox(&check_box);

    QObject::connect(&check_box, &QCheckBox::stateChanged,
                     [&settings, &empty_frame_track_warning](int state) {
                       settings.setValue(empty_frame_track_warning, static_cast<bool>(state));
                     });

    message_box.exec();
  }
}

void OrbitMainWindow::RestoreDefaultTabLayout() {
  for (auto& widget_and_layout : default_tab_layout_) {
    QTabWidget* tab_widget = widget_and_layout.first;
    tab_widget->clear();
    for (auto& tab_and_title : widget_and_layout.second.tabs_and_titles) {
      tab_widget->addTab(tab_and_title.first, tab_and_title.second);
    }
    tab_widget->setCurrentIndex(widget_and_layout.second.current_index);
  }

  UpdateCaptureStateDependentWidgets();
}

void OrbitMainWindow::OnTimerSelectionChanged(const orbit_client_protos::TimerInfo* timer_info) {
  std::optional<int> selected_row(std::nullopt);
  if (timer_info) {
    uint64_t function_id = timer_info->function_id();
    const auto live_functions_controller = ui->liveFunctions->GetLiveFunctionsController();
    CHECK(live_functions_controller.has_value());
    LiveFunctionsDataView& live_functions_data_view =
        live_functions_controller.value()->GetDataView();
    selected_row = live_functions_data_view.GetRowFromFunctionId(function_id);
    live_functions_data_view.UpdateSelectedFunctionId();
  }
  ui->liveFunctions->OnRowSelected(selected_row);
}

void OrbitMainWindow::on_actionSave_Capture_triggered() {
  ShowCaptureOnSaveWarningIfNeeded();

  if (!app_->HasCaptureData()) {
    QMessageBox::information(this, "Save capture", "Looks like there is no capture to save.");
    return;
  }

  const CaptureData& capture_data = app_->GetCaptureData();
  QString file = QFileDialog::getSaveFileName(
      this, "Save capture...",
      QString::fromStdString(
          (Path::CreateOrGetCaptureDir() / capture_serializer::GetCaptureFileName(capture_data))
              .string()),
      "*.orbit");
  if (file.isEmpty()) {
    return;
  }

  ErrorMessageOr<void> result = app_->OnSaveCapture(file.toStdString());
  if (result.has_error()) {
    QMessageBox::critical(this, "Error saving capture",
                          absl::StrFormat("Could not save capture in \"%s\":\n%s.",
                                          file.toStdString(), result.error().message())
                              .c_str());
  }
}

void OrbitMainWindow::on_actionOpen_Capture_triggered() {
  QString file = QFileDialog::getOpenFileName(
      this, "Open capture...", QString::fromStdString(Path::CreateOrGetCaptureDir().string()),
      "*.orbit");
  if (file.isEmpty()) {
    return;
  }

  OpenCapture(file.toStdString());
}

void OrbitMainWindow::OpenCapture(const std::string& filepath) {
  app_->OnLoadCapture(filepath);
  setWindowTitle(QString::fromStdString(filepath));
  UpdateCaptureStateDependentWidgets();
  FindParentTabWidget(ui->CaptureTab)->setCurrentWidget(ui->CaptureTab);
}

void OrbitMainWindow::on_actionCheckFalse_triggered() { CHECK(false); }

void OrbitMainWindow::on_actionNullPointerDereference_triggered() {
  int* null_pointer = nullptr;
  *null_pointer = 0;
}

void InfiniteRecursion(int num) {
  if (num != 1) {
    InfiniteRecursion(num);
  }

  LOG("num=%d", num);
}

void OrbitMainWindow::on_actionStackOverflow_triggered() { InfiniteRecursion(0); }

void OrbitMainWindow::on_actionServiceCheckFalse_triggered() {
  app_->CrashOrbitService(CrashOrbitServiceRequest_CrashType_CHECK_FALSE);
}

void OrbitMainWindow::on_actionServiceNullPointerDereference_triggered() {
  app_->CrashOrbitService(CrashOrbitServiceRequest_CrashType_NULL_POINTER_DEREFERENCE);
}

void OrbitMainWindow::on_actionServiceStackOverflow_triggered() {
  app_->CrashOrbitService(CrashOrbitServiceRequest_CrashType_STACK_OVERFLOW);
}

void OrbitMainWindow::OnCaptureCleared() {
  ui->liveFunctions->Reset();
  UpdateCaptureStateDependentWidgets();
}

bool OrbitMainWindow::eventFilter(QObject* watched, QEvent* event) {
  if (watched == ui->MainTabWidget->tabBar() || watched == ui->RightTabWidget->tabBar()) {
    if (event->type() == QEvent::MouseButtonRelease) {
      QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
      if (mouse_event->button() == Qt::MouseButton::RightButton) {
        int index = static_cast<QTabBar*>(watched)->tabAt(mouse_event->pos());
        if (index >= 0) {
          auto tab_widget = static_cast<QTabWidget*>(watched->parent());
          if (tab_widget->isTabEnabled(index)) {
            tab_widget->setCurrentIndex(index);
          }
          CreateTabBarContextMenu(tab_widget, index, mouse_event->globalPos());
        }
      }
    }
  } else if (watched == introspection_widget_) {
    if (event->type() == QEvent::Close) {
      app_->StopIntrospection();
    }
  }

  return QObject::eventFilter(watched, event);
}

void OrbitMainWindow::closeEvent(QCloseEvent* event) {
  if (app_->IsCapturing()) {
    event->ignore();

    if (QMessageBox::question(this, "Capture in progress",
                              "A capture is currently in progress. Do you want to abort the "
                              "capture and exit Orbit?") == QMessageBox::Yes) {
      // We need for the capture to clean up - close as soon as this is done
      app_->SetCaptureFailedCallback([&] { close(); });
      app_->AbortCapture();
    }
  } else {
    if (main_thread_executor_) {
      main_thread_executor_->AbortWaitingJobs();
    }
    QMainWindow::closeEvent(event);
  }
}

void OrbitMainWindow::SetTarget(const orbit_qt::StadiaTarget& target) {
  const orbit_qt::StadiaConnection* connection = target.GetConnection();
  orbit_qt::ServiceDeployManager* service_deploy_manager = connection->GetServiceDeployManager();
  app_->SetSecureCopyCallback([service_deploy_manager](std::string_view source,
                                                       std::string_view destination) {
    CHECK(service_deploy_manager != nullptr);
    return service_deploy_manager->CopyFileToLocal(std::string{source}, std::string{destination});
  });
  app_->SetGrpcChannel(connection->GetGrpcChannel());
  app_->SetProcessManager(target.GetProcessManager());
  app_->SetTargetProcess(target.GetProcess());

  target_label_->setStyleSheet(kTargetLabelDefaultStyleSheet.arg(kTargetLabelColorConnected));
  target_label_->setText(QString::fromStdString(target.GetProcess()->name()) + " @ " +
                         target.GetConnection()->GetInstance().display_name);

  using ProcessInfo = orbit_grpc_protos::ProcessInfo;
  target.GetProcessManager()->SetProcessListUpdateListener([&](std::vector<ProcessInfo> processes) {
    // This lambda is called from a background-thread, so we use QMetaObject::invokeMethod
    // to execute our logic on the main thread.
    QMetaObject::invokeMethod(
        this, [&, processes = std::move(processes)]() { OnProcessListUpdated(processes); });
  });
}

void OrbitMainWindow::SetTarget(const orbit_qt::LocalTarget& target) {
  const orbit_qt::LocalConnection* connection = target.GetConnection();
  app_->SetGrpcChannel(connection->GetGrpcChannel());
  app_->SetProcessManager(target.GetProcessManager());
  app_->SetTargetProcess(target.GetProcess());

  target_label_->setStyleSheet(kTargetLabelDefaultStyleSheet.arg(kTargetLabelColorConnected));
  target_label_->setText(QString("Local target: ") +
                         QString::fromStdString(target.GetProcess()->name()));

  using ProcessInfo = orbit_grpc_protos::ProcessInfo;
  target.GetProcessManager()->SetProcessListUpdateListener([&](std::vector<ProcessInfo> processes) {
    // This lambda is called from a background-thread, so we use QMetaObject::invokeMethod
    // to execute our logic on the main thread.
    QMetaObject::invokeMethod(
        this, [&, processes = std::move(processes)]() { OnProcessListUpdated(processes); });
  });
}

void OrbitMainWindow::SetTarget(const orbit_qt::FileTarget& target) {
  target_label_->setStyleSheet(kTargetLabelColorFileTarget.arg(kTargetLabelColorFileTarget));
  target_label_->setText(QString::fromStdString(target.GetCaptureFilePath().filename().string()));
}

void OrbitMainWindow::OnProcessListUpdated(
    const std::vector<orbit_grpc_protos::ProcessInfo>& processes) {
  const auto is_current_process = [this](const auto& process) {
    const ProcessData* const target_process = app_->GetTargetProcess();
    return target_process != nullptr && process.pid() == app_->GetTargetProcess()->pid();
  };
  const auto current_process = std::find_if(processes.begin(), processes.end(), is_current_process);
  const bool process_died = current_process == processes.end();

  if (process_died) {
    target_label_->setStyleSheet(
        kTargetLabelDefaultStyleSheet.arg(kTargetLabelColorTargetProcessDied));
    target_label_->setToolTip("The process ended on the instance");
    target_process_state_ = TargetProcessState::kEnded;
  } else {
    target_label_->setStyleSheet(kTargetLabelDefaultStyleSheet.arg(kTargetLabelColorConnected));
    target_label_->setToolTip({});
    target_process_state_ = TargetProcessState::kRunning;
  }
  UpdateProcessConnectionStateDependentWidgets();
}

orbit_qt::TargetConfiguration OrbitMainWindow::ClearTargetConfiguration() {
  if (std::holds_alternative<orbit_qt::StadiaTarget>(target_configuration_)) {
    std::get<orbit_qt::StadiaTarget>(target_configuration_)
        .GetProcessManager()
        ->SetProcessListUpdateListener(nullptr);
  } else if (std::holds_alternative<orbit_qt::LocalTarget>(target_configuration_)) {
    std::get<orbit_qt::LocalTarget>(target_configuration_)
        .GetProcessManager()
        ->SetProcessListUpdateListener(nullptr);
  }
  return std::move(target_configuration_);
}