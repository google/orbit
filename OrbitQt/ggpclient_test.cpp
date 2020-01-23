
#include "ggpclient.h"
#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>
#include <QStringView>
#include <cassert>
#include "ggpinstance.h"

namespace {
void checkInstance(const GgpInstance& instance) {
  assert(instance.pool == "fra-dev-gen1-1080p");
  assert(*instance.owner.rbegin() == '@');
  assert(instance.displayName.size() >= instance.owner.size() - 1);
  assert(QStringView{instance.owner}.left(instance.owner.size() - 1) ==
         QStringView{instance.displayName}.left(instance.owner.size() - 1));

  const QHostAddress addr{instance.ipAddress};
  assert(!addr.isNull());
  assert(instance.lastUpdated.isValid());
}
}  // namespace

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);

  qDebug()
      << "Information: This small test app requires you to have at least one "
         "instance reserved in fra-dev-gen1-1080p, otherwise we cannot "
         "verify the data model.\n";

  auto ggp = GgpClient::Instantiate();

  assert(ggp);
  assert(ggp->GetVersion() == "13352.1.40.0");

  qDebug() << "Trying the synchronous interface...";
  const auto instances = ggp->SyncGetInstances();
  assert(instances);
  assert(instances->size() >= 1);

  qDebug().noquote() << QString("\tFound %1 instance(s). Now checking them...")
                            .arg(instances->size());
  for (const auto& instance : *instances) {
    checkInstance(instance);
  }

  qDebug() << "Trying the asynchronous interface...";
  ggp->AsyncGetInstances(
      [&app](Expected<std::vector<GgpInstance>, QString> instances) {
        if (!instances) {
          qDebug() << instances.GetError();
          assert(false);
          return;
        }

        qDebug().noquote()
            << QString("\tFound %1 instance(s). Now checking them...")
                   .arg(instances->size());

        assert(instances->size() >= 1);
        for (const auto& instance : *instances) {
          checkInstance(instance);
        }
        app.quit();
      });

  assert(ggp->GetRequestsRunning() == 1);
  assert(app.exec() == 0);
  assert(ggp->GetRequestsRunning() == 0);
}