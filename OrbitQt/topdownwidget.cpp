// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "topdownwidget.h"

#include <QColor>
#include <QMenu>
#include <QSortFilterProxyModel>

#include "TopDownViewItemModel.h"
#include "absl/container/flat_hash_set.h"

class HighlightingSortFilterProxyModel : public QSortFilterProxyModel {
 public:
  explicit HighlightingSortFilterProxyModel(QObject* parent)
      : QSortFilterProxyModel{parent} {}

  // Specify a set where this ProxyModel should put internalPointers of indices
  // that match the current filter.
  void SetFilterAcceptedNodeCollectorSet(
      absl::flat_hash_set<void*>* filter_accepted_nodes_collector) {
    filter_accepted_nodes_collector_ = filter_accepted_nodes_collector;
  }

  // Specify a set of internalPointers whose indices should be highlighted.
  void SetNodesToHighlightSet(
      std::unique_ptr<absl::flat_hash_set<void*>> nodes_to_highlight) {
    nodes_to_highlight_ = std::move(nodes_to_highlight);
  }

  QVariant data(const QModelIndex& index, int role) const override {
    if (role == Qt::ForegroundRole && nodes_to_highlight_ != nullptr) {
      QModelIndex source_index = mapToSource(index);
      if (nodes_to_highlight_->contains(source_index.internalPointer())) {
        return QColor{Qt::green};
      }
    }
    return QSortFilterProxyModel::data(index, role);
  }

 protected:
  bool filterAcceptsRow(int source_row,
                        const QModelIndex& source_parent) const override {
    bool accepts_row =
        QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    if (accepts_row && filter_accepted_nodes_collector_ != nullptr) {
      QModelIndex source_index =
          sourceModel()->index(source_row, 0, source_parent);
      filter_accepted_nodes_collector_->insert(source_index.internalPointer());
    }
    return accepts_row;
  }

 private:
  mutable absl::flat_hash_set<void*>* filter_accepted_nodes_collector_ =
      nullptr;
  std::unique_ptr<absl::flat_hash_set<void*>> nodes_to_highlight_ = nullptr;
};

void TopDownWidget::SetTopDownView(std::unique_ptr<TopDownView> top_down_view) {
  auto* model =
      new TopDownViewItemModel{std::move(top_down_view), ui_->topDownTreeView};
  auto* proxy_model =
      new HighlightingSortFilterProxyModel{ui_->topDownTreeView};
  proxy_model->setSourceModel(model);
  proxy_model->setSortRole(Qt::EditRole);

  proxy_model->setFilterRole(Qt::DisplayRole);
  proxy_model->setFilterKeyColumn(TopDownViewItemModel::kThreadOrFunction);
  proxy_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
  proxy_model->setRecursiveFilteringEnabled(true);

  ui_->topDownTreeView->setModel(proxy_model);
  ui_->topDownTreeView->sortByColumn(TopDownViewItemModel::kInclusive,
                                     Qt::DescendingOrder);
  ui_->topDownTreeView->header()->resizeSections(QHeaderView::ResizeToContents);

  on_searchLineEdit_textEdited(ui_->searchLineEdit->text());
}

const QString TopDownWidget::kActionExpandRecursively =
    QStringLiteral("&Expand recursively");
const QString TopDownWidget::kActionCollapseRecursively =
    QStringLiteral("&Collapse recursively");
const QString TopDownWidget::kActionCollapseChildrenRecursively =
    QStringLiteral("Collapse children recursively");
const QString TopDownWidget::kActionExpandAll = QStringLiteral("Expand all");
const QString TopDownWidget::kActionCollapseAll =
    QStringLiteral("Collapse all");

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

static void CollapseChildrenRecursively(QTreeView* tree_view,
                                        const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }
  for (int i = 0; i < index.model()->rowCount(index); ++i) {
    const QModelIndex& child = index.child(i, 0);
    CollapseRecursively(tree_view, child);
  }
}

static void ExpandRecursivelyButCollapseLeaves(QTreeView* tree_view,
                                               const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }
  if (index.model()->rowCount(index) == 0) {
    tree_view->collapse(index);
    return;
  }
  for (int i = 0; i < index.model()->rowCount(index); ++i) {
    const QModelIndex& child = index.child(i, 0);
    ExpandRecursivelyButCollapseLeaves(tree_view, child);
  }
  if (!tree_view->isExpanded(index)) {
    tree_view->expand(index);
  }
}

static void ExpandAllButCollapseLeaves(QTreeView* tree_view) {
  for (int i = 0; i < tree_view->model()->rowCount(); ++i) {
    const QModelIndex& child = tree_view->model()->index(i, 0);
    ExpandRecursivelyButCollapseLeaves(tree_view, child);
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

void TopDownWidget::on_searchLineEdit_textEdited(const QString& text) {
  auto* proxy_model = dynamic_cast<HighlightingSortFilterProxyModel*>(
      ui_->topDownTreeView->model());
  if (proxy_model == nullptr) {
    return;
  }
  if (text.isEmpty()) {
    proxy_model->SetNodesToHighlightSet(nullptr);
    ui_->topDownTreeView->viewport()->update();
    return;
  }
  // Don't actually hide any node, but expand up to the parents of the matching
  // nodes (so that the matching nodes are shown) and collapse everything else.
  // To do this easily, set a filter on the QTreeView, which actually hides and
  // as a result collapses all the non-matching nodes. Then expand everything
  // that is still visible, but collapse leaves (note that a leaf being expanded
  // has no visual effect while the filter is in place, but those nodes would
  // retain the expanded state when the filter is removed). Finally, remove the
  // filter. The initial collapseAll greatly helps filtering performance.

  // In parallel, collect the nodes that match the filter from the
  // HighlightingSortFilterProxyModel and set them as nodes to highlight.
  ui_->topDownTreeView->collapseAll();
  auto filter_accepted_nodes_collector =
      std::make_unique<absl::flat_hash_set<void*>>();
  proxy_model->SetFilterAcceptedNodeCollectorSet(
      filter_accepted_nodes_collector.get());
  proxy_model->setFilterFixedString(text);
  ExpandAllButCollapseLeaves(ui_->topDownTreeView);
  proxy_model->SetFilterAcceptedNodeCollectorSet(nullptr);
  proxy_model->SetNodesToHighlightSet(
      std::move(filter_accepted_nodes_collector));
  proxy_model->setFilterFixedString(QStringLiteral(""));
}
