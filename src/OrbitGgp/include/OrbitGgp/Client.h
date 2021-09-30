// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GGP_CLIENT_H_
#define ORBIT_GGP_CLIENT_H_

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVector>
#include <chrono>
#include <functional>
#include <optional>
#include <string>

#include "Instance.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Project.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "SshInfo.h"

namespace orbit_ggp {

constexpr const char* kDefaultGgpProgram{"ggp"};

class Client : public QObject {
  Q_OBJECT

 public:
  static ErrorMessageOr<QPointer<Client>> Create(
      QObject* parent, orbit_qt_utils::MainThreadExecutorImpl* main_thread_executor,
      QString ggp_program = kDefaultGgpProgram,
      std::chrono::milliseconds timeout = GetDefaultTimeoutMs());

  orbit_base::Future<ErrorMessageOr<QVector<Instance>>> GetInstancesAsync(
      bool all_reserved = false, std::optional<Project> project = std::nullopt, int retry = 3);
  orbit_base::Future<ErrorMessageOr<SshInfo>> GetSshInfoAsync(
      const Instance& ggp_instance, std::optional<Project> project = std::nullopt);
  orbit_base::Future<ErrorMessageOr<QVector<Project>>> GetProjectsAsync();

 private:
  explicit Client(QObject* parent, orbit_qt_utils::MainThreadExecutorImpl* main_thread_executor,
                  QString ggp_program, std::chrono::milliseconds timeout)
      : QObject(parent),
        main_thread_executor_(main_thread_executor),
        ggp_program_(std::move(ggp_program)),
        timeout_(timeout) {}
  static std::chrono::milliseconds GetDefaultTimeoutMs();

  orbit_qt_utils::MainThreadExecutorImpl* main_thread_executor_;
  const QString ggp_program_;
  const std::chrono::milliseconds timeout_;
};

}  // namespace orbit_ggp

#endif  // ORBIT_GGP_CLIENT_H_
