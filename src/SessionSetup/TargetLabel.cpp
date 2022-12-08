// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/TargetLabel.h"

#include <QAction>
#include <QDesktopServices>
#include <QIcon>
#include <QImage>
#include <QLabel>
#include <QMenu>
#include <QPalette>
#include <QPixmap>
#include <QPoint>
#include <QUrl>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/AddrAndPort.h"
#include "SessionSetup/Connections.h"
#include "SessionSetup/DoubleClickableLabel.h"
#include "SessionSetup/TargetConfiguration.h"
#include "ui_TargetLabel.h"

namespace {
const QColor kDefaultTextColor{"white"};
const QColor kGreenColor{"#66BB6A"};
const QColor kOrangeColor{"orange"};
const QColor kRedColor{"#E64646"};
const QString kLocalhostName{"localhost"};

QPixmap ColorizeIcon(const QPixmap& pixmap, const QColor& color) {
  QImage colored_image = pixmap.toImage();

  for (int y = 0; y < colored_image.height(); y++) {
    for (int x = 0; x < colored_image.width(); x++) {
      QColor color_with_alpha = color;
      color_with_alpha.setAlpha(colored_image.pixelColor(x, y).alpha());
      colored_image.setPixelColor(x, y, color_with_alpha);
    }
  }

  return QPixmap::fromImage(std::move(colored_image));
}

QPixmap GetGreenConnectedIcon() {
  const static QPixmap green_connected_icon =
      ColorizeIcon(QPixmap{":/actions/connected"}, kGreenColor);
  return green_connected_icon;
}

QPixmap GetOrangeDisconnectedIcon() {
  const static QPixmap orange_disconnected_icon =
      ColorizeIcon(QPixmap{":/actions/alert"}, kOrangeColor);
  return orange_disconnected_icon;
}

QPixmap GetRedDisconnectedIcon() {
  const static QPixmap red_disconnected_icon =
      ColorizeIcon(QPixmap{":/actions/disconnected"}, kRedColor);
  return red_disconnected_icon;
}

}  // namespace

namespace orbit_session_setup {

namespace fs = std::filesystem;

TargetLabel::TargetLabel(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::TargetLabel>()) {
  ui_->setupUi(this);

  QObject::connect(ui_->fileLabel, &DoubleClickableLabel::DoubleClicked, this,
                   &TargetLabel::OpenContainingFolder);

  QObject::connect(ui_->fileLabel, &DoubleClickableLabel::customContextMenuRequested, this,
                   [this](const QPoint& pos) {
                     QAction action{QIcon(":/actions/folder"), "Open Containing Folder", this};
                     QObject::connect(&action, &QAction::triggered, this,
                                      &TargetLabel::OpenContainingFolder);

                     QMenu menu;
                     menu.addAction(&action);
                     menu.exec(mapToGlobal(pos));
                   });
}

TargetLabel::~TargetLabel() = default;

void TargetLabel::ChangeToFileTarget(const FileTarget& file_target) {
  ChangeToFileTarget(file_target.GetCaptureFilePath());
}

void TargetLabel::SetFile(const std::filesystem::path& file_path) {
  std::optional<std::filesystem::path> old_path = file_path_;

  file_path_ = file_path;
  ui_->fileLabel->setText(QString::fromStdString(file_path.filename().string()));
  ui_->fileLabel->setToolTip(QString::fromStdString(file_path.string()));
  ui_->fileLabel->setVisible(true);

  // Without this, the size of the target label is not correctly updated when a capture is opened
  // directly from the connection window. It's unclear why this happens, might be related to
  // multiple `SizeChanged` signals being emitted while the main window is still being shown.
  if (!old_path.has_value() || (old_path->filename() != file_path.filename())) {
    emit SizeChanged();
  }
}

void TargetLabel::ChangeToFileTarget(const fs::path& path) {
  Clear();
  SetFile(path);
  ui_->targetLabel->setVisible(false);
  setAccessibleName("File target");
  emit SizeChanged();
}

void TargetLabel::ChangeToSshTarget(const SshTarget& ssh_target) {
  ChangeToSshTarget(*ssh_target.GetProcess(),
                    ssh_target.GetConnection()->GetAddrAndPort().GetHumanReadable());
}

void TargetLabel::ChangeToSshTarget(const orbit_client_data::ProcessData& process,
                                    std::string_view ssh_target_id) {
  Clear();
  process_ = QString::fromStdString(process.name());
  machine_ = QString::fromUtf8(ssh_target_id.data(), ssh_target_id.size());
  SetProcessCpuUsageInPercent(process.cpu_usage());
  ui_->targetLabel->setVisible(true);
  ui_->fileLabel->setVisible(false);

  setToolTip(
      QString{"Connection active.<br/><br/>"
              "Machine: %1<br/>"
              "Process: %2 (%3)"}
          .arg(machine_, process_, QString::fromStdString(process.full_path())));
  setAccessibleName("Ssh target");
}

void TargetLabel::ChangeToLocalTarget(const LocalTarget& local_target) {
  ChangeToLocalTarget(*local_target.GetProcess());
}

void TargetLabel::ChangeToLocalTarget(const orbit_client_data::ProcessData& process) {
  ChangeToLocalTarget(QString::fromStdString(process.name()), process.cpu_usage());
}

void TargetLabel::ChangeToLocalTarget(const orbit_grpc_protos::ProcessInfo& process_info) {
  ChangeToLocalTarget(QString::fromStdString(process_info.name()), process_info.cpu_usage());
}

void TargetLabel::ChangeToLocalTarget(const QString& process_name, double cpu_usage) {
  Clear();
  process_ = process_name;
  machine_ = kLocalhostName;
  SetProcessCpuUsageInPercent(cpu_usage);
  ui_->targetLabel->setVisible(true);
  ui_->fileLabel->setVisible(false);
  setAccessibleName("Local target");
}

bool TargetLabel::SetProcessCpuUsageInPercent(double cpu_usage) {
  if (process_.isEmpty() || machine_.isEmpty()) return false;

  ui_->targetLabel->setText(
      QString{"%1 (%2%) @ %3"}.arg(process_).arg(cpu_usage, 0, 'f', 0).arg(machine_));
  SetColor(kGreenColor);
  SetIcon(IconType::kGreenConnectedIcon);
  emit SizeChanged();
  return true;
}

bool TargetLabel::SetProcessEnded() {
  if (process_.isEmpty() || machine_.isEmpty()) return false;

  ui_->targetLabel->setText(QString{"%1 @ %2"}.arg(process_, machine_));
  SetColor(kOrangeColor);
  setToolTip("The process ended.");
  SetIcon(IconType::kOrangeDisconnectedIcon);
  emit SizeChanged();
  return true;
}

bool TargetLabel::SetConnectionDead(const QString& error_message) {
  if (process_.isEmpty() || machine_.isEmpty()) return false;

  ui_->targetLabel->setText(QString{"%1 @ %2"}.arg(process_, machine_));
  SetColor(kRedColor);
  setToolTip(error_message);
  SetIcon(IconType::kRedDisconnectedIcon);
  emit SizeChanged();
  return true;
}

void TargetLabel::Clear() {
  process_ = "";
  machine_ = "";
  file_path_.reset();
  ui_->fileLabel->setText({});
  ui_->targetLabel->setText({});
  ui_->fileLabel->setVisible(false);
  ui_->targetLabel->setVisible(false);
  SetColor(kDefaultTextColor);
  setToolTip({});
  ClearIcon();
  emit SizeChanged();
}

QColor TargetLabel::GetTargetColor() const {
  return ui_->targetLabel->palette().color(QPalette::WindowText);
}

QString TargetLabel::GetTargetText() const { return ui_->targetLabel->text(); }

QString TargetLabel::GetFileText() const { return ui_->fileLabel->text(); }

void TargetLabel::SetColor(const QColor& color) {
  // This class is used in a QFrame and in QMenuBar. To make the coloring work in a QFrame the
  // QColorRole QPalette::WindowText needs to be set. For QMenuBar QPalette::ButtonText needs to be
  // set.
  QPalette palette{};
  palette.setColor(QPalette::WindowText, color);
  palette.setColor(QPalette::ButtonText, color);
  ui_->targetLabel->setPalette(palette);
}

void TargetLabel::SetIcon(IconType icon_type) {
  icon_type_ = icon_type;
  switch (icon_type) {
    case IconType::kGreenConnectedIcon:
      ui_->iconLabel->setPixmap(GetGreenConnectedIcon());
      break;
    case IconType::kOrangeDisconnectedIcon:
      ui_->iconLabel->setPixmap(GetOrangeDisconnectedIcon());
      break;
    case IconType::kRedDisconnectedIcon:
      ui_->iconLabel->setPixmap(GetRedDisconnectedIcon());
      break;
  }
  ui_->iconLabel->setVisible(true);
}

void TargetLabel::ClearIcon() {
  icon_type_ = std::nullopt;
  ui_->iconLabel->setPixmap(QPixmap{});
  ui_->iconLabel->setVisible(false);
}

void TargetLabel::OpenContainingFolder() {
  if (!file_path_.has_value()) return;
  QUrl url = QUrl::fromLocalFile(QString::fromStdString(file_path_->parent_path().string()));
  if (!QDesktopServices::openUrl(url)) {
    ORBIT_ERROR("Opening containing folder of \"%s\"", file_path_->string());
  }
}

}  // namespace orbit_session_setup
