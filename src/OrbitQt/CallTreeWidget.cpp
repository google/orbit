// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CallTreeWidget.h"

#include <absl/container/flat_hash_set.h>
#include <absl/flags/flag.h>
#include <absl/strings/match.h>
#include <math.h>
#include <stdint.h>

#include <QAbstractItemModel>
#include <QAction>
#include <QColor>
#include <QFlags>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QModelIndexList>
#include <QPainter>
#include <QPalette>
#include <QRect>
#include <QStaticStringData>
#include <QStringLiteral>
#include <QStyle>
#include <QStyleOptionProgressBar>
#include <QTreeView>
#include <algorithm>
#include <list>
#include <optional>
#include <set>
#include <utility>

#include "App.h"
#include "CallTreeViewItemModel.h"
#include "ClientModel/CaptureData.h"
#include "CopyKeySequenceEnabledTreeView.h"
#include "FunctionsDataView.h"
#include "OrbitBase/Logging.h"
#include "OrbitClientData/ModuleData.h"
#include "capture_data.pb.h"

using orbit_client_protos::FunctionInfo;

ABSL_DECLARE_FLAG(bool, enable_source_code_view);

CallTreeWidget::CallTreeWidget(QWidget* parent)
    : QWidget{parent}, ui_{std::make_unique<Ui::CallTreeWidget>()} {
  ui_->setupUi(this);
  ui_->callTreeTreeView->setItemDelegateForColumn(
      CallTreeViewItemModel::kInclusive, new ProgressBarItemDelegate{ui_->callTreeTreeView});

  connect(ui_->callTreeTreeView, &CopyKeySequenceEnabledTreeView::copyKeySequencePressed, this,
          &CallTreeWidget::onCopyKeySequencePressed);
  connect(ui_->callTreeTreeView, &QTreeView::customContextMenuRequested, this,
          &CallTreeWidget::onCustomContextMenuRequested);
  connect(ui_->searchLineEdit, &QLineEdit::textEdited, this,
          &CallTreeWidget::onSearchLineEditTextEdited);
}

void CallTreeWidget::SetCallTreeView(std::unique_ptr<CallTreeView> call_tree_view,
                                     std::unique_ptr<QIdentityProxyModel> hide_values_proxy_model) {
  CHECK(app_ != nullptr);

  model_ = std::make_unique<CallTreeViewItemModel>(std::move(call_tree_view));

  hide_values_proxy_model_ = std::move(hide_values_proxy_model);
  hide_values_proxy_model_->setSourceModel(model_.get());

  search_proxy_model_ = std::make_unique<HighlightCustomFilterSortFilterProxyModel>(nullptr);
  search_proxy_model_->setSourceModel(hide_values_proxy_model_.get());
  search_proxy_model_->setSortRole(Qt::EditRole);

  hooked_proxy_model_ = std::make_unique<HookedIdentityProxyModel>(app_, nullptr);
  hooked_proxy_model_->setSourceModel(search_proxy_model_.get());

  ui_->callTreeTreeView->setModel(hooked_proxy_model_.get());
  ui_->callTreeTreeView->sortByColumn(CallTreeViewItemModel::kInclusive, Qt::DescendingOrder);

  onSearchLineEditTextEdited(ui_->searchLineEdit->text());

  ResizeColumnsIfNecessary();
}

void CallTreeWidget::ClearCallTreeView() {
  hooked_proxy_model_.reset();
  search_proxy_model_.reset();
  hide_values_proxy_model_.reset();
  model_.reset();
}

namespace {

class HideValuesForTopDownProxyModel : public QIdentityProxyModel {
 public:
  explicit HideValuesForTopDownProxyModel(QObject* parent) : QIdentityProxyModel(parent) {}

  QVariant data(const QModelIndex& proxy_index, int role) const override {
    // Don't show "Exclusive" and "Of parent" for the first level (the thread level).
    if (!proxy_index.parent().isValid() && role == Qt::DisplayRole &&
        (proxy_index.column() == CallTreeViewItemModel::kExclusive ||
         proxy_index.column() == CallTreeViewItemModel::kOfParent)) {
      return QVariant{};
    }
    return QIdentityProxyModel::data(proxy_index, role);
  }
};

class HideValuesForBottomUpProxyModel : public QIdentityProxyModel {
 public:
  explicit HideValuesForBottomUpProxyModel(QObject* parent) : QIdentityProxyModel(parent) {}

  QVariant data(const QModelIndex& proxy_index, int role) const override {
    // Don't show "Of parent" for the first level (the innermost functions).
    if (!proxy_index.parent().isValid() && role == Qt::DisplayRole &&
        proxy_index.column() == CallTreeViewItemModel::kOfParent) {
      return QVariant{};
    }
    return QIdentityProxyModel::data(proxy_index, role);
  }
};

}  // namespace

void CallTreeWidget::SetTopDownView(std::unique_ptr<CallTreeView> top_down_view) {
  SetCallTreeView(std::move(top_down_view),
                  std::make_unique<HideValuesForTopDownProxyModel>(nullptr));
}

void CallTreeWidget::SetBottomUpView(std::unique_ptr<CallTreeView> bottom_up_view) {
  SetCallTreeView(std::move(bottom_up_view),
                  std::make_unique<HideValuesForBottomUpProxyModel>(nullptr));
  // Don't show the "Exclusive" column for the bottom-up tree, it provides no useful information.
  ui_->callTreeTreeView->hideColumn(CallTreeViewItemModel::kExclusive);
}

void CallTreeWidget::resizeEvent(QResizeEvent* /*event*/) {
  if (column_resizing_state_ == ColumnResizingState::kInitial) {
    column_resizing_state_ = ColumnResizingState::kWidgetSizeSet;
    ResizeColumnsIfNecessary();
  }
}

void CallTreeWidget::ResizeColumnsIfNecessary() {
  // Resize columns only once, when the following holds:
  // - a non-empty CallTreeView is set;
  // - the size of the QTreeView has been set, which means that the size of CallTreeWidget has been
  //   set, which in turns means that the tab has been shown at least once.
  if (column_resizing_state_ != ColumnResizingState::kWidgetSizeSet ||
      hooked_proxy_model_ == nullptr || hooked_proxy_model_->rowCount(QModelIndex{}) == 0) {
    return;
  }

  ui_->callTreeTreeView->header()->setStretchLastSection(false);
  ui_->callTreeTreeView->header()->resizeSections(QHeaderView::ResizeToContents);
  // Shrink the first column (to a minimum width) so that all columns are visible,
  // but still make it as wide as possible.
  static constexpr int kMinThreadOrFunctionColumnSize = 200;
  int thread_or_function_column_size = ui_->callTreeTreeView->header()->width();
  for (int column = 0; column < ui_->callTreeTreeView->header()->count(); ++column) {
    if (column != CallTreeViewItemModel::kThreadOrFunction) {
      thread_or_function_column_size -= ui_->callTreeTreeView->header()->sectionSize(column);
    }
  }
  thread_or_function_column_size =
      std::max(kMinThreadOrFunctionColumnSize, thread_or_function_column_size);
  ui_->callTreeTreeView->header()->resizeSection(CallTreeViewItemModel::kThreadOrFunction,
                                                 thread_or_function_column_size);
  ui_->callTreeTreeView->header()->setStretchLastSection(true);

  column_resizing_state_ = ColumnResizingState::kDone;
}

static std::string BuildStringFromIndices(QTreeView* tree_view, const QModelIndexList& indices) {
  // The order of the `QModelIndex`es in `indices` reflects the order of selection, not the order in
  // the tree. In other words, the order of the rows in the selection model is not the order in
  // which the rows are displayed.
  // To recover the desired order, for each `QModelIndex` in indices compute the list of
  // `QModelIndex::row`s from the root `QModelIndex` down to that `QModelIndex`.
  // The lexicographic order of these lists gives the order in the tree.
  struct ItemWithAncestorRows {
    enum ExpandedState { kLeaf = 0, kExpanded, kCollapsed };

    std::string data;
    bool is_first_in_row = false;
    ExpandedState expanded_state = kLeaf;
    std::list<int> ancestor_rows;
  };
  std::vector<ItemWithAncestorRows> items;

  std::optional<QModelIndex> prev_index;
  // Note: the indices vector is sorted by row in order of selection and then by column in ascending
  // order.
  for (const QModelIndex& index : indices) {
    ItemWithAncestorRows item;
    item.data = index.data().toString().toStdString();

    // row() is the position among siblings: also compare parent().
    item.is_first_in_row = !prev_index.has_value() || index.row() != prev_index->row() ||
                           index.parent() != prev_index->parent();

    if (index.model()->rowCount(index) == 0) {
      item.expanded_state = ItemWithAncestorRows::kLeaf;
    } else if (tree_view->isExpanded(index)) {
      item.expanded_state = ItemWithAncestorRows::kExpanded;
    } else {
      item.expanded_state = ItemWithAncestorRows::kCollapsed;
    }

    for (QModelIndex temp_index = index; temp_index.isValid(); temp_index = temp_index.parent()) {
      item.ancestor_rows.emplace_front(temp_index.row());
    }

    items.emplace_back(std::move(item));
    prev_index = index;
  }

  // Use stable_sort to maintain the order in the same row (i.e., across columns).
  std::stable_sort(items.begin(), items.end(),
                   [](const ItemWithAncestorRows& lhs, const ItemWithAncestorRows& rhs) {
                     return lhs.ancestor_rows < rhs.ancestor_rows;
                   });
  std::string buffer;
  for (size_t i = 0; i < items.size(); ++i) {
    const ItemWithAncestorRows& item = items[i];
    if (item.is_first_in_row) {
      if (i > 0) {
        buffer.push_back('\n');
      }

      buffer.append(2 * (item.ancestor_rows.size() - 1), ' ');

      switch (item.expanded_state) {
        case ItemWithAncestorRows::kLeaf:
          buffer.append("  ");
          break;
        case ItemWithAncestorRows::kExpanded:
          buffer.append("- ");
          break;
        case ItemWithAncestorRows::kCollapsed:
          buffer.append("+ ");
          break;
      }
    } else {
      buffer.append(", ");
    }
    buffer.append(item.data);
  }
  return buffer;
}

void CallTreeWidget::onCopyKeySequencePressed() {
  app_->SetClipboard(BuildStringFromIndices(
      ui_->callTreeTreeView, ui_->callTreeTreeView->selectionModel()->selectedIndexes()));
}

const QString CallTreeWidget::kActionExpandRecursively = QStringLiteral("&Expand recursively");
const QString CallTreeWidget::kActionCollapseRecursively = QStringLiteral("&Collapse recursively");
const QString CallTreeWidget::kActionCollapseChildrenRecursively =
    QStringLiteral("Collapse children recursively");
const QString CallTreeWidget::kActionExpandAll = QStringLiteral("Expand all");
const QString CallTreeWidget::kActionCollapseAll = QStringLiteral("Collapse all");
const QString CallTreeWidget::kActionLoadSymbols = QStringLiteral("&Load Symbols");
const QString CallTreeWidget::kActionSelect = QStringLiteral("&Hook");
const QString CallTreeWidget::kActionDeselect = QStringLiteral("&Unhook");
const QString CallTreeWidget::kActionDisassembly = QStringLiteral("Go to &Disassembly");
const QString CallTreeWidget::kActionSourceCode = QStringLiteral("Go to &Source Code");
const QString CallTreeWidget::kActionCopySelection = QStringLiteral("Copy Selection");

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

static std::vector<ModuleData*> GetModulesFromIndices(OrbitApp* app,
                                                      const std::vector<QModelIndex>& indices) {
  std::set<std::pair<std::string, std::string>> unique_module_paths_and_build_ids;
  for (const auto& index : indices) {
    QModelIndex model_index =
        index.model()->index(index.row(), CallTreeViewItemModel::kModule, index.parent());
    std::string module_path =
        model_index.data(CallTreeViewItemModel::kModulePathRole).toString().toStdString();
    std::string module_build_id =
        model_index.data(CallTreeViewItemModel::kModuleBuildIdRole).toString().toStdString();
    unique_module_paths_and_build_ids.emplace(module_path, module_build_id);
  }

  std::vector<ModuleData*> modules;
  for (const auto& [module_path, module_build_id] : unique_module_paths_and_build_ids) {
    ModuleData* module = app->GetMutableModuleByPathAndBuildId(module_path, module_build_id);
    if (module != nullptr) {
      modules.emplace_back(module);
    }
  }
  return modules;
}

static std::vector<const FunctionInfo*> GetFunctionsFromIndices(
    OrbitApp* app, const std::vector<QModelIndex>& indices) {
  absl::flat_hash_set<const FunctionInfo*> functions_set;
  const CaptureData& capture_data = app->GetCaptureData();
  for (const auto& index : indices) {
    uint64_t absolute_address =
        index.model()
            ->index(index.row(), CallTreeViewItemModel::kFunctionAddress, index.parent())
            .data(Qt::EditRole)
            .toLongLong();
    const FunctionInfo* function = capture_data.FindFunctionByAddress(absolute_address, false);
    if (function != nullptr) {
      functions_set.insert(function);
    }
  }
  return std::vector<const FunctionInfo*>(functions_set.begin(), functions_set.end());
}

void CallTreeWidget::onCustomContextMenuRequested(const QPoint& point) {
  if (app_ == nullptr) {
    return;
  }

  QModelIndex index = ui_->callTreeTreeView->indexAt(point);
  if (!index.isValid()) {
    return;
  }

  std::vector<QModelIndex> selected_tree_indices;
  for (const QModelIndex& selected_index :
       ui_->callTreeTreeView->selectionModel()->selectedIndexes()) {
    if (selected_index.column() != CallTreeViewItemModel::kThreadOrFunction) {
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
      if (ui_->callTreeTreeView->isExpanded(selected_index)) {
        enable_collapse_recursively = true;
      }
    }
  }

  std::vector<ModuleData*> modules_to_load;
  for (const auto& module : GetModulesFromIndices(app_, selected_tree_indices)) {
    if (!module->is_loaded()) {
      modules_to_load.push_back(module);
    }
  }
  bool enable_load = !modules_to_load.empty();

  std::vector<const FunctionInfo*> functions = GetFunctionsFromIndices(app_, selected_tree_indices);
  bool enable_select = false;
  bool enable_deselect = false;
  bool enable_disassembly = false;
  bool enable_source_code = false;
  if (app_->IsCaptureConnected(app_->GetCaptureData())) {
    for (const FunctionInfo* function : functions) {
      enable_select |= !app_->IsFunctionSelected(*function);
      enable_deselect |= app_->IsFunctionSelected(*function);
      enable_disassembly = true;
      enable_source_code = absl::GetFlag(FLAGS_enable_source_code_view);
    }
  }

  bool enable_copy = ui_->callTreeTreeView->selectionModel()->hasSelection();

  QMenu menu{ui_->callTreeTreeView};
  menu.addAction(kActionExpandRecursively)->setEnabled(enable_expand_recursively);
  menu.addAction(kActionCollapseRecursively)->setEnabled(enable_collapse_recursively);
  menu.addAction(kActionCollapseChildrenRecursively)->setEnabled(enable_collapse_recursively);
  menu.addSeparator();
  menu.addAction(kActionExpandAll);
  menu.addAction(kActionCollapseAll);
  menu.addSeparator();
  menu.addAction(kActionLoadSymbols)->setEnabled(enable_load);
  menu.addAction(kActionSelect)->setEnabled(enable_select);
  menu.addAction(kActionDeselect)->setEnabled(enable_deselect);
  menu.addAction(kActionDisassembly)->setEnabled(enable_disassembly);
  menu.addAction(kActionSourceCode)->setEnabled(enable_source_code);
  menu.addSeparator();
  menu.addAction(kActionCopySelection)->setEnabled(enable_copy);

  QAction* action = menu.exec(ui_->callTreeTreeView->mapToGlobal(point));
  if (action == nullptr) {
    return;
  }

  if (action->text() == kActionExpandRecursively) {
    for (const QModelIndex& selected_index : selected_tree_indices) {
      ExpandRecursively(ui_->callTreeTreeView, selected_index);
    }
  } else if (action->text() == kActionCollapseRecursively) {
    for (const QModelIndex& selected_index : selected_tree_indices) {
      CollapseRecursively(ui_->callTreeTreeView, selected_index);
    }
  } else if (action->text() == kActionCollapseChildrenRecursively) {
    for (const QModelIndex& selected_index : selected_tree_indices) {
      CollapseChildrenRecursively(ui_->callTreeTreeView, selected_index);
    }
  } else if (action->text() == kActionExpandAll) {
    ui_->callTreeTreeView->expandAll();
  } else if (action->text() == kActionCollapseAll) {
    ui_->callTreeTreeView->collapseAll();
  } else if (action->text() == kActionLoadSymbols) {
    app_->RetrieveModulesAndLoadSymbols(modules_to_load);
  } else if (action->text() == kActionSelect) {
    for (const FunctionInfo* function : functions) {
      app_->SelectFunction(*function);
    }
  } else if (action->text() == kActionDeselect) {
    for (const FunctionInfo* function : functions) {
      app_->DeselectFunction(*function);
    }
  } else if (action->text() == kActionDisassembly) {
    for (const FunctionInfo* function : functions) {
      app_->Disassemble(app_->GetCaptureData().process_id(), *function);
    }
  } else if (action->text() == kActionSourceCode) {
    for (const FunctionInfo* function : functions) {
      app_->ShowSourceCode(*function);
    }
  } else if (action->text() == kActionCopySelection) {
    app_->SetClipboard(BuildStringFromIndices(
        ui_->callTreeTreeView, ui_->callTreeTreeView->selectionModel()->selectedIndexes()));
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

void CallTreeWidget::onSearchLineEditTextEdited(const QString& text) {
  if (search_proxy_model_ == nullptr) {
    return;
  }
  search_proxy_model_->SetFilter(text.toStdString());
  ui_->callTreeTreeView->viewport()->update();
  if (!text.isEmpty()) {
    ExpandCollapseBasedOnRole(
        ui_->callTreeTreeView,
        CallTreeWidget::HighlightCustomFilterSortFilterProxyModel::kMatchesCustomFilterRole);
  }
}

const QColor CallTreeWidget::HighlightCustomFilterSortFilterProxyModel::kHighlightColor{Qt::green};

QVariant CallTreeWidget::HighlightCustomFilterSortFilterProxyModel::data(const QModelIndex& index,
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

bool CallTreeWidget::HighlightCustomFilterSortFilterProxyModel::ItemMatchesFilter(
    const QModelIndex& index) const {
  if (lowercase_filter_tokens_.empty()) {
    return false;
  }
  std::string haystack = absl::AsciiStrToLower(
      index.model()
          ->index(index.row(), CallTreeViewItemModel::kThreadOrFunction, index.parent())
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

QVariant CallTreeWidget::HookedIdentityProxyModel::data(const QModelIndex& index, int role) const {
  QVariant data = QIdentityProxyModel::data(index, role);
  if ((role != Qt::DisplayRole && role != Qt::ToolTipRole) ||
      index.column() != CallTreeViewItemModel::kThreadOrFunction) {
    return data;
  }

  bool is_ulonglong = false;
  uint64_t function_address =
      index.model()
          ->index(index.row(), CallTreeViewItemModel::kFunctionAddress, index.parent())
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

void CallTreeWidget::ProgressBarItemDelegate::paint(QPainter* painter,
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
