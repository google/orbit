// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_PROCESS_LIST_WIDGET_H_
#define SESSION_SETUP_PROCESS_LIST_WIDGET_H_

#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QWidget>
#include <memory>
#include <optional>

#include "GrpcProtos/process.pb.h"
#include "SessionSetup/ProcessItemModel.h"

namespace Ui {
class ProcessListWidget;
}

namespace orbit_session_setup {

class ProcessListWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ProcessListWidget(QWidget* parent = nullptr);
  ~ProcessListWidget() noexcept override;

  void SetNameToSelect(std::string name) { name_to_select_ = std::move(name); }
  std::optional<orbit_grpc_protos::ProcessInfo> GetSelectedProcess();

 public slots:
  void Clear();
  void UpdateList(std::vector<orbit_grpc_protos::ProcessInfo> list);

 signals:
  void NoSelection();
  void ProcessSelected(orbit_grpc_protos::ProcessInfo process_info);
  // `ProcessConfirmed` is emitted when the user confirms the selection from the widget via a double
  // click onto the selected process.
  void ProcessConfirmed();

 private:
  void SelectionChanged(const QModelIndex& index);
  bool TrySelectProcessByName(const std::string& process_name);
  void TryConfirm();

  std::unique_ptr<Ui::ProcessListWidget> ui_;
  ProcessItemModel model_;
  QSortFilterProxyModel proxy_model_;
  std::string name_to_select_;
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_PROCESS_LIST_WIDGET_H_