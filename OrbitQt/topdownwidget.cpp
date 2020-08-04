// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "topdownwidget.h"

#include <QMenu>

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

const std::string TopDownWidget::kActionExpandAll = "Expand all";
const std::string TopDownWidget::kActionCollapseAll = "Collapse all";

static void ExpandRecursively(QTreeView* tree_view, const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }
  for (int i = 0; i < index.model()->rowCount(index); ++i) {
    const QModelIndex& child = index.child(i, 0);
    ExpandRecursively(tree_view, child);
  }
  if (!tree_view->isExpanded(index)) {
    tree_view->expand(index);
  }
}

static void CollapseRecursively(QTreeView* tree_view,
                                const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }
  for (int i = 0; i < index.model()->rowCount(index); ++i) {
    const QModelIndex& child = index.child(i, 0);
    CollapseRecursively(tree_view, child);
  }
  if (tree_view->isExpanded(index)) {
    tree_view->collapse(index);
  }
}

void TopDownWidget::onCustomContextMenuRequested(const QPoint& point) {
  QModelIndex index = ui_->topDownTreeView->indexAt(point);
  if (!index.isValid()) {
    return;
  }
  QMenu menu{ui_->topDownTreeView};

  if (index.model()->rowCount(index) > 0) {
    // Always show "Expand all", as even if the selected node is expanded there
    // could be subtrees not expanded, but only show "Collapse all" when the
    // selected node is expanded, as it would otherwise be unintuitive to
    // collapse subtrees none of which is visible.
    menu.addAction(kActionExpandAll.c_str());
    if (ui_->topDownTreeView->isExpanded(index)) {
      menu.addAction(kActionCollapseAll.c_str());
    }
  }

  QAction* action = menu.exec(ui_->topDownTreeView->mapToGlobal(point));
  if (action == nullptr) {
    return;
  }
  if (action->text().toStdString() == kActionExpandAll) {
    ExpandRecursively(ui_->topDownTreeView, index);
  } else if (action->text().toStdString() == kActionCollapseAll) {
    CollapseRecursively(ui_->topDownTreeView, index);
  }
}
