#include "orbittreemodel.h"

#include <QColor>
#include <QStringList>

#include "orbittreeitem.h"

OrbitTreeModel::OrbitTreeModel(const QString& data, QObject* parent)
    : QAbstractItemModel(parent) {
  QList<QVariant> rootData;
  rootData << "Header";
  rootItem = new OrbitTreeItem(rootData);
  setupModelData(data.split(QString("\n")), rootItem);
}

OrbitTreeModel::~OrbitTreeModel() { delete rootItem; }

int OrbitTreeModel::columnCount(const QModelIndex& parent) const {
  if (parent.isValid())
    return static_cast<OrbitTreeItem*>(parent.internalPointer())->columnCount();
  else
    return rootItem->columnCount();
}

QVariant OrbitTreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();

  OrbitTreeItem* item = static_cast<OrbitTreeItem*>(index.internalPointer());

  if (role == Qt::ForegroundRole) {
    return item->MatchesFilter() ? QColor(42, 130, 218) : QColor(255, 255, 255);
  } else if (role == Qt::DisplayRole) {
    return item->data(index.column());
  }

  return QVariant();
}

Qt::ItemFlags OrbitTreeModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) return 0;

  return QAbstractItemModel::flags(index);
}

QVariant OrbitTreeModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return rootItem->data(section);

  return QVariant();
}

QModelIndex OrbitTreeModel::index(int row, int column,
                                  const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) return QModelIndex();

  OrbitTreeItem* parentItem;

  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<OrbitTreeItem*>(parent.internalPointer());

  OrbitTreeItem* childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex OrbitTreeModel::parent(const QModelIndex& index) const {
  if (!index.isValid()) return QModelIndex();

  OrbitTreeItem* childItem =
      static_cast<OrbitTreeItem*>(index.internalPointer());
  OrbitTreeItem* parentItem = childItem->parentItem();

  if (parentItem == rootItem) return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

int OrbitTreeModel::rowCount(const QModelIndex& parent) const {
  OrbitTreeItem* parentItem;
  if (parent.column() > 0) return 0;

  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<OrbitTreeItem*>(parent.internalPointer());

  return parentItem->childCount();
}

void OrbitTreeModel::setupModelData(const QStringList& lines,
                                    OrbitTreeItem* parent) {
  QList<OrbitTreeItem*> parents;
  QList<int> indentations;
  parents << parent;
  indentations << 0;

  int number = 0;

  while (number < lines.count()) {
    int position = 0;
    while (position < lines[number].length()) {
      if (lines[number].at(position) != ' ') break;
      position++;
    }

    QString lineData = lines[number].mid(position).trimmed();

    if (!lineData.isEmpty()) {
      // Read the column data from the rest of the line.
      QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
      QList<QVariant> columnData;
      for (int column = 0; column < columnStrings.count(); ++column)
        columnData << columnStrings[column];

      if (position > indentations.last()) {
        // The last child of the current parent is now the new parent
        // unless the current parent has no children.

        if (parents.last()->childCount() > 0) {
          parents << parents.last()->child(parents.last()->childCount() - 1);
          indentations << position;
        }
      } else {
        while (position < indentations.last() && parents.count() > 0) {
          parents.pop_back();
          indentations.pop_back();
        }
      }

      // Append a new item to the current parent's list of children.
      parents.last()->appendChild(
          new OrbitTreeItem(columnData, parents.last()));
    }

    ++number;
  }
}

void OrbitTreeModel::Filter(const std::string& a_Filter) {
  if (rootItem) {
    rootItem->Filter(a_Filter);
  }
}
