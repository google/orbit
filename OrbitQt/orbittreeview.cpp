//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "orbittreeview.h"

#include <QApplication>
#include <QFontDatabase>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QSignalMapper>
#include <set>
#include <utility>

#include "../OrbitGl/App.h"
#include "../OrbitGl/DataView.h"
#include "orbitglwidget.h"

//-----------------------------------------------------------------------------
OrbitTreeView::OrbitTreeView(QWidget* parent)
    : QTreeView(parent), auto_resize_(true) {
  header()->setSortIndicatorShown(true);
  header()->setSectionsClickable(true);

  setRootIsDecorated(false);
  setItemsExpandable(false);
  setContextMenuPolicy(Qt::CustomContextMenu);
  setSelectionBehavior(QTreeView::SelectRows);
  setUniformRowHeights(true);
  setTextElideMode(Qt::ElideMiddle);

  connect(header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this,
          SLOT(OnSort(int, Qt::SortOrder)));

  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this,
          SLOT(ShowContextMenu(const QPoint&)));

  connect(header(), SIGNAL(sectionResized(int, int, int)), this,
          SLOT(columnResized(int, int, int)));

  connect(this, SIGNAL(clicked(const QModelIndex)), this,
          SLOT(OnClicked(const QModelIndex)));

  connect(verticalScrollBar(), SIGNAL(rangeChanged(int, int)), this,
          SLOT(OnRangeChanged(int, int)));
}

//-----------------------------------------------------------------------------
void OrbitTreeView::Initialize(DataViewType type) {
  model_ = std::make_unique<OrbitTableModel>(type);
  setModel(model_.get());
  header()->resizeSections(QHeaderView::ResizeToContents);

  if (!model_->IsSortingAllowed()) {
    // Don't do setSortingEnabled(model_->IsSortingAllowed()); as with true it
    // forces a sort by the first column.
    setSortingEnabled(false);
  } else {
    std::pair<int, Qt::SortOrder> column_and_order =
        model_->GetDefaultSortingColumnAndOrder();
    sortByColumn(column_and_order.first, column_and_order.second);
  }

  if (model_->GetUpdatePeriodMs() > 0) {
    timer_ = std::make_unique<QTimer>();
    connect(timer_.get(), SIGNAL(timeout()), this, SLOT(OnTimer()));
    timer_->start(model_->GetUpdatePeriodMs());
  }

  if (type == DataViewType::FUNCTIONS || type == DataViewType::LIVE_FUNCTIONS ||
      type == DataViewType::CALLSTACK || type == DataViewType::MODULES ||
      type == DataViewType::GLOBALS) {
    setSelectionMode(ExtendedSelection);
  }

  if (type == DataViewType::LOG) {
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    setFont(fixedFont);
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::SetDataModel(std::shared_ptr<DataView> data_view) {
  model_ = std::make_unique<OrbitTableModel>();
  model_->SetDataView(std::move(data_view));
  setModel(model_.get());
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnSort(int section, Qt::SortOrder order) {
  model_->sort(section, order);
  Refresh();
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnFilter(const QString& filter) {
  model_->OnFilter(filter);
  Refresh();
}

//-----------------------------------------------------------------------------
void OrbitTreeView::Select(int row) {
  QModelIndex idx = model_->CreateIndex(row, 0);
  model_->OnClicked(idx);
  Refresh();
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnTimer() {
  if (isVisible() && !model_->GetDataView()->SkipTimer()) {
    model_->OnTimer();
    Refresh();
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnClicked(const QModelIndex& index) {
  model_->OnClicked(index);

  for (OrbitTreeView* tree_view : links_) {
    tree_view->Refresh();
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::Refresh() {
  QModelIndexList list = selectionModel()->selectedIndexes();

  if (model_->GetDataView()->GetType() == DataViewType::LIVE_FUNCTIONS) {
    model_->layoutAboutToBeChanged();
    model_->layoutChanged();
    return;
  }

  reset();

  // Re-select previous selection
  int selected = model_->GetSelectedIndex();
  if (selected >= 0) {
    QItemSelectionModel* selection = selectionModel();
    QModelIndex idx = model_->CreateIndex(selected, 0);
    selection->select(
        idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::resizeEvent(QResizeEvent* event) {
  if (auto_resize_ && model_ && model_->GetDataView()) {
    QSize headerSize = size();
    for (size_t i = 0; i < model_->GetDataView()->GetColumns().size(); ++i) {
      float ratio = model_->GetDataView()->GetColumns()[i].ratio;
      if (ratio > 0.f) {
        header()->resizeSection(i,
                                static_cast<int>(headerSize.width() * ratio));
      }
    }
  }

  QTreeView::resizeEvent(event);
}

//-----------------------------------------------------------------------------
void OrbitTreeView::Link(OrbitTreeView* link) {
  links_.push_back(link);

  std::shared_ptr<DataView> dataView = link->GetModel()->GetDataView();
  model_->GetDataView()->LinkDataView(dataView.get());
}

//-----------------------------------------------------------------------------
void OrbitTreeView::SetGlWidget(OrbitGLWidget* a_GlWidget) {
  model_->GetDataView()->SetGlPanel(a_GlWidget->GetPanel());
}

//-----------------------------------------------------------------------------
void OrbitTreeView::drawRow(QPainter* painter,
                            const QStyleOptionViewItem& options,
                            const QModelIndex& index) const {
  QTreeView::drawRow(painter, options, index);
}

//-----------------------------------------------------------------------------
QMenu* GContextMenu;

//-----------------------------------------------------------------------------
void OrbitTreeView::ShowContextMenu(const QPoint& pos) {
  QModelIndex index = indexAt(pos);
  if (index.isValid()) {
    int clicked_index = index.row();

    QModelIndexList selection_list = selectionModel()->selectedIndexes();
    std::set<int> selection_set;
    for (QModelIndex& selected_index : selection_list) {
      selection_set.insert(selected_index.row());
    }
    std::vector<int> selected_indices(selection_set.begin(),
                                      selection_set.end());

    std::vector<std::string> menu =
        model_->GetDataView()->GetContextMenu(clicked_index, selected_indices);
    if (!menu.empty()) {
      QMenu contextMenu(tr("ContextMenu"), this);
      GContextMenu = &contextMenu;
      std::vector<std::unique_ptr<QAction>> actions;

      for (int i = 0; i < (int)menu.size(); ++i) {
        actions.push_back(
            std::make_unique<QAction>(QString::fromStdString(menu[i])));
        connect(actions[i].get(), &QAction::triggered,
                [this, &menu, i] { OnMenuClicked(menu[i], i); });
        contextMenu.addAction(actions[i].get());
      }

      contextMenu.exec(mapToGlobal(pos));
      GContextMenu = nullptr;
    }
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnMenuClicked(const std::string& a_Action,
                                  int a_MenuIndex) {
  QModelIndexList selection_list = selectionModel()->selectedIndexes();
  std::set<int> selection_set;
  for (QModelIndex& index : selection_list) {
    selection_set.insert(index.row());
  }

  std::vector<int> indices(selection_set.begin(), selection_set.end());
  if (!indices.empty()) {
    model_->GetDataView()->OnContextMenu(a_Action, a_MenuIndex, indices);
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::keyPressEvent(QKeyEvent* event) {
  if (event->matches(QKeySequence::Copy)) {
    QModelIndexList list = selectionModel()->selectedIndexes();
    std::set<int> selection;
    for (QModelIndex& index : list) {
      selection.insert(index.row());
    }

    std::vector<int> items(selection.begin(), selection.end());
    model_->GetDataView()->CopySelection(items);
  } else {
    QTreeView::keyPressEvent(event);
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnRangeChanged(int /*a_Min*/, int a_Max) {
  std::shared_ptr<DataView> DataView = model_->GetDataView();
  if (DataView->ScrollToBottom()) {
    verticalScrollBar()->setValue(a_Max);
  }
}

//-----------------------------------------------------------------------------
std::string OrbitTreeView::GetLabel() {
  if (model_ != nullptr && model_->GetDataView() != nullptr) {
    return model_->GetDataView()->GetLabel();
  }
  return "";
}

//-----------------------------------------------------------------------------
void OrbitTreeView::columnResized(int /*column*/, int /*oldSize*/,
                                  int /*newSize*/) {
  if (QApplication::mouseButtons() == Qt::LeftButton) {
    auto_resize_ = false;
  }
}
