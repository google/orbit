// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_ENGINE_TRANSPORT_H_
#define WEB_ENGINE_TRANSPORT_H_

#include <QObject>
#include <QWebChannelAbstractTransport>
#include <QWebSocket>
#include <memory>

#include "WebEngine/DeleteLaterDeleter.h"

namespace WebEngine {

/* Json-based transport for QtWebChannel

   A transport in the QtWebChannel framework is the link
   between to C++ WebChannel object and the websocket talking
   to the JavaScript part.

   This is a very basic implementation and also the default.
   It encodes all messages as JSON. The default JavaScript
   channel implementation (qwebchannel.js) is also expecting
   the message to be JSON-encoded.
   So the serialization mechanism can't be easily swapped
   out without rewriting the JS part as well.

   Usage: Call QWebChannel's connectTo method whenever a new
   websocket connection is pending:

   QWebChannel* web_channel = new QWebChannel{...};
   QWebSocketServer* web_socket_server = new QWebSocketServer{...};
   web_channel->connectTo(new JsonTransport{web_socket_server->nextPendingConnection()});
*/
class JsonTransport : public QWebChannelAbstractTransport {
  Q_OBJECT

 public:
  explicit JsonTransport(QWebSocket* socket);

  void sendMessage(const QJsonObject& msg) override;

 private:
  std::unique_ptr<QWebSocket, DeleteLaterDeleter> socket_;
};
}  // namespace WebEngine
#endif  // WEB_ENGINE_TRANSPORT_H_