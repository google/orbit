#include <QApplication>
#include <QDebug>
#include <QGridLayout>
#include <QMainWindow>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTimer>
#include "ggpclient.h"
#include "ggpinstanceitemmodel.h"

int main(int argc, char* argv[]) {
  QApplication app{argc, argv};

  QMainWindow window{};

  QTableView view{};
  window.setCentralWidget(&view);

  auto client = GgpClient::Instantiate();

  if (!client) {
    QMessageBox::critical(nullptr, "GGP Instamce Model Demo",
                          "Error calling ggp");
    return 1;
  }

  GgpInstanceItemModel model{};
  if (const auto instances = client->SyncGetInstances(); instances) {
    model.setInstances(std::move(*instances));
  } else {
    QMessageBox::critical(nullptr, "GGP Instance Model Demo",
                          QString("Could not retrieve the list of currently "
                                  "running instance. The error was: %1")
                              .arg(instances.GetError()));
    return 1;
  }

  QSortFilterProxyModel proxyModel{};
  proxyModel.setSourceModel(&model);
  view.setModel(&proxyModel);
  view.setSortingEnabled(true);

  QTimer refreshTimer{};
  QObject::connect(&refreshTimer, &QTimer::timeout, &refreshTimer, [&]() {
    if (client->GetRequestsRunning() == 0) {
      qDebug() << "Starting update.";
      client->AsyncGetInstances(
          [&](Expected<std::vector<GgpInstance>, QString> instances) {
            if (instances)
              model.setInstances(std::move(*instances));
            else
              qWarning() << instances.GetError();
            qDebug() << "Updated.";
          });
    }
  });
  refreshTimer.start(7'000);

  window.show();
  return app.exec();
}