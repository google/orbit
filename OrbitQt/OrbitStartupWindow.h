// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_STARTUP_WINDOW_H_
#define ORBIT_QT_ORBIT_STARTUP_WINDOW_H_

#include <QDialog>
#include <QPointer>
#include <QPushButton>
#include <QString>
#include <QWidget>
#include <optional>
#include <outcome.hpp>
#include <system_error>
#include <variant>

#include "Error.h"
#include "OrbitBase/Logging.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/InstanceItemModel.h"
#include "OrbitGgp/SshInfo.h"

namespace OrbitQt {

class OrbitStartupWindow : public QDialog {
  using Client = OrbitGgp::Client;
  using Instance = OrbitGgp::Instance;
  using InstanceItemModel = OrbitGgp::InstanceItemModel;
  using SshInfo = OrbitGgp::SshInfo;

 public:
  explicit OrbitStartupWindow(QWidget* parent = nullptr);

  template <typename Credentials>
  outcome::result<std::variant<Credentials, QString>> Run() {
    OUTCOME_TRY(create_result, Client::Create(this));
    ggp_client_ = std::move(create_result);

    ReloadInstances();

    result_ = std::monostate{};
    const int dialog_result = exec();

    if (dialog_result == 0) {
      return Error::kUserClosedStartUpWindow;
    }

    if (std::holds_alternative<SshInfo>(result_)) {
      auto& ssh_info = std::get<SshInfo>(result_);
      Credentials credentials{};
      credentials.addr_and_port = {ssh_info.host.toStdString(), ssh_info.port};
      credentials.key_path = ssh_info.key_path.toStdString();
      credentials.known_hosts_path = ssh_info.known_hosts_path.toStdString();
      credentials.user = ssh_info.user.toStdString();
      return outcome::success(std::move(credentials));
    } else if (std::holds_alternative<QString>(result_)) {
      return outcome::success(std::get<QString>(result_));
    } else {
      UNREACHABLE();
    }
  }

 private:
  void ReloadInstances();

  QPointer<Client> ggp_client_;
  std::optional<Instance> chosen_instance_;
  std::variant<std::monostate, SshInfo, QString> result_;
  QPointer<InstanceItemModel> model_;
  QPointer<QPushButton> refresh_button_;
};

}  // namespace OrbitQt

#endif  // ORBIT_QT_ORBIT_STARTUP_WINDOW_H_
