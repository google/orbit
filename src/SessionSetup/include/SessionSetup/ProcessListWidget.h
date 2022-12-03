// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_PROCESS_LIST_WIDGET_H_
#define SESSION_SETUP_PROCESS_LIST_WIDGET_H_

#include <QMetaType>
#include <QModelIndex>
#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>
#include <QVector>
#include <QWidget>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

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

  // `SetProcessNameToSelect` saves a process name that will be selected after the process list is
  // filled for the first time (via `UpdateList`). This selection also happens when the process list
  // was cleared (via `Clear`) and is then updated (via `UpdateList`). A call to
  // `SetProcessNameToSelect` will not clear the selection a user made via the UI.
  void SetProcessNameToSelect(std::string name) { name_to_select_ = std::move(name); }
  [[nodiscard]] std::optional<orbit_grpc_protos::ProcessInfo> GetSelectedProcess() const;

  void Clear();
  void UpdateList(QVector<orbit_grpc_protos::ProcessInfo> list);

 signals:
  void ProcessSelectionCleared();
  // `ProcessSelected` is emitted when the user selects a process in the UI and after each call to
  // `UpdateList` when a valid selection exists.
  void ProcessSelected(orbit_grpc_protos::ProcessInfo process_info);
  // `ProcessConfirmed` is emitted when the user confirms the selection from the widget via a double
  // click onto the selected process.
  void ProcessConfirmed(orbit_grpc_protos::ProcessInfo process_info);

 private:
  void HandleSelectionChanged(const QModelIndex& index);
  bool TrySelectProcessByName(std::string_view process_name);
  void TryConfirm();

  std::unique_ptr<Ui::ProcessListWidget> ui_;
  ProcessItemModel model_;
  QSortFilterProxyModel proxy_model_;
  std::string name_to_select_;
};

}  // namespace orbit_session_setup

Q_DECLARE_METATYPE(orbit_grpc_protos::ProcessInfo);

#endif  // SESSION_SETUP_PROCESS_LIST_WIDGET_H_