// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBITQT_ORBIT_STARTUP_WINDOW_H_
#define ORBITQT_ORBIT_STARTUP_WINDOW_H_

#include <QDialog>
#include <QPointer>
#include <QString>
#include <QWidget>
#include <optional>
#include <outcome.hpp>
#include <system_error>
#include <variant>

#include "OrbitBase/Logging.h"
#include "OrbitGgp/GgpClient.h"
#include "OrbitGgp/GgpInstance.h"
#include "OrbitGgp/GgpInstanceItemModel.h"
#include "OrbitGgp/GgpSshInfo.h"

class OrbitStartupWindow : public QDialog {
 public:
  explicit OrbitStartupWindow(QWidget* parent = nullptr);

  template <typename Credentials>
  outcome::result<std::variant<Credentials, QString>> Run() {
    result_ = std::monostate{};
    const int dialog_result = exec();

    if (dialog_result != 0) {
      if (std::holds_alternative<GgpSshInfo>(result_)) {
        auto& ssh_info = std::get<GgpSshInfo>(result_);
        Credentials credentials{};
        credentials.host = ssh_info.host.toStdString();
        credentials.key_path = ssh_info.key_path.toStdString();
        credentials.known_hosts_path = ssh_info.known_hosts_path.toStdString();
        credentials.user = ssh_info.user.toStdString();
        credentials.port = ssh_info.port;
        return outcome::success(std::move(credentials));
      } else if (std::holds_alternative<QString>(result_)) {
        return outcome::success(std::get<QString>(result_));
      } else {
        UNREACHABLE();
      }
    }
    // TODO(hebecker): That's a hack for now. We need a proper error category
    // which clearly states that the user pressed the cancel button.
    return std::errc::interrupted;
  }

 private:
  void ReloadInstances(QPointer<QPushButton> refresh_button);

  std::optional<GgpClient> ggp_client_;
  std::optional<GgpInstance> chosen_instance_;
  std::variant<std::monostate, GgpSshInfo, QString> result_;
  QPointer<GgpInstanceItemModel> model_;
};

#endif  // ORBITQT_ORBIT_STARTUP_WINDOW_H_