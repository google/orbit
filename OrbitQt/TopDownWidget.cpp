// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TopDownWidget.h"

#include <QColor>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QStyleOption>

#include "App.h"
#include "FunctionsDataView.h"
#include "TopDownViewItemModel.h"

using orbit_client_protos::FunctionInfo;

TopDownWidget::TopDownWidget(QWidget* parent)
    : QWidget{parent}, ui_{std::make_unique<Ui::TopDownWidget>()} {
  ui_->setupUi(this);
  ui_->topDownTreeView->setItemDelegateForColumn(TopDownViewItemModel::kInclusive,
                                                 new ProgressBarItemDelegate{ui_->topDownTreeView});

  connect(ui_->topDownTreeView, &CopyKeySequenceEnabledTreeView::copyKeySequencePressed, this,
          &TopDownWidget::onCopyKeySequencePressed);
  connect(ui_->topDownTreeView, &QTreeView::customContextMenuRequested, this,
          &TopDownWidget::onCustomContextMenuRequested);
  connect(ui_->searchLineEdit, &QLineEdit::textEdited, this,
          &TopDownWidget::onSearchLineEditTextEdited);
}

void TopDownWidget::SetTopDownView(std::unique_ptr<TopDownView> top_down_view) {
  CHECK(app_ != nullptr);

  model_ = std::make_unique<TopDownViewItemModel>(std::move(top_down_view));
  search_proxy_model_ = std::make_unique<HighlightCustomFilterSortFilterProxyModel>(nullptr);
  search_proxy_model_->setSourceModel(model_.get());
  search_proxy_model_->setSortRole(Qt::EditRole);

  hooked_proxy_model_ = std::make_unique<HookedIdentityProxyModel>(app_, nullptr);
  hooked_proxy_model_->setSourceModel(search_proxy_model_.get());

  ui_->topDownTreeView->setModel(hooked_proxy_model_.get());
  ui_->topDownTreeView->sortByColumn(TopDownViewItemModel::kInclusive, Qt::DescendingOrder);

  // Resize columns only the first time a non-empty TopDownView is set.
  if (!columns_already_resized_ && hooked_proxy_model_->rowCount(QModelIndex{}) > 0) {
    ui_->topDownTreeView->header()->resizeSections(QHeaderView::ResizeToContents);
    columns_already_resized_ = true;
  }

  onSearchLineEditTextEdited(ui_->searchLineEdit->text());
}

static std::string BuildStringFromIndices(const QModelIndexList& indices) {
  std::string buffer;
  std::optional<QModelIndex> prev_index;
  // Note: indices are sorted by row in order of selection and then by column in ascending order.
  for (const QModelIndex& index : indices) {
    if (prev_index.has_value()) {
      // row() is the position among siblings: also compare parent().
      if (index.row() != prev_index->row() || index.parent() != prev_index->parent()) {
        buffer += "\n";
      } else {
        buffer += ", ";
      }
    }
    buffer += index.data().toString().toStdString();
    prev_index = index;
  }
  return buffer;
}

void TopDownWidget::onCopyKeySequencePressed() {
  app_->SetClipboard(
      BuildStringFromIndices(ui_->topDownTreeView->selectionModel()->selectedIndexes()));
}

const QString TopDownWidget::kActionExpandRecursively = QStringLiteral("&Expand recursively");
const QString TopDownWidget::kActionCollapseRecursively = QStringLiteral("&Collapse recursively");
const QString TopDownWidget::kActionCollapseChildrenRecursively =
    QStringLiteral("Collapse children recursively");
const QString TopDownWidget::kActionExpandAll = QStringLiteral("Expand all");
const QString TopDownWidget::kActionCollapseAll = QStringLiteral("Collapse all");
const QString TopDownWidget::kActionLoadSymbols = QStringLiteral("&Load Symbols");
const QString TopDownWidget::kActionSelect = QStringLiteral("&Hook");
const QString TopDownWidget::kActionDeselect = QStringLiteral("&Unhook");
const QString TopDownWidget::kActionDisassembly = QStringLiteral("Go to &Disassembly");
const QString TopDownWidget::kActionCopySelection = QStringLiteral("Copy Selection");

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

static std::vector<std::shared_ptr<Module>> GetModulesFromIndices(
    OrbitApp* app, const std::vector<QModelIndex>& indices) {
  const std::shared_ptr<Process>& process = app->GetCaptureData().process();
  CHECK(process != nullptr);

  std::set<std::string> unique_module_paths;
  for (const auto& index : indices) {
    std::string module_path =
        index.model()
            ->index(index.row(), TopDownViewItemModel::kModule, index.parent())
            .data(TopDownViewItemModel::kModulePathRole)
            .toString()
            .toStdString();
    unique_module_paths.insert(module_path);
  }

  std::vector<std::shared_ptr<Module>> modules;
  for (const std::string& module_path : unique_module_paths) {
    std::shared_ptr<Module> module = process->GetModuleFromPath(module_path);
    if (module != nullptr) {
      modules.emplace_back(std::move(module));
    }
  }
  return modules;
}

static std::vector<FunctionInfo*> GetFunctionsFromIndices(OrbitApp* app,
                                                          const std::vector<QModelIndex>& indices) {
  const std::shared_ptr<Process>& process = app->GetCaptureData().process();
  CHECK(process != nullptr);

  absl::flat_hash_set<FunctionInfo*> functions_set;
  for (const auto& index : indices) {
    uint64_t absolute_address =
        index.model()
            ->index(index.row(), TopDownViewItemModel::kFunctionAddress, index.parent())
            .data(Qt::EditRole)
            .toLongLong();
    FunctionInfo* function = process->GetFunctionFromAddress(absolute_address);
    if (function != nullptr) {
      functions_set.insert(function);
    }
  }
  return std::vector<FunctionInfo*>(functions_set.begin(), functions_set.end());
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

  std::vector<std::shared_ptr<Module>> modules_to_load;
  for (const auto& module : GetModulesFromIndices(app_, selected_tree_indices)) {
    if (!module->IsLoaded()) {
      modules_to_load.push_back(module);
    }
  }
  bool enable_load = !modules_to_load.empty();

  std::vector<FunctionInfo*> functions = GetFunctionsFromIndices(app_, selected_tree_indices);
  bool enable_select = false;
  bool enable_deselect = false;
  for (FunctionInfo* function : functions) {
    enable_select |= !app_->IsFunctionSelected(*function);
    enable_deselect |= app_->IsFunctionSelected(*function);
  }

  bool enable_disassembly = !functions.empty();

  bool enable_copy = ui_->topDownTreeView->selectionModel()->hasSelection();

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
  menu.addSeparator();
  if (enable_load) {
    menu.addAction(kActionLoadSymbols);
  }
  if (enable_select) {
    menu.addAction(kActionSelect);
  }
  if (enable_deselect) {
    menu.addAction(kActionDeselect);
  }
  if (enable_disassembly) {
    menu.addAction(kActionDisassembly);
  }
  menu.addSeparator();
  if (enable_copy) {
    menu.addAction(kActionCopySelection);
  }

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
  } else if (action->text() == kActionLoadSymbols) {
    app_->LoadModules(app_->GetCaptureData().process(), modules_to_load);
  } else if (action->text() == kActionSelect) {
    for (FunctionInfo* function : functions) {
      app_->SelectFunction(*function);
    }
  } else if (action->text() == kActionDeselect) {
    for (FunctionInfo* function : functions) {
      app_->DeselectFunction(*function);
    }
  } else if (action->text() == kActionDisassembly) {
    for (FunctionInfo* function : functions) {
      app_->Disassemble(app_->GetCaptureData().process_id(), *function);
    }
  } else if (action->text() == kActionCopySelection) {
    app_->SetClipboard(
        BuildStringFromIndices(ui_->topDownTreeView->selectionModel()->selectedIndexes()));
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

void TopDownWidget::onSearchLineEditTextEdited(const QString& text) {
  if (search_proxy_model_ == nullptr) {
    return;
  }
  search_proxy_model_->SetFilter(text.toStdString());
  ui_->topDownTreeView->viewport()->update();
  if (!text.isEmpty()) {
    ExpandCollapseBasedOnRole(
        ui_->topDownTreeView,
        TopDownWidget::HighlightCustomFilterSortFilterProxyModel::kMatchesCustomFilterRole);
  }
}

const QColor TopDownWidget::HighlightCustomFilterSortFilterProxyModel::kHighlightColor{Qt::green};

QVariant TopDownWidget::HighlightCustomFilterSortFilterProxyModel::data(const QModelIndex& index,
                                                                        int role) const {
  if (role == Qt::ForegroundRole) {
    if (ItemMatchesFilter(index)) {
      return kHighlightColor;
    }
  } else if (role == kMatchesCustomFilterRole) {
    return ItemMatchesFilter(index);
  }
  return QSortFilterProxyModel::data(index, role);
}

bool TopDownWidget::HighlightCustomFilterSortFilterProxyModel::ItemMatchesFilter(
    const QModelIndex& index) const {
  if (lowercase_filter_tokens_.empty()) {
    return false;
  }
  std::string haystack = absl::AsciiStrToLower(
      index.model()
          ->index(index.row(), TopDownViewItemModel::kThreadOrFunction, index.parent())
          .data()
          .toString()
          .toStdString());
  for (const std::string& token : lowercase_filter_tokens_) {
    if (!absl::StrContains(haystack, token)) {
      return false;
    }
  }
  return true;
}

QVariant TopDownWidget::HookedIdentityProxyModel::data(const QModelIndex& index, int role) const {
  QVariant data = QIdentityProxyModel::data(index, role);
  if ((role != Qt::DisplayRole && role != Qt::ToolTipRole) ||
      index.column() != TopDownViewItemModel::kThreadOrFunction) {
    return data;
  }

  bool is_ulonglong = false;
  uint64_t function_address =
      index.model()
          ->index(index.row(), TopDownViewItemModel::kFunctionAddress, index.parent())
          .data(Qt::EditRole)
          .toULongLong(&is_ulonglong);
  if (!is_ulonglong) {
    // This is the case for a thread node, where "Function address" is empty.
    return data;
  }

  if (!app_->IsFunctionSelected(function_address)) {
    return data;
  }

  if (role == Qt::ToolTipRole) {
    static const QString kTooltipHookedPrefix = QStringLiteral("[HOOKED] ");
    return kTooltipHookedPrefix + data.toString();
  }
  static const QString kDisplayHookedPrefix =
      QStringLiteral("[") + QString::fromStdString(FunctionsDataView::kSelectedFunctionString) +
      QStringLiteral("] ");
  return kDisplayHookedPrefix + data.toString();
}

void TopDownWidget::ProgressBarItemDelegate::paint(QPainter* painter,
                                                   const QStyleOptionViewItem& option,
                                                   const QModelIndex& index) const {
  bool is_float = false;
  float inclusive_percent = index.data(Qt::EditRole).toFloat(&is_float);
  if (!is_float) {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }

  bool highlight =
      index.data(HighlightCustomFilterSortFilterProxyModel::kMatchesCustomFilterRole).toBool();
  if (option.state & QStyle::State_Selected) {
    painter->fillRect(option.rect, option.palette.highlight());
    // Don't highlight the progress bar text when the row is selected, for consistency with the
    // other columns.
    highlight = false;
  }

  QStyleOptionProgressBar option_progress_bar;
  option_progress_bar.rect = option.rect;
  option_progress_bar.palette = option.palette;
  option_progress_bar.minimum = 0;
  option_progress_bar.maximum = 100;
  option_progress_bar.progress = static_cast<int>(round(inclusive_percent));

  const QColor bar_background_color = option.palette.color(QPalette::Disabled, QPalette::Base);
  option_progress_bar.palette.setColor(QPalette::Base, bar_background_color);

  const QColor palette_highlight_color = option.palette.color(QPalette::Highlight);
  static const float kBarColorValueReductionFactor = .3f / .4f;
  const QColor bar_foreground_color = QColor::fromHsv(
      palette_highlight_color.hue(), palette_highlight_color.saturation(),
      static_cast<int>(round(palette_highlight_color.value() * kBarColorValueReductionFactor)));
  option_progress_bar.palette.setColor(QPalette::Highlight, bar_foreground_color);

  option_progress_bar.text = index.data(Qt::DisplayRole).toString();
  option_progress_bar.textVisible = true;

  if (highlight) {
    option_progress_bar.palette.setColor(
        QPalette::Text, HighlightCustomFilterSortFilterProxyModel::kHighlightColor);
    option_progress_bar.palette.setColor(
        QPalette::HighlightedText, HighlightCustomFilterSortFilterProxyModel::kHighlightColor);
  }

  option.widget->style()->drawControl(QStyle::CE_ProgressBar, &option_progress_bar, painter);
}
