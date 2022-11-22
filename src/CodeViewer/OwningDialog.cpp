// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CodeViewer/OwningDialog.h"

#include <Qt>
#include <utility>

namespace orbit_code_viewer {

OwningDialog::~OwningDialog() {
  // `code_report_` is deleted before the base class is deleted, so we have to clear the code report
  // pointer from the base class first.
  ClearHeatmap();
}

void OwningDialog::SetOwningHeatmap(FontSizeInEm heatmap_bar_width,
                                    std::unique_ptr<orbit_code_report::CodeReport> code_report) {
  ClearHeatmap();
  code_report_ = std::move(code_report);
  SetHeatmap(heatmap_bar_width, code_report_.get());
}

void OwningDialog::ClearOwningHeatmap() {
  ClearHeatmap();
  code_report_.reset();
}

QPointer<OwningDialog> OpenAndDeleteOnClose(std::unique_ptr<OwningDialog> dialog) {
  dialog->open();
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  return QPointer<OwningDialog>{dialog.release()};
}

}  // namespace orbit_code_viewer