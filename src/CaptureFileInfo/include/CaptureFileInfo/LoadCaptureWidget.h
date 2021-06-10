// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_INFO_LOAD_CAPTURE_WIDGET_H_
#define CAPTURE_FILE_INFO_LOAD_CAPTURE_WIDGET_H_

#include <QEvent>
#include <QSortFilterProxyModel>
#include <QWidget>
#include <memory>

#include "CaptureFileInfo/ItemModel.h"

namespace Ui {
class LoadCaptureWidget;  // IWYU pragma: keep
}

namespace orbit_capture_file_info {

class LoadCaptureWidget : public QWidget {
  Q_OBJECT
  Q_PROPERTY(bool active READ IsActive WRITE SetActive)

 public:
  explicit LoadCaptureWidget(QWidget* parent = nullptr);
  ~LoadCaptureWidget() noexcept override;

  [[nodiscard]] bool IsActive() const;

 public slots:
  void SetActive(bool value);

 signals:
  void Activated();
  void FileSelected(std::filesystem::path file_path);
  void SelectionConfirmed();

 private:
  std::unique_ptr<Ui::LoadCaptureWidget> ui_;
  ItemModel item_model_;
  QSortFilterProxyModel proxy_item_model_;

  void showEvent(QShowEvent* event) override;
  void DetachRadioButton();
  void SelectViaFilePicker();
};

}  // namespace orbit_capture_file_info

#endif  // CAPTURE_FILE_INFO_LOAD_CAPTURE_WIDGET_H_