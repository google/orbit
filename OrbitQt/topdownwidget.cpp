// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "topdownwidget.h"

#include <QColor>
#include <QMenu>

#include "TopDownViewItemModel.h"

void TopDownWidget::SetTopDownView(std::unique_ptr<TopDownView> top_down_view) {
  model_ = std::make_unique<TopDownViewItemModel>(std::move(top_down_view));
  proxy_model_ = std::make_unique<HighlightCustomFilterSortFilterProxyModel>(nullptr);
  proxy_model_->setSourceModel(model_.get());
  proxy_model_->setSortRole(Qt::EditRole);

  ui_->topDownTreeView->setModel(proxy_model_.get());
  ui_->topDownTreeView->sortByColumn(TopDownViewItemModel::kInclusive, Qt::DescendingOrder);
  ui_->topDownTreeView->header()->resizeSections(QHeaderView::ResizeToContents);

  on_searchLineEdit_textEdited(ui_->searchLineEdit->text());
}

const QString TopDownWidget::kActionExpandRecursively = QStringLiteral("&Expand recursively");
const QString TopDownWidget::kActionCollapseRecursively = QStringLiteral("&Collapse recursively");
const QString TopDownWidget::kActionCollapseChildrenRecursively =
    QStringLiteral("Collapse children recursively");
const QString TopDownWidget::kActionExpandAll = QStringLiteral("Expand all");
const QString TopDownWidget::kActionCollapseAll = QStringLiteral("Collapse all");

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

static void CollapseRecursively(QTreeView* tree_view, const QModelIndex& index) {
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

static void CollapseChildrenRecursively(QTreeView* tree_view, const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }
  for (int i = 0; i < index.model()->rowCount(index); ++i) {
    const QModelIndex& child = index.child(i, 0);
    CollapseRecursively(tree_view, child);
  }
}

void TopDownWidget::onCustomContextMenuRequested(const QPoint& point) {
  QModelIndex index = ui_->topDownTreeView->indexAt(point);
  if (!index.isValid()) {
    return;
  }

  std::vector<QModelIndex> selected_tree_indices;
  for (const QModelIndex& selected_index :
       ui_->topDownTreeView->selectionModel()->selectedIndexes()) {
    if (selected_index.column() != TopDownViewItemModel::kThreadOrFunction) {
      continue;
    }
    selected_tree_indices.push_back(selected_index);
  }

  bool enable_expand_recursively = false;
  bool enable_collapse_recursively = false;
  for (const QModelIndex& selected_index : selected_tree_indices) {
    if (selected_index.model()->rowCount(selected_index) > 0) {
      // As long as at least one of the selected nodes has children, always show
      // "Expand recursively", as even if the selected node is expanded there
      // could be subtrees not expanded. But only show "Collapse recursively"
      // and "Collapse children recursively" when at least one selected node is
      // expanded, as it would otherwise be unintuitive to collapse subtrees
      // none of which is visible.
      enable_expand_recursively = true;
      if (ui_->topDownTreeView->isExpanded(selected_index)) {
        enable_collapse_recursively = true;
      }
    }
  }

  QMenu menu{ui_->topDownTreeView};
  if (enable_expand_recursively) {
    menu.addAction(kActionExpandRecursively);
  }
  if (enable_collapse_recursively) {
    menu.addAction(kActionCollapseRecursively);
    menu.addAction(kActionCollapseChildrenRecursively);
  }
  menu.addSeparator();
  menu.addAction(kActionExpandAll);
  menu.addAction(kActionCollapseAll);

  QAction* action = menu.exec(ui_->topDownTreeView->mapToGlobal(point));
  if (action == nullptr) {
    return;
  }

  if (action->text() == kActionExpandRecursively) {
    for (const QModelIndex& selected_index : selected_tree_indices) {
      ExpandRecursively(ui_->topDownTreeView, selected_index);
    }
  } else if (action->text() == kActionCollapseRecursively) {
    for (const QModelIndex& selected_index : selected_tree_indices) {
      CollapseRecursively(ui_->topDownTreeView, selected_index);
    }
  } else if (action->text() == kActionCollapseChildrenRecursively) {
    for (const QModelIndex& selected_index : selected_tree_indices) {
      CollapseChildrenRecursively(ui_->topDownTreeView, selected_index);
    }
  } else if (action->text() == kActionExpandAll) {
    ui_->topDownTreeView->expandAll();
  } else if (action->text() == kActionCollapseAll) {
    ui_->topDownTreeView->collapseAll();
  }
}

static bool ExpandCollapseRecursivelyBasedOnDescendantsRole(QTreeView* tree_view,
                                                            const QModelIndex& index, int role) {
  if (!index.isValid()) {
    return false;
  }
  bool matches = index.data(role).toBool();
  bool descendant_matches = false;
  for (int i = 0; i < index.model()->rowCount(index); ++i) {
    const QModelIndex& child = index.child(i, 0);
    descendant_matches |= ExpandCollapseRecursivelyBasedOnDescendantsRole(tree_view, child, role);
  }
  if (descendant_matches && !tree_view->isExpanded(index)) {
    tree_view->expand(index);
  } else if (!descendant_matches && tree_view->isExpanded(index)) {
    tree_view->collapse(index);
  }
  return matches || descendant_matches;
}

static void ExpandCollapseBasedOnRole(QTreeView* tree_view, int role) {
  for (int i = 0; i < tree_view->model()->rowCount(); ++i) {
    const QModelIndex& child = tree_view->model()->index(i, 0);
    ExpandCollapseRecursivelyBasedOnDescendantsRole(tree_view, child, role);
  }
}

void TopDownWidget::on_searchLineEdit_textEdited(const QString& text) {
  if (proxy_model_ == nullptr) {
    return;
  }
  proxy_model_->SetFilter(text.toStdString());
  ui_->topDownTreeView->viewport()->update();
  if (!text.isEmpty()) {
    ExpandCollapseBasedOnRole(
        ui_->topDownTreeView,
        TopDownWidget::HighlightCustomFilterSortFilterProxyModel::kMatchesCustomFilterRole);
  }
}

QVariant TopDownWidget::HighlightCustomFilterSortFilterProxyModel::data(const QModelIndex& index,
                                                                        int role) const {
  if (role == Qt::ForegroundRole) {
    if (!lowercase_filter_.empty() && ItemMatchesFilter(index)) {
      return QColor{Qt::green};
    }
  } else if (role == kMatchesCustomFilterRole) {
    return ItemMatchesFilter(index);
  }
  return QSortFilterProxyModel::data(index, role);
}

bool TopDownWidget::HighlightCustomFilterSortFilterProxyModel::ItemMatchesFilter(
    const QModelIndex& index) const {
  std::string haystack = absl::AsciiStrToLower(
      index.model()
          ->index(index.row(), TopDownViewItemModel::kThreadOrFunction, index.parent())
          .data()
          .toString()
          .toStdString());
  return absl::StrContains(haystack, lowercase_filter_);
}
