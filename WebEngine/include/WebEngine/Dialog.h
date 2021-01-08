// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_ENGINE_DIALOG_H_
#define WEB_ENGINE_DIALOG_H_

#include <QDialog>
#include <QGridLayout>
#include <QObject>
#include <QString>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include <QWebSocketServer>
#include <QWidget>
#include <memory>

#include "WebEngine/DeleteLaterDeleter.h"

namespace web_engine {

/* web_engine::Dialog is a dialog window with a full screen web view.

   The class bundles all the resources needed to show a web page
   and to communicate with the C++ part using Qt's webchannel.

   You can use the RegisterObject method to expose a C++ object
   to the JavaScript core. It's based on Qt's reflection system
   to enumerate functions and variables, so this class needs to
   derive from QObject and exposed functions need either be attributed
   with Q_INVOKABLE or marked as a slot.

   Signals work as well and can trigger events on the JavaScript side.
   Qt Properties can also be used.

   Be aware that constructing that object can take around 100 to 200
   milliseconds, especially when the Chromium engine needs to be fired
   up first. Depending on the use case that delay might be noticable to
   the user. In that case it helps to construct the dialog upfront and
   hide it until it's needed.

   Example:
   QWebEngineProfile profile{};
   web_engine::Dialog dialog{&profile};
   dialog.GetWebEnginePage()->load(QUrl{"https://www.google.com/"});
   dialog.setWindowModality(Qt::ApplicationModal);
   dialog.exec();
*/
class Dialog : public QDialog {
  Q_OBJECT

 public:
  explicit Dialog(QWebEngineProfile* profile, QWidget* parent = nullptr);

  QWebEngineView* GetWebEngineView() { return &view_; }
  const QWebEngineView* GetWebEngineView() const { return &view_; }

  QWebEnginePage* GetWebEnginePage() { return &page_; }
  const QWebEnginePage* GetWebEnginePage() const { return &page_; }

  QWebChannel* GetWebChannel() { return web_channel_.get(); }
  const QWebChannel* GetWebChannel() const { return web_channel_.get(); }

  QWebSocketServer* GetWebSocketServer() { return web_socket_server_.get(); }
  const QWebSocketServer* GetWebSocketServer() const { return web_socket_server_.get(); }

  void RegisterObject(const QString& id, QObject* obj) { web_channel_->registerObject(id, obj); }

 private:
  QGridLayout layout_;
  QWebEngineView view_;
  QWebEnginePage page_;
  std::unique_ptr<QWebSocketServer, DeleteLaterDeleter> web_socket_server_;
  std::unique_ptr<QWebChannel, DeleteLaterDeleter> web_channel_;
};
}  // namespace web_engine

#endif  // WEB_ENGINE_DIALOG_H_