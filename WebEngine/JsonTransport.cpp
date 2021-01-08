// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "JsonTransport.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "OrbitBase/Logging.h"

namespace web_engine {

JsonTransport::JsonTransport(QWebSocket* socket) : socket_(socket) {
  QObject::connect(socket_.get(), &QWebSocket::textMessageReceived, this,
                   [this](const QString& msg) {
                     QJsonParseError parse_error;
                     const auto doc = QJsonDocument::fromJson(msg.toUtf8(), &parse_error);

                     if (parse_error.error) {
                       ERROR("Failed to parse web channel message: %s. Message:\n%s",
                             parse_error.errorString().toStdString(), msg.toStdString());
                       return;
                     }

                     emit messageReceived(doc.object(), this);
                   });
}

void JsonTransport::sendMessage(const QJsonObject& msg) {
  const auto document = QJsonDocument{msg};
  socket_->sendTextMessage(QString::fromUtf8(document.toJson(QJsonDocument::Compact)));
}

}  // namespace web_engine