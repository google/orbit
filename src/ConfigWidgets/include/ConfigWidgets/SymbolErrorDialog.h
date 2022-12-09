// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONFIG_WIDGETS_SYMBOL_ERROR_DIALOG_H_
#define CONFIG_WIDGETS_SYMBOL_ERROR_DIALOG_H_

#include <QDialog>
#include <QObject>
#include <QString>
#include <QWidget>
#include <memory>
#include <string>
#include <string_view>

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
                             std::string_view detailed_error, QWidget* parent = nullptr);
  ~SymbolErrorDialog() override;
  [[nodiscard]] Result Exec();

  // TODO(https://github.com/google/orbit/issues/4589): Connect slots via code and not via UI files,
  // and remove the "public slots" specifier
 public slots:  // NOLINT(readability-redundant-access-specifiers)
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