// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WebEngine/Dialog.h"

#include <QHostAddress>
#include <QMargins>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebSocketServer>
#include <memory>

#include "JsonTransport.h"
#include "OrbitBase/Logging.h"

namespace WebEngine {

Dialog::Dialog(QWebEngineProfile* profile, QWidget* parent) : QDialog(parent), page_(profile) {
  web_socket_server_.reset(
      // NOLINTNEXTLINE
      new QWebSocketServer{"Web channel server", QWebSocketServer::NonSecureMode});

  if (!web_socket_server_->listen(QHostAddress::LocalHost, 0)) {
    ERROR("Opening a port for the web socket server failed: %s",
          web_socket_server_->errorString().toStdString());
    // We continue executing at that point to maintain invariance.
    // If we returned here, web_channel_ would be a nullptr.
  }

  // NOLINTNEXTLINE
  web_channel_.reset(new QWebChannel{});

  QObject::connect(
      web_socket_server_.get(), &QWebSocketServer::newConnection, web_channel_.get(), [this]() {
        // NOLINTNEXTLINE
        web_channel_->connectTo(new JsonTransport{web_socket_server_->nextPendingConnection()});
      });

  resize(QSize{800, 600});

  layout_.setContentsMargins(QMargins{});
  setLayout(&layout_);
  layout_.addWidget(&view_);

  view_.setPage(&page_);
}

}  // namespace WebEngine