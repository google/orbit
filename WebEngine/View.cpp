// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WebEngine/View.h"

#include <QHostAddress>
#include <QWebChannel>
#include <QWebSocketServer>
#include <memory>
#include <optional>

#include "JsonTransport.h"
#include "OrbitBase/Logging.h"

namespace web_engine {

View::View(std::optional<int> web_socket_port, QObject* parent) : QObject(parent) {
  // NOLINTNEXTLINE
  web_channel_.reset(new QWebChannel{});

  if (web_socket_port) {
    web_socket_server_.reset(
        // NOLINTNEXTLINE
        new QWebSocketServer{"Web channel server", QWebSocketServer::NonSecureMode});

    if (!web_socket_server_->listen(QHostAddress::LocalHost, web_socket_port.value())) {
      ERROR("Opening a port for the web socket server failed: %s",
            web_socket_server_->errorString().toStdString());
      // We continue executing at that point to maintain invariance.
      // If we returned here, web_channel_ would be a nullptr.
    }

    QObject::connect(
        web_socket_server_.get(), &QWebSocketServer::newConnection, web_channel_.get(), [this]() {
          // NOLINTNEXTLINE
          web_channel_->connectTo(new JsonTransport{web_socket_server_->nextPendingConnection()});
        });
  }
}

}  // namespace web_engine