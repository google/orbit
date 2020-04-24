#ifndef ORBITQT_ORBIT_STARTUP_WINDOW_H_
#define ORBITQT_ORBIT_STARTUP_WINDOW_H_

#include <QDialog>
#include <QPointer>
#include <QWidget>
#include <optional>

#include "GgpClient.h"
#include "GgpInstance.h"
#include "GgpInstanceItemModel.h"

class OrbitStartupWindow : public QDialog {
 public:
  explicit OrbitStartupWindow(QWidget* parent = nullptr);
  int Run(std::string* ip_address);

 private:
  void ReloadInstances();

  std::optional<GgpClient> ggp_client_;
  std::optional<GgpInstance> chosen_instance_;
  QPointer<GgpInstanceItemModel> model_;
};

#endif  // ORBITQT_ORBIT_STARTUP_WINDOW_H_