//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

// This needs to be first because if it is not GL/glew.h
// complains about being included after gl.h
// clang-format off
#include "OpenGl.h"
// clang-format on

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

#include "App.h"
#include "DataView.h"
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
void OrbitTreeView::Initialize(DataView* data_view,
                               SelectionType selection_type,
                               FontType font_type) {
  model_ = std::make_unique<OrbitTableModel>(data_view);
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

  if (selection_type == SelectionType::kExtended) {
    setSelectionMode(ExtendedSelection);
  }

  if (font_type == FontType::kFixed) {
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    setFont(fixedFont);
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::SetDataModel(DataView* data_view) {
  model_ = std::make_unique<OrbitTableModel>();
  model_->SetDataView(data_view);
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
  model_->OnSelected(GetSelectedIndexes(), row);
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
  model_->OnSelected(GetSelectedIndexes(), index.row());

  for (OrbitTreeView* tree_view : links_) {
    tree_view->Refresh();
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::Refresh() {

  if (model_->GetDataView()->GetType() == DataViewType::LIVE_FUNCTIONS) {
    model_->layoutAboutToBeChanged();
    model_->layoutChanged();
    return;
  }

  reset();

  // Re-select previous selection
  const std::vector<int> selected_indexes = model_->GetSelectedIndexes();

  QItemSelectionModel* selection_model = selectionModel();
  QItemSelection selection;

  for (int selected_index : selected_indexes) {
    QModelIndex idx = model_->CreateIndex(selected_index, 0);
    selection.select(idx, idx);
  }

  selection_model->select(selection, QItemSelectionModel::ClearAndSelect |
                                         QItemSelectionModel::Rows);
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

  DataView* data_view = link->GetModel()->GetDataView();
  model_->GetDataView()->LinkDataView(data_view);
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

std::vector<int> OrbitTreeView::GetSelectedIndexes() {
  QModelIndexList selection_list = selectionModel()->selectedIndexes();

  std::set<int> selection_set;
  for (QModelIndex& selected_index : selection_list) {
    selection_set.insert(selected_index.row());
  }

  return std::vector<int>(selection_set.begin(), selection_set.end());
}

void OrbitTreeView::ShowContextMenu(const QPoint& pos) {
  QModelIndex index = indexAt(pos);
  if (index.isValid()) {
    int clicked_index = index.row();

    std::vector<int> selected_indices = GetSelectedIndexes();

    std::vector<std::string> menu =
        model_->GetDataView()->GetContextMenu(clicked_index, selected_indices);
    if (!menu.empty()) {
      QMenu contextMenu(tr("ContextMenu"), this);
      GContextMenu = &contextMenu;
      std::vector<std::unique_ptr<QAction>> actions;

      for (size_t i = 0; i < menu.size(); ++i) {
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

void OrbitTreeView::OnMenuClicked(const std::string& a_Action,
                                  int a_MenuIndex) {
  std::vector<int> indices = GetSelectedIndexes();
  if (!indices.empty()) {
    model_->GetDataView()->OnContextMenu(a_Action, a_MenuIndex, indices);
  }
}

void OrbitTreeView::keyPressEvent(QKeyEvent* event) {
  if (event->matches(QKeySequence::Copy)) {
    std::vector<int> items = GetSelectedIndexes();
    model_->GetDataView()->CopySelection(items);
  } else {
    QTreeView::keyPressEvent(event);
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnRangeChanged(int /*a_Min*/, int a_Max) {
  DataView* data_view = model_->GetDataView();
  if (data_view->ScrollToBottom()) {
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
