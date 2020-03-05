//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "orbittreeview.h"

#include <QFontDatabase>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>
#include <QSignalMapper>
#include <set>

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

  if (m_Model->GetUpdatePeriodMs() > 0) {
    m_Timer = new QTimer(this);
    connect(m_Timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
    m_Timer->start(m_Model->GetUpdatePeriodMs());
  }

  if (a_Type == DataViewType::FUNCTIONS ||
      a_Type == DataViewType::LIVEFUNCTIONS ||
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
  m_Model->SetDataView(a_Model);
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

  if (this->m_Model->GetDataView()->GetType() == DataViewType::LIVEFUNCTIONS) {
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
    const std::vector<float>& columnRatios =
        m_Model->GetDataView()->GetColumnHeadersRatios();
    for (size_t i = 0; i < columnRatios.size(); ++i) {
      float ratio = columnRatios[i];
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
    std::vector<std::wstring> menu =
        m_Model->GetDataView()->GetContextMenu(index.row());

    if (menu.size() > 0) {
      QMenu contextMenu(tr("ContextMenu"), this);
      GContextMenu = &contextMenu;
      QSignalMapper signalMapper(this);
      std::vector<QAction*> actions;

      for (int i = 0; i < (int)menu.size(); ++i) {
        actions.push_back(new QAction(QString::fromStdWString(menu[i])));
        connect(actions[i], SIGNAL(triggered()), &signalMapper, SLOT(map()));
        signalMapper.setMapping(actions[i], i);
        contextMenu.addAction(actions[i]);
      }

      connect(&signalMapper, SIGNAL(mapped(int)), this,
              SLOT(OnMenuClicked(int)));
      contextMenu.exec(mapToGlobal(pos));
      GContextMenu = nullptr;

      for (QAction* action : actions) delete action;
    }
  }
}

//-----------------------------------------------------------------------------
void OrbitTreeView::OnMenuClicked(int a_Index) {
  QModelIndexList list = selectionModel()->selectedIndexes();
  std::set<int> selection;
  for (QModelIndex& index : list) {
    selection.insert(index.row());
  }

  std::vector<int> indices(selection.begin(), selection.end());
  if (indices.size()) {
    const std::vector<std::wstring>& menu =
        m_Model->GetDataView()->GetContextMenu(indices[0]);
    m_Model->GetDataView()->OnContextMenu(menu[a_Index], a_Index, indices);
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
std::wstring OrbitTreeView::GetLabel() {
  if (m_Model && m_Model->GetDataView()) {
    return m_Model->GetDataView()->GetLabel();
  }

  return L"";
}

//-----------------------------------------------------------------------------
void OrbitTreeView::columnResized(int /*column*/, int /*oldSize*/,
                                  int /*newSize*/) {
#ifdef _WIN32
  if ((GetKeyState(VK_LBUTTON) & 0x100) != 0) {
    m_AutoResize = false;
  }
#endif
}
