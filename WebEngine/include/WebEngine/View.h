// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_ENGINE_VIEW_H_
#define WEB_ENGINE_VIEW_H_

#include <QWebChannel>
#include <QWebSocketServer>
#include <memory>
#include <optional>

#include "WebEngine/DeleteLaterDeleter.h"

namespace web_engine {

/* web_engine::View acts as an interface to a QWebEnginePage.

   It provides a QWebChannel and serves that via web socket
   or the internal IPC mechanism.

   You can use the RegisterObject method to expose a C++ object
   to the JavaScript core. It's based on Qt's reflection system
   to enumerate functions and variables, so this class needs to
   derive from QObject and exposed functions need either be attributed
   with Q_INVOKABLE or marked as a slot.

   Signals work as well and can trigger events on the JavaScript side.
   Qt Properties can also be used.

   The communication with the JavaScript engine usually works via Chromium's
   internal IPC mechanism. This won't be available when the web view is loaded
   in an external browser for debugging purposes. Therefore it is also possible
   to start a websocket server which can also expose the communication channel.
   To do so, specify a port number as the second argument. 0 is also valid and
   will instruct the operating system to choose a port. It can be obtained via
   `dialog.GetWebSocketServer().value()->serverPort()`. Both communication
   channels work at the same time. The web socket server will only listen on
   localhost.

   Example:
   QObject* GetMyData();

   web_engine::View view{std::nullopt};
   view.RegisterObject("my_data", GetMyData());

   QWebEnginePage page{}; // Construct QWebEnginePage
   page.setWebChannel(view.GetWebChannel());
*/
class View : public QObject {
  Q_OBJECT

 public:
  explicit View(std::optional<int> web_socket_port_number, QObject* parent = nullptr);

  QWebChannel* GetWebChannel() { return web_channel_.get(); }
  const QWebChannel* GetWebChannel() const { return web_channel_.get(); }

  std::optional<QWebSocketServer*> GetWebSocketServer() {
    if (web_socket_server_) {
      return web_socket_server_.get();
    } else {
      return std::nullopt;
    }
  }
  std::optional<const QWebSocketServer*> GetWebSocketServer() const {
    if (web_socket_server_) {
      return web_socket_server_.get();
    } else {
      return std::nullopt;
    }
  }

  void RegisterObject(const QString& id, QObject* obj) { web_channel_->registerObject(id, obj); }

 private:
  std::unique_ptr<QWebSocketServer, DeleteLaterDeleter> web_socket_server_;
  std::unique_ptr<QWebChannel, DeleteLaterDeleter> web_channel_;
};
}  // namespace web_engine

#endif  // WEB_ENGINE_VIEW_H_