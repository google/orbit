#include "ggpclient.h"
#include <QDebug>
#include <QEventLoop>
#include <QPointer>
#include <QProcess>
#include <QTimer>
#include <cassert>
#include "expected.h"
#include "ggpinstance.h"

const auto InstanceRequestTimeoutInMilliseconds = 10'000;

Expected<GgpClient, QString> GgpClient::Instantiate() {
  QProcess ggpProcess{};
  ggpProcess.setProgram("ggp");
  ggpProcess.setArguments({"version"});
  ggpProcess.start(QIODevice::ReadOnly);
  ggpProcess.waitForFinished();

  if (ggpProcess.exitStatus() != QProcess::NormalExit)
    return {ExpectedErrorTag, ggpProcess.errorString()};
  if (ggpProcess.exitCode() != 0)
    return {ExpectedErrorTag, ggpProcess.errorString()};

  const auto stdout = ggpProcess.readAllStandardOutput();
  const auto tokens = stdout.split(' ');

  if (tokens.size() < 2)
    return {ExpectedErrorTag,
            "The current version of GGP is not supported by this integration."};

  GgpClient client{};
  client.version = tokens.first().toStdString();
  return client;
}

Expected<std::vector<GgpInstance>, QString> GgpClient::SyncGetInstances() {
  // This code looks a bit weird because the synchronous version is
  // implemented in terms of the asynchronous version. Such that we avoid code
  // duplication. This local event loop thingy is more or less a standard
  // idiom in qt-based code bases to achieve exactly that.

  Expected<std::vector<GgpInstance>, QString> instances;
  QEventLoop loop{};

  AsyncGetInstances(
      [&](Expected<std::vector<GgpInstance>, QString> instances_) {
        instances = std::move(instances_);
        loop.quit();
      });

  loop.exec();
  return instances;
}

void GgpClient::AsyncGetInstances(InstanceCallback callback) {
  assert(callback);

  const auto ggpProcess = QPointer{new QProcess{}};
  ggpProcess->setProgram("ggp");
  ggpProcess->setArguments({"instance", "list", "-s"});

  const auto timeoutTimer = QPointer{new QTimer{}};
  QObject::connect(timeoutTimer, &QTimer::timeout, timeoutTimer,
                   [ggpProcess, timeoutTimer, callback, this]() {
                     callback({ExpectedErrorTag,
                               QString("Request timed out after %1 ms.")
                                   .arg(InstanceRequestTimeoutInMilliseconds)});

                     ggpProcess->terminate();
                     ggpProcess->waitForFinished();
                     requestsRunning--;

                     if (ggpProcess) ggpProcess->deleteLater();
                     timeoutTimer->deleteLater();
                   });

  QObject::connect(
      ggpProcess,
      static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
          &QProcess::finished),
      ggpProcess,
      [callback, ggpProcess, timeoutTimer, this](
          const int exitCode, const QProcess::ExitStatus exitStatus) {
        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
          callback({ExpectedErrorTag, ggpProcess->errorString()});
          return;
        }

        requestsRunning--;

        const auto jsonData = ggpProcess->readAllStandardOutput();
        callback(GgpInstance::GetListFromJson(jsonData));
        ggpProcess->deleteLater();

        if (timeoutTimer) {
          timeoutTimer->stop();
          timeoutTimer->deleteLater();
        }
      });

  requestsRunning++;
  ggpProcess->start(QIODevice::ReadOnly);
  timeoutTimer->start(InstanceRequestTimeoutInMilliseconds);
}