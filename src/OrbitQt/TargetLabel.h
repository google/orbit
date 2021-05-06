// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_TARGET_LABEL_H_
#define ORBIT_QT_TARGET_LABEL_H_

#include <QObject>
#include <QWidget>
#include <memory>
#include <optional>

#include "ClientData/ProcessData.h"
#include "OrbitGgp/Instance.h"
#include "TargetConfiguration.h"

namespace Ui {
class TargetLabel;  // IWYU pragma: keep
}

namespace orbit_qt {

class TargetLabel : public QWidget {
  Q_OBJECT
 public:
  enum class IconType { kGreenConnectedIcon, kOrangeDisconnectedIcon, kRedDisconnectedIcon };
  explicit TargetLabel(QWidget* parent = nullptr);
  ~TargetLabel() override;

  void ChangeToFileTarget(const FileTarget& file_target);
  void ChangeToFileTarget(const std::filesystem::path& path);
  void ChangeToStadiaTarget(const StadiaTarget& stadia_target);
  void ChangeToStadiaTarget(const orbit_client_data::ProcessData& process,
                            const orbit_ggp::Instance& instance);
  void ChangeToStadiaTarget(const QString& process_name, double cpu_usage,
                            const QString& instance_name);
  void ChangeToLocalTarget(const LocalTarget& local_target);
  void ChangeToLocalTarget(const orbit_client_data::ProcessData& process);
  void ChangeToLocalTarget(const QString& process_name, double cpu_usage);

  bool SetProcessCpuUsageInPercent(double cpu_usage);
  bool SetProcessEnded();
  bool SetConnectionDead(const QString& error_message);

  void Clear();

  [[nodiscard]] QColor GetColor() const;
  [[nodiscard]] QString GetText() const;
  [[nodiscard]] QString GetToolTip() const { return toolTip(); }
  [[nodiscard]] const std::optional<IconType>& GetIconType() const { return icon_type_; }

 signals:
  void SizeChanged();

 private:
  std::unique_ptr<Ui::TargetLabel> ui_;
  QString process_;
  QString machine_;
  std::optional<IconType> icon_type_;

  void SetColor(const QColor& color);
  void SetIcon(IconType icon_type);
  void ClearIcon();
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_TARGET_LABEL_H_
