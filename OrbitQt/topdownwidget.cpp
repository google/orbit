// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "topdownwidget.h"

#include "TopDownViewItemModel.h"

void TopDownWidget::SetTopDownView(std::unique_ptr<TopDownView> top_down_view) {
  auto* model =
      new TopDownViewItemModel{std::move(top_down_view), ui_->topDownTreeView};
  auto* proxy_model = new QSortFilterProxyModel{ui_->topDownTreeView};
  proxy_model->setSourceModel(model);
  proxy_model->setSortRole(Qt::EditRole);
  ui_->topDownTreeView->setModel(proxy_model);
  ui_->topDownTreeView->sortByColumn(TopDownViewItemModel::kInclusive,
                                     Qt::DescendingOrder);
  for (int column = 0; column < TopDownViewItemModel::kColumnCount; ++column) {
    ui_->topDownTreeView->resizeColumnToContents(column);
  }
}
