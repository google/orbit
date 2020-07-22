#ifndef WEB_ENGINE_TRANSPORT_H_
#define WEB_ENGINE_TRANSPORT_H_

#include <QObject>
#include <QWebChannelAbstractTransport>
#include <QWebSocket>
#include <memory>

#include "WebEngine/DeleteLaterDeleter.h"

namespace WebEngine {

class Transport : public QWebChannelAbstractTransport {
  Q_OBJECT

 public:
  explicit Transport(QWebSocket* socket);

  void sendMessage(const QJsonObject& msg) override;

 private:
  std::unique_ptr<QWebSocket, DeleteLaterDeleter> socket_;
};
}  // namespace WebEngine
#endif  // WEB_ENGINE_TRANSPORT_H_