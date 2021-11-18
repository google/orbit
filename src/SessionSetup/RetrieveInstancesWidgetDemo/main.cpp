// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include <QMessageBox>
#include <memory>

#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "SessionSetup/RetrieveInstances.h"
#include "SessionSetup/RetrieveInstancesWidget.h"

namespace {
constexpr const char* kOrganizationName = "The Orbit Authors";
constexpr const char* kApplicationName = "RetrieveInstancesWidgetDemo";
}  // namespace

using orbit_ggp::Client;
using orbit_session_setup::RetrieveInstances;
using orbit_session_setup::RetrieveInstancesWidget;

int main(int argc, char* argv[]) {
  QApplication app{argc, argv};
  QCoreApplication::setOrganizationName(kOrganizationName);
  QCoreApplication::setApplicationName(kApplicationName);

  ErrorMessageOr<std::unique_ptr<Client>> client_or_error = orbit_ggp::CreateClient();
  FAIL_IF(client_or_error.has_error(), "%s", client_or_error.error().message());
  Client* client_ptr = client_or_error.value().get();

  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> executor{
      orbit_qt_utils::MainThreadExecutorImpl::Create()};

  RetrieveInstancesWidget widget{};

  std::unique_ptr<RetrieveInstances> retrieve_instances =
      RetrieveInstances::Create(client_ptr, executor.get());
  widget.SetRetrieveInstances(retrieve_instances.get());

  QObject::connect(&widget, &RetrieveInstancesWidget::LoadingSuccessful, &widget,
                   [&widget](const QVector<orbit_ggp::Instance>& instances) {
                     QString message =
                         QString{"Retrieved %1 instance. This is the list (display name):\n"}.arg(
                             instances.count());
                     for (const auto& instance : instances) {
                       message.append(QString{"* %1\n"}.arg(instance.display_name));
                     }

                     QMessageBox::information(&widget, QApplication::applicationName(), message);
                   });

  widget.show();
  widget.Start();

  return QApplication::exec();
}
