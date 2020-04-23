//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "orbittablemodel.h"

#include <QColor>
#include <memory>

//-----------------------------------------------------------------------------
OrbitTableModel::OrbitTableModel(DataViewType a_Type, QObject* parent)
    : QAbstractTableModel(parent),
      m_DataView(nullptr),
      m_AlternateRowColor(true) {
  m_DataView = DataView::Create(a_Type);

  if (a_Type == DataViewType::LOG) {
    m_AlternateRowColor = false;
  }
}

//-----------------------------------------------------------------------------
OrbitTableModel::OrbitTableModel(QObject* parent)
    : QAbstractTableModel(parent),
      m_DataView(nullptr),
      m_AlternateRowColor(true) {}

//-----------------------------------------------------------------------------
OrbitTableModel::~OrbitTableModel() {}

//-----------------------------------------------------------------------------
int OrbitTableModel::columnCount(const QModelIndex& /*parent*/) const {
  return (int)m_DataView->GetColumns().size();
}

//-----------------------------------------------------------------------------
int OrbitTableModel::rowCount(const QModelIndex& /*parent*/) const {
  return (int)m_DataView->GetNumElements();
}

//-----------------------------------------------------------------------------
QVariant OrbitTableModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const {
  switch (role) {
    case Qt::DisplayRole:
      if (orientation == Qt::Horizontal &&
          section < (int)m_DataView->GetColumns().size()) {
        std::string header = m_DataView->GetColumns()[section].header;
        return QString::fromStdString(header);
      } else if (orientation == Qt::Vertical) {
        return section;
      } else {
        return QVariant();
      }

    case Qt::InitialSortOrderRole:
      return m_DataView->GetColumns()[section].initial_order ==
                     DataView::SortingOrder::Ascending
                 ? Qt::AscendingOrder
                 : Qt::DescendingOrder;

    default:
      break;
  }

  return QVariant();
}

//-----------------------------------------------------------------------------
QVariant OrbitTableModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::DisplayRole) {
    std::string value = m_DataView->GetValue(index.row(), index.column());
    return QVariant(QString::fromStdString(value));
  } else if (role == Qt::BackgroundRole) {
    if (m_AlternateRowColor) {
      if (index.row() % 2 == 1)
        return QColor(60, 60, 60);
      else
        return QColor(45, 45, 48);
    } else {
      return QColor(37, 37, 38);
    }
  } else if (role == Qt::ForegroundRole) {
    if (m_DataView->WantsDisplayColor()) {
      unsigned char r, g, b;
      if (m_DataView->GetDisplayColor(index.row(), index.column(), r, g, b)) {
        return QColor(r, g, b);
      }
    }
  } else if (role == Qt::ToolTipRole) {
    std::string tooltip = m_DataView->GetToolTip(index.row(), index.column());
    return QString::fromStdString(tooltip);
  }

  return QVariant();
}

//-----------------------------------------------------------------------------
void OrbitTableModel::sort(int column, Qt::SortOrder order) {
  // On Linux, the arrows for ascending/descending are reversed, e.g. ascending
  // has an arrow pointing down, which is unexpected. This is because of some
  // Linux UI guideline, it is not the case on Windows.
  m_DataView->OnSort(column, order == Qt::AscendingOrder
                                 ? DataView::SortingOrder::Ascending
                                 : DataView::SortingOrder::Descending);
}

//-----------------------------------------------------------------------------
std::pair<int, Qt::SortOrder>
OrbitTableModel::GetDefaultSortingColumnAndOrder() {
  if (!IsSortingAllowed()) {
    return std::make_pair(-1, Qt::AscendingOrder);
  }

  int column = m_DataView->GetDefaultSortingColumn();
  Qt::SortOrder order = m_DataView->GetColumns()[column].initial_order ==
                                DataView::SortingOrder::Ascending
                            ? Qt::AscendingOrder
                            : Qt::DescendingOrder;
  return std::make_pair(column, order);
}

//-----------------------------------------------------------------------------
void OrbitTableModel::OnTimer() { m_DataView->OnTimer(); }

//-----------------------------------------------------------------------------
void OrbitTableModel::OnFilter(const QString& a_Filter) {
  m_DataView->OnFilter(a_Filter.toStdString());
}

//-----------------------------------------------------------------------------
void OrbitTableModel::OnClicked(const QModelIndex& index) {
  if ((int)m_DataView->GetNumElements() > index.row()) {
    m_DataView->OnSelect(index.row());
  }
}
