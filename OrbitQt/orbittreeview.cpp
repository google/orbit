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
    : QTreeView(parent),
      m_Model(nullptr),
      m_Timer(nullptr),
      m_AutoResize(true) {
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

  connect(this->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), this,
          SLOT(OnRangeChanged(int, int)));
}

//-----------------------------------------------------------------------------
OrbitTreeView::~OrbitTreeView() {
  delete m_Model;
  delete m_Timer;
}

//-----------------------------------------------------------------------------
void OrbitTreeView::Initialize(DataViewType a_Type) {
  m_Model = new OrbitTableModel(a_Type);
  setModel(m_Model);
  header()->resizeSections(QHeaderView::ResizeToContents);

  if (!m_Model->IsSortingAllowed()) {
    // Don't do setSortingEnabled(m_Model->IsSortingAllowed()); as with true it
    // forces a sort by the first column.
    setSortingEnabled(false);
  } else {
    std::pair<int, Qt::SortOrder> column_and_order =
        m_Model->GetDefaultSortingColumnAndOrder();
    sortByColumn(column_and_order.first, column_and_order.second);
  }

  if (m_Model->GetUpdatePeriodMs() > 0) {
    m_Timer = new QTimer(this);
    connect(m_Timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
    m_Timer->start(m_Model->GetUpdatePeriodMs());
  }

  if (a_Type == DataViewType::FUNCTIONS ||
      a_Type == DataViewType::LIVE_FUNCTIONS ||
      a_Type == DataViewType::CALLSTACK || a_Type == DataViewType::MODULES ||
      a_Type == DataViewType::GLOBALS) {
    setSelectionMode(ExtendedSelection);
  }

  if (a_Type == DataViewType::LOG) {
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    this->setFont(fixedFont);
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::SetDataModel(std::shared_ptr<DataView> a_Model) {
  m_Model = new OrbitTableModel();
  m_Model->SetDataView(std::move(a_Model));
  setModel(m_Model);
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnSort(int a_Section, Qt::SortOrder a_Order) {
  m_Model->sort(a_Section, a_Order);
  Refresh();
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnFilter(const QString& a_Filter) {
  m_Model->OnFilter(a_Filter);
  Refresh();
}

//-----------------------------------------------------------------------------
void OrbitTreeView::Select(int a_Row) {
  QModelIndex idx = m_Model->CreateIndex(a_Row, 0);
  m_Model->OnClicked(idx);
  Refresh();
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnTimer() {
  if (this->isVisible() && !m_Model->GetDataView()->SkipTimer()) {
    m_Model->OnTimer();
    Refresh();
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnClicked(const QModelIndex& index) {
  m_Model->OnClicked(index);

  for (OrbitTreeView* treeView : m_Links) {
    treeView->Refresh();
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::Refresh() {
  QModelIndexList list = selectionModel()->selectedIndexes();

  if (this->m_Model->GetDataView()->GetType() == DataViewType::LIVE_FUNCTIONS) {
    m_Model->layoutAboutToBeChanged();
    m_Model->layoutChanged();
    return;
  }

  reset();

  // Re-select previous selection
  int selected = m_Model->GetSelectedIndex();
  if (selected >= 0) {
    QItemSelectionModel* selection = selectionModel();
    QModelIndex idx = m_Model->CreateIndex(selected, 0);
    selection->select(
        idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::resizeEvent(QResizeEvent* event) {
  if (m_AutoResize && m_Model && m_Model->GetDataView()) {
    QSize headerSize = size();
    for (size_t i = 0; i < m_Model->GetDataView()->GetColumns().size(); ++i) {
      float ratio = m_Model->GetDataView()->GetColumns()[i].ratio;
      if (ratio > 0.f) {
        header()->resizeSection(i,
                                static_cast<int>(headerSize.width() * ratio));
      }
    }
  }

  QTreeView::resizeEvent(event);
}

//-----------------------------------------------------------------------------
void OrbitTreeView::Link(OrbitTreeView* a_Link) {
  m_Links.push_back(a_Link);

  std::shared_ptr<DataView> dataView = a_Link->GetModel()->GetDataView();
  m_Model->GetDataView()->LinkDataView(dataView.get());
}

//-----------------------------------------------------------------------------
void OrbitTreeView::SetGlWidget(OrbitGLWidget* a_GlWidget) {
  this->m_Model->GetDataView()->SetGlPanel(a_GlWidget->GetPanel());
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
  QModelIndex index = this->indexAt(pos);
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
        m_Model->GetDataView()->GetContextMenu(clicked_index, selected_indices);
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
    m_Model->GetDataView()->OnContextMenu(a_Action, a_MenuIndex, indices);
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
    this->m_Model->GetDataView()->CopySelection(items);
  } else {
    QTreeView::keyPressEvent(event);
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnRangeChanged(int /*a_Min*/, int a_Max) {
  std::shared_ptr<DataView> DataView = m_Model->GetDataView();
  if (DataView->ScrollToBottom()) {
    verticalScrollBar()->setValue(a_Max);
  }
}

//-----------------------------------------------------------------------------
std::string OrbitTreeView::GetLabel() {
  if (m_Model != nullptr && m_Model->GetDataView() != nullptr) {
    return m_Model->GetDataView()->GetLabel();
  }
  return "";
}

//-----------------------------------------------------------------------------
void OrbitTreeView::columnResized(int /*column*/, int /*oldSize*/,
                                  int /*newSize*/) {
  if (QApplication::mouseButtons() == Qt::LeftButton) {
    m_AutoResize = false;
  }
}
