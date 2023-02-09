// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/CallTreeWidget.h"

#include <absl/container/flat_hash_set.h>
#include <absl/flags/flag.h>
#include <absl/hash/hash.h>
#include <absl/strings/match.h>
#include <absl/types/span.h>
#include <stddef.h>
#include <stdint.h>

#include <QAbstractItemModel>
#include <QAction>
#include <QColor>
#include <QFlags>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMenu>
#include <QModelIndexList>
#include <QPainter>
#include <QPalette>
#include <QRect>
#include <QSlider>
#include <QStringLiteral>
#include <QStyle>
#include <QStyleOptionProgressBar>
#include <QTimer>
#include <QTreeView>
#include <Qt>
#include <algorithm>
#include <cmath>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>

#include "ClientData/CallstackEvent.h"
#include "ClientData/CaptureData.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleIdentifier.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ModulePathAndBuildId.h"
#include "ClientFlags/ClientFlags.h"
#include "DataViews/FunctionsDataView.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Sort.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitQt/CallTreeViewItemModel.h"
#include "OrbitQt/CustomSignalsTreeView.h"
#include "UtilWidgets/NoticeWidget.h"
#include "absl/flags/internal/flag.h"
#include "ui_CallTreeWidget.h"

using orbit_client_data::CaptureData;
using orbit_client_data::FunctionInfo;
using orbit_client_data::ModuleData;
using orbit_client_data::ModuleIdentifier;
using orbit_client_data::ModuleManager;

[[nodiscard]] static std::optional<float> FloatFromIndex(const QModelIndex& index) {
  bool is_float = false;
  float value = index.data(Qt::EditRole).toFloat(&is_float);
  return is_float ? std::optional<float>(value) : std::optional<float>(std::nullopt);
}

[[nodiscard]] static bool IsSliderEnabled() { return absl::GetFlag(FLAGS_devmode); }

CallTreeWidget::CallTreeWidget(QWidget* parent)
    : QWidget{parent}, ui_{std::make_unique<Ui::CallTreeWidget>()} {
  ui_->setupUi(this);
  ui_->callTreeTreeView->setItemDelegateForColumn(
      CallTreeViewItemModel::kInclusive, new ProgressBarItemDelegate{ui_->callTreeTreeView});
  search_typing_finished_timer_->setSingleShot(true);
  ui_->inspectionNoticeWidget->InitializeAsInspection();

  connect(ui_->inspectionNoticeWidget, &orbit_util_widgets::NoticeWidget::ButtonClicked, this,
          &CallTreeWidget::OnLeaveInspectionButtonClicked);
  connect(ui_->callTreeTreeView, &QTreeView::expanded, this, &CallTreeWidget::OnRowExpanded);
  connect(ui_->callTreeTreeView, &CustomSignalsTreeView::copyKeySequencePressed, this,
          &CallTreeWidget::OnCopyKeySequencePressed);
  connect(ui_->callTreeTreeView, &CustomSignalsTreeView::altKeyAndMousePressed, this,
          &CallTreeWidget::OnAltKeyAndMousePressed);
  connect(ui_->callTreeTreeView, &QTreeView::customContextMenuRequested, this,
          &CallTreeWidget::OnCustomContextMenuRequested);
  connect(ui_->searchLineEdit, &QLineEdit::textEdited, this,
          &CallTreeWidget::OnSearchLineEditTextEdited);
  connect(search_typing_finished_timer_, &QTimer::timeout, this,
          &CallTreeWidget::OnSearchTypingFinishedTimerTimeout);

  if (IsSliderEnabled()) {
    connect(ui_->horizontalSlider, &QSlider::valueChanged, this,
            &CallTreeWidget::OnSliderValueChanged);
  } else {
    ui_->horizontalSlider->setVisible(false);
  }
}

CallTreeWidget::~CallTreeWidget() = default;

void CallTreeWidget::SetCallTreeView(std::shared_ptr<const CallTreeView> call_tree_view,
                                     std::unique_ptr<QIdentityProxyModel> hide_values_proxy_model) {
  ORBIT_CHECK(app_ != nullptr);

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

  OnSearchLineEditTextEdited(ui_->searchLineEdit->text());
  OnSearchTypingFinishedTimerTimeout();

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

  [[nodiscard]] QVariant data(const QModelIndex& proxy_index, int role) const override {
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

  [[nodiscard]] QVariant data(const QModelIndex& proxy_index, int role) const override {
    // Don't show "Of parent" for the first level (the innermost functions).
    if (!proxy_index.parent().isValid() && role == Qt::DisplayRole &&
        proxy_index.column() == CallTreeViewItemModel::kOfParent) {
      return QVariant{};
    }
    return QIdentityProxyModel::data(proxy_index, role);
  }
};

}  // namespace

static void ExpandRecursively(QTreeView* tree_view, const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }
  for (int i = 0; i < index.model()->rowCount(index); ++i) {
    const QModelIndex child = index.model()->index(i, 0, index);
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
    const QModelIndex child = index.model()->index(i, 0, index);
    CollapseRecursively(tree_view, child);
  }
  if (tree_view->isExpanded(index)) {
    tree_view->collapse(index);
  }
}

// Nodes with an inclusive percentage over `expansion_threshold` will be expanded, the other nodes
// are collapsed recursively.
static void ExpandRecursivelyWithThreshold(QTreeView* tree_view, const QModelIndex& index,
                                           float expansion_threshold = 0.f) {
  if (!index.isValid()) {
    return;
  }
  for (int i = 0; i < index.model()->rowCount(index); ++i) {
    const QModelIndex child = index.model()->index(i, 0, index);
    std::optional<float> inclusive_percent = FloatFromIndex(index.sibling(index.row(), 1));
    if (inclusive_percent.has_value() && inclusive_percent.value() > expansion_threshold) {
      if (!tree_view->isExpanded(index)) {
        tree_view->expand(index);
      }
      ExpandRecursivelyWithThreshold(tree_view, child, expansion_threshold);
    } else {
      CollapseRecursively(tree_view, index);
    }
  }
}

void CallTreeWidget::SetTopDownView(std::shared_ptr<const CallTreeView> top_down_view) {
  // Expand recursively if CallTreeView contains information for a single thread.
  bool should_expand = IsSliderEnabled() && top_down_view->GetCallTreeRoot()->thread_count() == 1;

  SetCallTreeView(std::move(top_down_view),
                  std::make_unique<HideValuesForTopDownProxyModel>(nullptr));

  if (should_expand) {
    float expansion_threshold = 100.f - ui_->horizontalSlider->value();
    ExpandRecursivelyWithThreshold(
        ui_->callTreeTreeView, ui_->callTreeTreeView->model()->index(0, 0), expansion_threshold);
  }
}

void CallTreeWidget::SetBottomUpView(std::shared_ptr<const CallTreeView> bottom_up_view) {
  SetCallTreeView(std::move(bottom_up_view),
                  std::make_unique<HideValuesForBottomUpProxyModel>(nullptr));
  // Don't show the "Exclusive" column for the bottom-up tree, it provides no useful information.
  ui_->callTreeTreeView->hideColumn(CallTreeViewItemModel::kExclusive);
}

void CallTreeWidget::SetInspection() { ui_->inspectionNoticeWidget->show(); }

void CallTreeWidget::ClearInspection() { ui_->inspectionNoticeWidget->hide(); }

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

static int ComputeHeightOfSubtreeOfVisibleNodes(QTreeView* tree_view, const QModelIndex& index) {
  if (!tree_view->isExpanded(index)) {
    return 0;
  }

  int depth_difference = 0;
  for (int i = 0; i < index.model()->rowCount(index); ++i) {
    const QModelIndex child = index.model()->index(i, 0, index);
    depth_difference =
        std::max(depth_difference, 1 + ComputeHeightOfSubtreeOfVisibleNodes(tree_view, child));
  }
  return depth_difference;
}

void CallTreeWidget::ResizeThreadOrFunctionColumnToShowVisibleDescendants(
    const QModelIndex& index) {
  int one_based_depth_of_index = 0;
  for (QModelIndex temp_index = index; temp_index.isValid(); temp_index = temp_index.parent()) {
    ++one_based_depth_of_index;
  }

  int one_based_depth_of_deepest_visible_descendant =
      one_based_depth_of_index + ComputeHeightOfSubtreeOfVisibleNodes(ui_->callTreeTreeView, index);

  static constexpr int kMinThreadOrFunctionColumnSizeWithoutIndentation = 100;
  int min_thread_or_function_column_size =
      one_based_depth_of_deepest_visible_descendant * ui_->callTreeTreeView->indentation() +
      kMinThreadOrFunctionColumnSizeWithoutIndentation;
  int actual_thread_or_function_column_size =
      ui_->callTreeTreeView->header()->sectionSize(CallTreeViewItemModel::kThreadOrFunction);

  if (min_thread_or_function_column_size > actual_thread_or_function_column_size) {
    ui_->callTreeTreeView->header()->resizeSection(CallTreeViewItemModel::kThreadOrFunction,
                                                   min_thread_or_function_column_size);
  }
}

void CallTreeWidget::ResizeThreadOrFunctionColumnToShowAllVisibleNodes() {
  for (int i = 0; i < ui_->callTreeTreeView->model()->rowCount(); ++i) {
    ResizeThreadOrFunctionColumnToShowVisibleDescendants(
        ui_->callTreeTreeView->model()->index(i, 0));
  }
}

void CallTreeWidget::OnLeaveInspectionButtonClicked() { app_->ClearInspection(); }

void CallTreeWidget::OnRowExpanded(const QModelIndex& index) {
  if (resize_thread_or_function_column_on_row_expanded_) {
    // Some descendants deeper than the immediate children also become visible if some descendant
    // had already been expanded. But it's possible that the first column was shrunk again after
    // that previous expansion. Instead of (possibly) resizing the first column only based on the
    // depth of the direct children of the expanded row, this function considers the deepest
    // descendant of this row.
    ResizeThreadOrFunctionColumnToShowVisibleDescendants(index);
  }
}

// When expanding a large number of nodes programmatically, we don't want
// ResizeThreadOrFunctionColumnToShowVisibleDescendants to be called for each node, as it's
// computationally expensive.
void CallTreeWidget::DisableThreadOrFunctionColumnResizeOnRowExpanded() {
  resize_thread_or_function_column_on_row_expanded_ = false;
}

void CallTreeWidget::ReEnableThreadOrFunctionColumnResizeOnRowExpanded() {
  resize_thread_or_function_column_on_row_expanded_ = true;
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
    ExpandedState expanded_state = kLeaf;
    std::list<int> ancestor_rows;
    int column = 0;
  };
  std::vector<ItemWithAncestorRows> items;

  // Note: the indices vector is sorted by row in order of selection and then by column in ascending
  // order.
  for (const QModelIndex& index : indices) {
    ItemWithAncestorRows item;
    item.data = index.data(CallTreeViewItemModel::kCopyableValueRole).toString().toStdString();

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

    item.column = index.column();

    items.emplace_back(std::move(item));
  }

  orbit_base::sort(items.begin(), items.end(), [](const ItemWithAncestorRows& row) {
    return std::tie(row.ancestor_rows, row.column);
  });

  constexpr char kFieldSeparator = '\t';
  constexpr char kLineSeparator = '\n';
  std::string buffer;

  // Copy the column headers.
  {
    // We are not always copying all columns. This keeps the index of the current column in the
    // resulting string to copy, which might differ from the index of the current column in the UI.
    int column_in_output = 0;
    for (int column = 0; column < tree_view->model()->columnCount(); ++column) {
      // If this column is not shown in the UI (e.g., it is hidden in the bottom-up view), then
      // don't copy it.
      if (tree_view->isColumnHidden(column)) {
        continue;
      }

      if (column_in_output > 0) {
        buffer.push_back(kFieldSeparator);
      }
      buffer.append(
          tree_view->model()
              ->headerData(column, Qt::Horizontal, CallTreeViewItemModel::kCopyableValueRole)
              .toString()
              .toStdString());
      ++column_in_output;
    }
    buffer.push_back(kLineSeparator);
  }

  // Copy the actual data.
  {
    int column_in_output = 0;
    for (size_t item_index = 0; item_index < items.size(); ++item_index) {
      const ItemWithAncestorRows& item = items[item_index];
      if (item.column == 0) {
        column_in_output = 0;
        if (item_index > 0) {
          buffer.push_back(kLineSeparator);
        }
      }

      if (tree_view->isColumnHidden(item.column)) {
        continue;
      }
      if (column_in_output > 0) {
        buffer.push_back(kFieldSeparator);
      }
      if (item.column == 0) {
        // Keep the tree representation when copying from the first UI column.
        constexpr const char* kNoBreakSpace = "\u00A0";
        for (size_t j = 0; j < 2 * (item.ancestor_rows.size() - 1); ++j) {
          buffer.append(kNoBreakSpace);
        }

        switch (item.expanded_state) {
          case ItemWithAncestorRows::kLeaf:
            buffer.append(kNoBreakSpace).append(kNoBreakSpace);
            break;
          case ItemWithAncestorRows::kExpanded:
            buffer.append("▾").append(kNoBreakSpace);
            break;
          case ItemWithAncestorRows::kCollapsed:
            buffer.append("▸").append(kNoBreakSpace);
            break;
        }
      }
      buffer.append(item.data);
      ++column_in_output;
    }
  }

  return buffer;
}

void CallTreeWidget::OnCopyKeySequencePressed() {
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
const QString CallTreeWidget::kActionInspectCallstacks = QStringLiteral("Inspect these callstacks");
const QString CallTreeWidget::kActionSelectCallstacks = QStringLiteral("Select these callstacks");
const QString CallTreeWidget::kActionCopySelection = QStringLiteral("Copy Selection");

void CallTreeWidget::OnAltKeyAndMousePressed(const QPoint& point) {
  QModelIndex index = ui_->callTreeTreeView->indexAt(point);
  if (!index.isValid()) return;

  if (ui_->callTreeTreeView->isExpanded(index)) {
    CollapseRecursively(ui_->callTreeTreeView, index);
  } else {
    DisableThreadOrFunctionColumnResizeOnRowExpanded();
    ExpandRecursively(ui_->callTreeTreeView, index);
    ReEnableThreadOrFunctionColumnResizeOnRowExpanded();
    ResizeThreadOrFunctionColumnToShowVisibleDescendants(index);
  }
}

static void CollapseChildrenRecursively(QTreeView* tree_view, const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }
  for (int i = 0; i < index.model()->rowCount(index); ++i) {
    const QModelIndex child = index.model()->index(i, 0, index);
    CollapseRecursively(tree_view, child);
  }
}

static std::vector<ModuleData*> GetModulesFromIndices(OrbitApp* app,
                                                      absl::Span<const QModelIndex> indices) {
  absl::flat_hash_set<orbit_client_data::ModulePathAndBuildId> unique_module_paths_and_build_ids;
  for (const auto& index : indices) {
    const QModelIndex model_index =
        index.model()->index(index.row(), CallTreeViewItemModel::kModule, index.parent());
    const std::string module_path =
        model_index.data(CallTreeViewItemModel::kModulePathRole).toString().toStdString();
    const std::string module_build_id =
        model_index.data(CallTreeViewItemModel::kModuleBuildIdRole).toString().toStdString();
    unique_module_paths_and_build_ids.emplace(orbit_client_data::ModulePathAndBuildId{
        .module_path = module_path, .build_id = module_build_id});
  }

  std::vector<ModuleData*> modules;
  for (const orbit_client_data::ModulePathAndBuildId& module_path_and_build_id :
       unique_module_paths_and_build_ids) {
    ModuleData* module = app->GetMutableModuleByModulePathAndBuildId(module_path_and_build_id);
    if (module != nullptr) {
      modules.emplace_back(module);
    }
  }
  return modules;
}

static std::vector<const FunctionInfo*> GetFunctionsFromIndices(
    OrbitApp* app, absl::Span<const QModelIndex> indices) {
  absl::flat_hash_set<const FunctionInfo*> functions_set;
  const CaptureData& capture_data = app->GetCaptureData();
  const ModuleManager* module_manager = app->GetModuleManager();
  for (const auto& index : indices) {
    uint64_t absolute_address =
        index.model()
            ->index(index.row(), CallTreeViewItemModel::kFunctionAddress, index.parent())
            .data(Qt::EditRole)
            .toLongLong();
    const FunctionInfo* function = orbit_client_data::FindFunctionByAddress(
        *capture_data.process(), *module_manager, absolute_address, false);
    if (function != nullptr) {
      functions_set.insert(function);
    }
  }
  return std::vector<const FunctionInfo*>(functions_set.begin(), functions_set.end());
}

namespace {
struct QModelIndexHash {
  size_t operator()(const QModelIndex& o) const {
    // These are the values used in QModelIndex::operator==.
    return absl::HashOf(o.row(), o.column(), o.internalId(), o.model());
  }
};
}  // namespace

static void GetCallstackEventsUnderSelectionRecursively(
    const QModelIndex& index,
    absl::flat_hash_set<orbit_client_data::CallstackEvent>* callstack_events,
    absl::flat_hash_set<QModelIndex, QModelIndexHash>* indices_already_visited) {
  indices_already_visited->emplace(index);

  const auto* index_callstack_events =
      index.data(CallTreeViewItemModel::kExclusiveCallstackEventsRole)
          .value<const std::vector<orbit_client_data::CallstackEvent>*>();
  for (const orbit_client_data::CallstackEvent& index_callstack_event : *index_callstack_events) {
    callstack_events->emplace(index_callstack_event);
  }

  for (int i = 0; i < index.model()->rowCount(index); ++i) {
    const QModelIndex child = index.model()->index(i, 0, index);
    if (!indices_already_visited->contains(child)) {
      GetCallstackEventsUnderSelectionRecursively(child, callstack_events, indices_already_visited);
    }
  }
}

static absl::flat_hash_set<orbit_client_data::CallstackEvent> GetCallstackEventsUnderSelection(
    absl::Span<const QModelIndex> indices) {
  // We can have duplicate CallstackEvents in the selection, e.g., with the top-down view when
  // selecting both from the "(all threads)" tree and other single-thread trees. Hence the set.
  absl::flat_hash_set<orbit_client_data::CallstackEvent> callstack_events;
  // Prevent visiting nodes multiple times when the selection contains more than one row.
  absl::flat_hash_set<QModelIndex, QModelIndexHash> indices_already_visited;
  for (const QModelIndex& index : indices) {
    if (!indices_already_visited.contains(index)) {
      GetCallstackEventsUnderSelectionRecursively(index, &callstack_events,
                                                  &indices_already_visited);
    }
  }
  return callstack_events;
}

void CallTreeWidget::OnCustomContextMenuRequested(const QPoint& point) {
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
    if (!module->AreDebugSymbolsLoaded()) {
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
      enable_select |= !app_->IsFunctionSelected(*function) && function->IsFunctionSelectable();
      enable_deselect |= app_->IsFunctionSelected(*function);
      enable_disassembly = true;
      enable_source_code = true;
    }
  }

  bool enable_select_callstacks = ui_->callTreeTreeView->selectionModel()->hasSelection();
  bool enable_copy = ui_->callTreeTreeView->selectionModel()->hasSelection();

  QMenu menu{ui_->callTreeTreeView};
  static const QString kAltClickShortcut = QStringLiteral("\tALT+Click");
  bool is_expanded = ui_->callTreeTreeView->isExpanded(index);
  QString action_expand_recursively = (!is_expanded && enable_expand_recursively)
                                          ? kActionExpandRecursively + kAltClickShortcut
                                          : kActionExpandRecursively;
  menu.addAction(action_expand_recursively)->setEnabled(enable_expand_recursively);
  QString action_collapse_recursively = (is_expanded && enable_collapse_recursively)
                                            ? kActionCollapseRecursively + kAltClickShortcut
                                            : kActionCollapseRecursively;
  menu.addAction(action_collapse_recursively)->setEnabled(enable_collapse_recursively);
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
  if (absl::GetFlag(FLAGS_time_range_selection)) {
    menu.addAction(kActionInspectCallstacks)->setEnabled(enable_select_callstacks);
  } else {
    menu.addAction(kActionSelectCallstacks)->setEnabled(enable_select_callstacks);
  }
  menu.addSeparator();
  menu.addAction(kActionCopySelection)->setEnabled(enable_copy);

  QAction* action = menu.exec(ui_->callTreeTreeView->mapToGlobal(point));
  if (action == nullptr) {
    return;
  }

  if (action->text() == action_expand_recursively) {
    for (const QModelIndex& selected_index : selected_tree_indices) {
      DisableThreadOrFunctionColumnResizeOnRowExpanded();
      ExpandRecursively(ui_->callTreeTreeView, selected_index);
      ReEnableThreadOrFunctionColumnResizeOnRowExpanded();
      ResizeThreadOrFunctionColumnToShowVisibleDescendants(selected_index);
    }
  } else if (action->text() == action_collapse_recursively) {
    for (const QModelIndex& selected_index : selected_tree_indices) {
      CollapseRecursively(ui_->callTreeTreeView, selected_index);
    }
  } else if (action->text() == kActionCollapseChildrenRecursively) {
    for (const QModelIndex& selected_index : selected_tree_indices) {
      CollapseChildrenRecursively(ui_->callTreeTreeView, selected_index);
    }
  } else if (action->text() == kActionExpandAll) {
    DisableThreadOrFunctionColumnResizeOnRowExpanded();
    ui_->callTreeTreeView->expandAll();
    ReEnableThreadOrFunctionColumnResizeOnRowExpanded();
    ResizeThreadOrFunctionColumnToShowAllVisibleNodes();
  } else if (action->text() == kActionCollapseAll) {
    ui_->callTreeTreeView->collapseAll();
  } else if (action->text() == kActionLoadSymbols) {
    app_->LoadSymbolsManually(modules_to_load);
  } else if (action->text() == kActionSelect) {
    for (const FunctionInfo* function : functions) {
      app_->SelectFunction(*function);
    }
  } else if (action->text() == kActionDeselect) {
    for (const FunctionInfo* function : functions) {
      app_->DeselectFunction(*function);
    }
  } else if (action->text() == kActionDisassembly) {
    constexpr int kMaxNumberOfWindowsToOpen = 10;
    int i = 0;
    for (const FunctionInfo* function : functions) {
      app_->Disassemble(app_->GetCaptureData().process_id(), *function);
      if (++i >= kMaxNumberOfWindowsToOpen) break;
    }
  } else if (action->text() == kActionSourceCode) {
    constexpr int kMaxNumberOfWindowsToOpen = 10;
    int i = 0;
    for (const FunctionInfo* function : functions) {
      app_->ShowSourceCode(*function);
      if (++i >= kMaxNumberOfWindowsToOpen) break;
    }
  } else if (action->text() == kActionInspectCallstacks) {
    absl::flat_hash_set<orbit_client_data::CallstackEvent> selected_callstack_events =
        GetCallstackEventsUnderSelection(selected_tree_indices);
    ORBIT_CHECK(!selected_callstack_events.empty());
    app_->InspectCallstackEvents(
        // This copies the content of the absl::flat_hash_set into a std::vector. We consider this
        // fine in order to keep OrbitApp::InspectCallstackEvents as simple as it is now.
        std::vector<orbit_client_data::CallstackEvent>{selected_callstack_events.begin(),
                                                       selected_callstack_events.end()});
  } else if (action->text() == kActionSelectCallstacks) {
    absl::flat_hash_set<orbit_client_data::CallstackEvent> selected_callstack_events =
        GetCallstackEventsUnderSelection(selected_tree_indices);
    ORBIT_CHECK(!selected_callstack_events.empty());
    app_->SelectCallstackEvents(
        // This copies the content of the absl::flat_hash_set into a std::vector. We consider this
        // fine in order to keep OrbitApp::SelectCallstackEvents as simple as it is now.
        std::vector<orbit_client_data::CallstackEvent>{selected_callstack_events.begin(),
                                                       selected_callstack_events.end()});
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
    const QModelIndex child = index.model()->index(i, 0, index);
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
    const QModelIndex child = tree_view->model()->index(i, 0);
    ExpandCollapseRecursivelyBasedOnDescendantsRole(tree_view, child, role);
  }
}

void CallTreeWidget::OnSearchLineEditTextEdited(const QString& /*text*/) {
  static constexpr int kSearchTypingFinishedTimerTimeoutMs = 400;
  search_typing_finished_timer_->start(kSearchTypingFinishedTimerTimeoutMs);
}

void CallTreeWidget::OnSearchTypingFinishedTimerTimeout() {
  if (search_proxy_model_ == nullptr) {
    return;
  }
  const std::string search_text = ui_->searchLineEdit->text().toStdString();
  search_proxy_model_->SetFilter(search_text);
  ui_->callTreeTreeView->viewport()->update();
  if (!search_text.empty()) {
    DisableThreadOrFunctionColumnResizeOnRowExpanded();
    ExpandCollapseBasedOnRole(ui_->callTreeTreeView,
                              CallTreeViewItemModel::kMatchesCustomFilterRole);
    ReEnableThreadOrFunctionColumnResizeOnRowExpanded();
    ResizeThreadOrFunctionColumnToShowAllVisibleNodes();
  }
}

void CallTreeWidget::OnSliderValueChanged(int value) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_CHECK(value >= 0);
  ORBIT_CHECK(value <= 100);
  ExpandRecursivelyWithThreshold(ui_->callTreeTreeView, ui_->callTreeTreeView->model()->index(0, 0),
                                 100 - value);
}

const QColor CallTreeWidget::HighlightCustomFilterSortFilterProxyModel::kHighlightColor{Qt::green};

QVariant CallTreeWidget::HighlightCustomFilterSortFilterProxyModel::data(const QModelIndex& index,
                                                                         int role) const {
  if (role == Qt::ForegroundRole) {
    if (ItemMatchesFilter(index)) {
      return kHighlightColor;
    }
  } else if (role == CallTreeViewItemModel::kMatchesCustomFilterRole) {
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
      QStringLiteral("[") +
      QString::fromStdString(orbit_data_views::FunctionsDataView::kSelectedFunctionString) +
      QStringLiteral("] ");
  return kDisplayHookedPrefix + data.toString();
}

void CallTreeWidget::ProgressBarItemDelegate::paint(QPainter* painter,
                                                    const QStyleOptionViewItem& option,
                                                    const QModelIndex& index) const {
  std::optional<float> inclusive_percent = FloatFromIndex(index);
  if (!inclusive_percent.has_value()) {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }

  bool highlight = index.data(CallTreeViewItemModel::kMatchesCustomFilterRole).toBool();
  if ((option.state & QStyle::State_Selected) != 0u) {
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
  option_progress_bar.progress = static_cast<int>(std::round(inclusive_percent.value()));

  const QColor bar_background_color = option.palette.color(QPalette::Disabled, QPalette::Base);
  option_progress_bar.palette.setColor(QPalette::Base, bar_background_color);

  const QColor palette_highlight_color = option.palette.color(QPalette::Highlight);
  static constexpr float kBarColorValueReductionFactor = .3f / .4f;
  const QColor bar_foreground_color =
      QColor::fromHsv(palette_highlight_color.hue(), palette_highlight_color.saturation(),
                      static_cast<int>(std::round(palette_highlight_color.value() *
                                                  kBarColorValueReductionFactor)));
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
