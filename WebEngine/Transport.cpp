#include "WebEngine/Transport.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "OrbitBase/Logging.h"

namespace WebEngine {

Transport::Transport(QWebSocket* socket) : socket_(socket) {
  QObject::connect(
      socket_.get(), &QWebSocket::textMessageReceived, this,
      [this](const QString& msg) {
        QJsonParseError parse_error;
        const auto doc = QJsonDocument::fromJson(msg.toUtf8(), &parse_error);

        if (parse_error.error) {
          ERROR("Failed to parse web channel message: %s. The error was: %s",
                msg.toStdString(), parse_error.errorString().toStdString());
        }

        emit messageReceived(doc.object(), this);
      });
}

void Transport::sendMessage(const QJsonObject& msg) {
  const auto document = QJsonDocument{msg};
  socket_->sendTextMessage(
      QString::fromUtf8(document.toJson(QJsonDocument::Compact)));
}

}  // namespace WebEngine