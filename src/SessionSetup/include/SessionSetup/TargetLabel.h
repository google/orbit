// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_TARGET_LABEL_H_
#define SESSION_SETUP_TARGET_LABEL_H_

#include <QColor>
#include <QObject>
#include <QString>
#include <QWidget>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "ClientData/ProcessData.h"
#include "GrpcProtos/process.pb.h"
#include "TargetConfiguration.h"

namespace Ui {
class TargetLabel;  // IWYU pragma: keep
}

namespace orbit_session_setup {

class TargetLabel : public QWidget {
  Q_OBJECT
 public:
  enum class IconType { kGreenConnectedIcon, kOrangeDisconnectedIcon, kRedDisconnectedIcon };
  explicit TargetLabel(QWidget* parent = nullptr);
  ~TargetLabel() override;

  void ChangeToFileTarget(const FileTarget& file_target);
  void ChangeToFileTarget(const std::filesystem::path& path);
  void ChangeToSshTarget(const SshTarget& ssh_target);
  void ChangeToSshTarget(const orbit_client_data::ProcessData& process,
                         std::string_view ssh_target_id);
  void ChangeToLocalTarget(const LocalTarget& local_target);
  void ChangeToLocalTarget(const orbit_client_data::ProcessData& process);
  void ChangeToLocalTarget(const orbit_grpc_protos::ProcessInfo& process_info);
  void ChangeToLocalTarget(const QString& process_name, double cpu_usage);

  bool SetProcessCpuUsageInPercent(double cpu_usage);
  bool SetProcessEnded();
  bool SetConnectionDead(const QString& error_message);
  void SetFile(const std::filesystem::path& file_path);

  void Clear();

  [[nodiscard]] QColor GetTargetColor() const;
  [[nodiscard]] QString GetTargetText() const;
  [[nodiscard]] QString GetFileText() const;
  [[nodiscard]] QString GetToolTip() const { return toolTip(); }
  [[nodiscard]] const std::optional<IconType>& GetIconType() const { return icon_type_; }
  [[nodiscard]] const std::optional<std::filesystem::path>& GetFilePath() const {
    return file_path_;
  }

 signals:
  void SizeChanged();

 private:
  std::unique_ptr<Ui::TargetLabel> ui_;
  QString process_;
  QString machine_;
  std::optional<IconType> icon_type_;
  std::optional<std::filesystem::path> file_path_;

  void SetColor(const QColor& color);
  void SetIcon(IconType icon_type);
  void ClearIcon();
  void OpenContainingFolder();
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_TARGET_LABEL_H_
