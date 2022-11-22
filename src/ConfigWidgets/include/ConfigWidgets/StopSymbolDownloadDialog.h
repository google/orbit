// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONFIG_WIDGETS_STOP_SYMBOL_DOWNLOAD_DIALOG_H_
#define CONFIG_WIDGETS_STOP_SYMBOL_DOWNLOAD_DIALOG_H_

#include <absl/container/flat_hash_set.h>

#include <QDialog>
#include <QObject>
#include <QString>
#include <memory>

#include "ClientData/ModuleData.h"

namespace Ui {
class StopSymbolDownloadDialog;
}

namespace orbit_config_widgets {

class StopSymbolDownloadDialog : public QDialog {
  Q_OBJECT;

 public:
  enum class Result { kCancel, kStop, kStopAndDisable };

  explicit StopSymbolDownloadDialog(const orbit_client_data::ModuleData* module);
  ~StopSymbolDownloadDialog() override;
  [[nodiscard]] Result Exec();

 private:
  std::unique_ptr<Ui::StopSymbolDownloadDialog> ui_;
};

}  // namespace orbit_config_widgets

#endif  // CONFIG_WIDGETS_STOP_SYMBOL_DOWNLOAD_DIALOG_H_