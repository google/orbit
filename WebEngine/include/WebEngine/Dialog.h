#ifndef WEB_ENGINE_DIALOG_H_
#define WEB_ENGINE_DIALOG_H_

#include <QDialog>
#include <QGridLayout>
#include <QWebChannel>
#include <QWebEngineView>
#include <QWebSocketServer>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <memory>

#include "WebEngine/DeleteLaterDeleter.h"

namespace WebEngine {

class Dialog : public QDialog {
 public:
  explicit Dialog(QWebEngineProfile* profile, QWidget* parent = nullptr);

  QWebEngineView* GetWebEngineView() { return &view_; }
  const QWebEngineView* GetWebEngineView() const { return &view_; }

  QWebEnginePage* GetWebEnginePage() { return &page_; }
  const QWebEnginePage* GetWebEnginePage() const { return &page_; }

  QWebChannel* GetWebChannel() { return web_channel_.get(); }
  const QWebChannel* GetWebChannel() const { return web_channel_.get(); }

  void RegisterObject(const QString& id, QObject* obj) {
    web_channel_->registerObject(id, obj);
  }

 private:
  QGridLayout layout_;
  QWebEngineView view_;
  QWebEnginePage page_;
  std::unique_ptr<QWebSocketServer, DeleteLaterDeleter> web_socket_server_;
  std::unique_ptr<QWebChannel, DeleteLaterDeleter> web_channel_;
};
}  // namespace WebEngine

#endif  // WEB_ENGINE_DIALOG_H_