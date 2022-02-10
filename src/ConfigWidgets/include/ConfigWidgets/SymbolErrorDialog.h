// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONFIG_WIDGETS_SYMBOL_ERROR_DIALOG_H_
#define CONFIG_WIDGETS_SYMBOL_ERROR_DIALOG_H_

#include <QDialog>
#include <memory>

#include "ClientData/ModuleData.h"

namespace Ui {
class SymbolErrorDialog;
}

namespace orbit_config_widgets {

class SymbolErrorDialog : public QDialog {
  Q_OBJECT

 public:
  enum class Result { kCancel, kTryAgain, kAddSymbolLocation };

  explicit SymbolErrorDialog(const orbit_client_data::ModuleData* module,
                             const std::string& detailed_error, QWidget* parent = nullptr);
  ~SymbolErrorDialog() override;
  [[nodiscard]] Result Exec();

 public slots:
  void OnShowErrorButtonClicked();
  void OnAddSymbolLocationButtonClicked();
  void OnTryAgainButtonClicked();

 private:
  std::unique_ptr<Ui::SymbolErrorDialog> ui_;
  const orbit_client_data::ModuleData* module_;
  Result result_ = Result::kCancel;
};

}  // namespace orbit_config_widgets

#endif  // CONFIG_WIDGETS_SYMBOL_ERROR_DIALOG_H_