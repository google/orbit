// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include <memory>

#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "SessionSetup/RetrieveInstancesWidget.h"

using orbit_ggp::Client;

int main(int argc, char* argv[]) {
  QApplication app{argc, argv};

  ErrorMessageOr<std::unique_ptr<Client>> client_or_error = orbit_ggp::CreateClient();
  FAIL_IF(client_or_error.has_error(), "%s", client_or_error.error().message());

  orbit_session_setup::RetrieveInstancesWidget widget{client_or_error.value().get()};
  widget.show();

  return QApplication::exec();
}
