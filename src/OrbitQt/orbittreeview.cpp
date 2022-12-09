// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/orbittreeview.h"

#include <QAction>
#include <QFont>
#include <QFontDatabase>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMenu>
#include <QModelIndexList>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSize>
#include <algorithm>
#include <cstddef>
#include <set>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "DataViews/DataView.h"
#include "OrbitBase/Logging.h"

OrbitTreeView::OrbitTreeView(QWidget* parent) : QTreeView(parent) {
  header()->setSortIndicatorShown(true);
  header()->setSectionsClickable(true);

  setRootIsDecorated(false);
  setItemsExpandable(false);
  setContextMenuPolicy(Qt::CustomContextMenu);
  setSelectionBehavior(QTreeView::SelectRows);
  setTextElideMode(Qt::ElideMiddle);

  connect(header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this,
          SLOT(OnSort(int, Qt::SortOrder)));

  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this,
          SLOT(ShowContextMenu(const QPoint&)));

  connect(header(), SIGNAL(sectionResized(int, int, int)), this,
          SLOT(columnResized(int, int, int)));

  connect(verticalScrollBar(), SIGNAL(rangeChanged(int, int)), this,
          SLOT(OnRangeChanged(int, int)));

  connect(this, &OrbitTreeView::doubleClicked, this, &OrbitTreeView::OnDoubleClicked);
}

void OrbitTreeView::Initialize(orbit_data_views::DataView* data_view, SelectionType selection_type,
                               FontType font_type, bool uniform_row_height,
                               QFlags<Qt::AlignmentFlag> text_alignment) {
  ORBIT_SCOPE("OrbitTreeView::Initialize");
  setUniformRowHeights(uniform_row_height);

  model_ = std::make_unique<OrbitTableModel>(data_view, /*parent=*/nullptr, text_alignment);
  setModel(model_.get());
  {
    ORBIT_SCOPE("resizeSections");
    header()->resizeSections(QHeaderView::ResizeToContents);
  }

  if (!model_->IsSortingAllowed()) {
    // Don't do setSortingEnabled(model_->IsSortingAllowed()); as with true it
    // forces a sort by the first column.
    setSortingEnabled(false);
  } else {
    ORBIT_SCOPE("sortByColumn");
    std::pair<int, Qt::SortOrder> column_and_order = model_->GetDefaultSortingColumnAndOrder();
    sortByColumn(column_and_order.first, column_and_order.second);
  }

  if (model_->GetUpdatePeriodMs() > 0) {
    timer_ = std::make_unique<QTimer>();
    connect(timer_.get(), SIGNAL(timeout()), this, SLOT(OnTimer()));
    timer_->start(model_->GetUpdatePeriodMs());
  }

  if (selection_type == SelectionType::kExtended) {
    setSelectionMode(ExtendedSelection);
  }

  setAlternatingRowColors(true);

  if (font_type == FontType::kFixed) {
    const QFont fixed_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    setFont(fixed_font);
  }
  // After a round of Deinitialize() and Initialize(), the table column sizes get reset and
  // resizeEvent doesn't get called again. Manually trigger the event here to make sure they get set
  // properly.
  QResizeEvent event = QResizeEvent(size(), QSize{-1, -1});
  resizeEvent(&event);
}

void OrbitTreeView::Deinitialize() {
  timer_.reset();
  setModel(nullptr);
  model_.reset();
}

void OrbitTreeView::SetDataModel(orbit_data_views::DataView* data_view) {
  model_ = std::make_unique<OrbitTableModel>();
  model_->SetDataView(data_view);
  setModel(model_.get());
}

void OrbitTreeView::ClearDataModel() {
  setModel(nullptr);
  model_.reset();
}

void OrbitTreeView::OnSort(int section, Qt::SortOrder order) {
  if (model_ == nullptr) {
    return;
  }

  model_->sort(section, order);
  Refresh(RefreshMode::kOnSort);
}

void OrbitTreeView::OnFilter(const QString& filter) {
  if (model_ == nullptr) {
    return;
  }

  model_->OnFilter(filter);
  Refresh(RefreshMode::kOnFilter);
}

void OrbitTreeView::OnTimer() {
  if (model_ != nullptr && isVisible() && !model_->GetDataView()->SkipTimer()) {
    model_->OnTimer();
    Refresh();
  }
}

void OrbitTreeView::Refresh(RefreshMode refresh_mode) {
  if (model_ == nullptr) {
    return;
  }

  if (model_->GetDataView()->ResetOnRefresh()) {
    reset();
  } else {
    model_->layoutAboutToBeChanged();
    model_->layoutChanged();
    // Skip the following re-selection unless the refresh is caused by filtering or sorting.
    if (refresh_mode == RefreshMode::kOther) return;
  }

  // Re-select previous selection
  // Don't re-trigger row selection callback when re-selecting.
  is_internal_refresh_ = true;
  std::vector<int> visible_selected_indices = model_->GetVisibleSelectedIndices();

  QModelIndex index;
  QItemSelection selection{};
  for (int row : visible_selected_indices) {
    index = model_->CreateIndex(row, 0);
    selection.select(index, index);
  }
  selectionModel()->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows |
                                          QItemSelectionModel::Clear);

  is_internal_refresh_ = false;

  model_->GetDataView()->OnRefresh(visible_selected_indices, refresh_mode);
}

void OrbitTreeView::resizeEvent(QResizeEvent* event) {
  const bool width_resized = event->size().width() != event->oldSize().width();
  if (width_resized && model_ != nullptr && model_->GetDataView() != nullptr) {
    // Get initial column ratios once.
    if (column_ratios_.size() == 0) {
      for (const auto& column : model_->GetDataView()->GetColumns()) {
        column_ratios_.emplace_back(column.ratio);
      }
    }

    // Resize columns based on current ratios.
    float header_width = static_cast<float>(size().width());
    for (size_t i = 0; i < column_ratios_.size(); ++i) {
      float ratio = column_ratios_[i];
      if (ratio > 0.f) {
        header()->resizeSection(i, static_cast<int>(header_width * ratio));
      }
    }
  }

  QTreeView::resizeEvent(event);
}

void OrbitTreeView::Link(OrbitTreeView* link) {
  links_.push_back(link);

  orbit_data_views::DataView* data_view = link->GetModel()->GetDataView();
  model_->GetDataView()->LinkDataView(data_view);
}

void OrbitTreeView::ShowContextMenu(const QPoint& pos) {
  QModelIndex index = indexAt(pos);
  if (!index.isValid()) return;

  QModelIndexList selection_list = selectionModel()->selectedIndexes();
  if (selection_list.isEmpty()) return;

  std::set<int> selection_set;
  for (QModelIndex& selected_index : selection_list) {
    selection_set.insert(selected_index.row());
  }
  std::vector<int> selected_indices(selection_set.begin(), selection_set.end());

  int clicked_index = index.row();
  const std::vector<orbit_data_views::DataView::ActionGroup> menu_with_grouping =
      model_->GetDataView()->GetContextMenuWithGrouping(clicked_index, selected_indices);
  if (menu_with_grouping.empty()) return;

  QMenu context_menu(tr("ContextMenu"), this);
  std::vector<std::unique_ptr<QAction>> actions;
  for (size_t i = 0; i < menu_with_grouping.size(); ++i) {
    if (i > 0) context_menu.addSeparator();

    for (const orbit_data_views::DataView::Action& action : menu_with_grouping[i]) {
      actions.push_back(std::make_unique<QAction>(QString::fromStdString(action.name)));
      actions.back()->setEnabled(action.enabled);
      size_t action_index = actions.size();
      connect(actions.back().get(), &QAction::triggered,
              [this, action, action_index] { OnMenuClicked(action.name, action_index); });
      context_menu.addAction(actions.back().get());
    }
  }

  context_menu.exec(mapToGlobal(pos));
}

void OrbitTreeView::OnMenuClicked(std::string_view action, int menu_index) {
  if (model_ == nullptr) {
    return;
  }

  QModelIndexList selection_list = selectionModel()->selectedIndexes();
  std::set<int> selection_set;
  for (QModelIndex& index : selection_list) {
    selection_set.insert(index.row());
  }

  std::vector<int> indices(selection_set.begin(), selection_set.end());
  if (!indices.empty()) {
    model_->GetDataView()->OnContextMenu(action, menu_index, indices);
  }
}

void OrbitTreeView::keyPressEvent(QKeyEvent* event) {
  if (model_ == nullptr) {
    return;
  }

  if (event->matches(QKeySequence::Copy)) {
    QModelIndexList list = selectionModel()->selectedIndexes();
    std::set<int> selection;
    for (QModelIndex& index : list) {
      selection.insert(index.row());
    }

    std::vector<int> items(selection.begin(), selection.end());
    model_->GetDataView()->OnCopySelectionRequested(items);
  } else {
    QTreeView::keyPressEvent(event);
  }
}

void OrbitTreeView::mousePressEvent(QMouseEvent* event) {
  QModelIndex index = indexAt(event->pos());
  // Deselect previous selections if clicking the empty area of the OrbitTreeView.
  if (!index.isValid()) {
    setCurrentIndex(QModelIndex());
  } else {
    QTreeView::mousePressEvent(event);
  }
}

void OrbitTreeView::selectionChanged(const QItemSelection& selected,
                                     const QItemSelection& deselected) {
  QTreeView::selectionChanged(selected, deselected);

  // Don't trigger callbacks if selection was initiated internally.
  if (is_internal_refresh_) return;

  // Row selection callback.
  std::vector<int> selected_rows;
  if (is_multi_selection_) {
    QModelIndexList selected_indexes = selectionModel()->selectedIndexes();
    for (QModelIndex& index : selected_indexes) {
      selected_rows.push_back(index.row());
    }
  } else {
    QModelIndex index = selectionModel()->currentIndex();
    if (index.isValid()) {
      selected_rows.push_back(index.row());
    }
  }

  OnRowsSelected(selected_rows);
}

void OrbitTreeView::OnRowsSelected(std::vector<int>& rows) {
  if (model_ != nullptr) {
    std::set<int> row_set(rows.begin(), rows.end());
    rows.assign(row_set.begin(), row_set.end());
    model_->OnRowsSelected(rows);
  }
  for (OrbitTreeView* tree_view : links_) {
    tree_view->Refresh();
  }
}

void OrbitTreeView::OnRangeChanged(int /*min*/, int max) {
  if (model_ == nullptr) {
    return;
  }

  orbit_data_views::DataView* data_view = model_->GetDataView();
  if (data_view->ScrollToBottom()) {
    verticalScrollBar()->setValue(max);
  }
}

void OrbitTreeView::OnDoubleClicked(QModelIndex index) {
  if (model_ != nullptr) {
    model_->GetDataView()->OnDoubleClicked(index.row());
  }
}

std::string OrbitTreeView::GetLabel() {
  if (model_ != nullptr && model_->GetDataView() != nullptr) {
    return model_->GetDataView()->GetLabel();
  }
  return "";
}

bool OrbitTreeView::HasRefreshButton() const {
  if (model_ != nullptr && model_->GetDataView() != nullptr) {
    return model_->GetDataView()->HasRefreshButton();
  }
  return false;
}

void OrbitTreeView::OnRefreshButtonClicked() {
  if (model_ != nullptr && model_->GetDataView() != nullptr) {
    model_->GetDataView()->OnRefreshButtonClicked();
  }
}

void OrbitTreeView::columnResized(int column, int /*oldSize*/, int new_size) {
  // We'd need to run this only when a column is being resized directly, not when the entire table
  // is being resized and in turn triggers column resize events, otherwise the ratios can be set
  // to 0 when shrinking the table width to 0 which we can't recover from. For this reason,
  // maintain_user_column_ratios_ defaults to "false", the code can be enabled once we find the
  // proper event filtering magic that will let us differentiate between direct and indirect column
  // resizing.
  if (maintain_user_column_ratios_ && (column_ratios_.size() != 0u)) {
    ORBIT_CHECK(column < static_cast<int>(column_ratios_.size()));
    column_ratios_[column] = static_cast<float>(new_size) / size().width();
  }
}
