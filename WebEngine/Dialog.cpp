#include "WebEngine/Dialog.h"

#include <QHostAddress>
#include <QMargins>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebSocketServer>
#include <memory>

#include "WebEngine/Transport.h"

namespace WebEngine {

Dialog::Dialog(QWebEngineProfile* profile, QWidget* parent)
    : QDialog(parent), page_(profile) {
  web_socket_server_.reset(new QWebSocketServer{
      "Web channel server", QWebSocketServer::NonSecureMode});
  web_socket_server_->listen(QHostAddress::LocalHost, 12345);

  web_channel_.reset(new QWebChannel{});

  QObject::connect(web_socket_server_.get(), &QWebSocketServer::newConnection,
                   web_channel_.get(), [this]() {
                     qDebug() << "new connection";
                     web_channel_->connectTo(new Transport{
                         web_socket_server_->nextPendingConnection()});
                   });
  web_channel_->registerObject("dialog", this);

  resize(QSize{800, 600});

  layout_.setContentsMargins(QMargins{});
  setLayout(&layout_);
  layout_.addWidget(&view_);

  view_.setPage(&page_);
}

}  // namespace WebEngine