// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODE_VIEWER_OWNING_DIALOG_H_
#define CODE_VIEWER_OWNING_DIALOG_H_

#include <QObject>
#include <QPointer>
#include <QString>
#include <memory>
#include <optional>
#include <variant>

#include "CodeReport/CodeReport.h"
#include "CodeViewer/Dialog.h"
#include "CodeViewer/FontSizeInEm.h"

namespace orbit_code_viewer {

/*
  This owning version of `Dialog` is meant to be "self-sustaining",
  meaning it owns all the resources needed.

  It is meant to be used in conjunction with `OpenAndDeleteOnClose`, example:

  auto dialog = std::make_unique<orbit_code_viewer::OwningDialog>();
  dialog->SetMainContent(...);
  dialog->SetOwningHeatmap(kSidebarWidth, std::move(disassembly_report));
  orbit_code_viewer::OpenAndDeleteOnClose(std::move(dialog));
*/
class OwningDialog : public Dialog {
  Q_OBJECT

 public:
  using Dialog::Dialog;
  ~OwningDialog() override;

  void SetOwningHeatmap(FontSizeInEm heatmap_bar_width,
                        std::unique_ptr<orbit_code_report::CodeReport> code_report);
  void ClearOwningHeatmap();

 private:
  std::unique_ptr<orbit_code_report::CodeReport> code_report_;
};

// This function opens the given dialog and ensures it is deleted when closed.
// Note, this function returns immediately after opening the dialog, NOT when it is closed. Use
// `QDialog::exec` to wait for the dialog.
QPointer<OwningDialog> OpenAndDeleteOnClose(std::unique_ptr<OwningDialog> dialog);
}  // namespace orbit_code_viewer

#endif  // CODE_VIEWER_OWNING_DIALOG_H_