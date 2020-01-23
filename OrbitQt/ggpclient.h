#ifndef ORBIT_ORBITQT_GGPCLIENT_H
#define ORBIT_ORBITQT_GGPCLIENT_H

#include <functional>
#include <string>
#include <vector>
#include "ggpinstance.h"
#include "expected.h"

class GgpClient {
  GgpClient() = default;

  std::string version;
  int requestsRunning = 0;

 public:
  static Expected<GgpClient, QString> Instantiate();

  const std::string &GetVersion() const { return version; }
  int GetRequestsRunning() const { return requestsRunning; }

  Expected<std::vector<GgpInstance>, QString> SyncGetInstances();

  using InstanceCallback = std::function<void(Expected<std::vector<GgpInstance>, QString>)>;
  void AsyncGetInstances(InstanceCallback callback);
};

#endif  // ORBIT_ORBITQT_GGPCLIENT_H
