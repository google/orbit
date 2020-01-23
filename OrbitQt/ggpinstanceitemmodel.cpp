#include "ggpinstanceitemmodel.h"
#include <qabstractitemmodel.h>
#include <qnamespace.h>
#include "ggpinstance.h"

namespace {
enum class Columns {
  DisplayName,
  ID,
  IPAddress,
  LastUpdated,
  Owner,
  Pool,
  NumberOfColumns
};
}  // namespace

GgpInstanceItemModel::GgpInstanceItemModel(std::vector<GgpInstance> instances_,
                                           QObject* parent)
    : QAbstractItemModel(parent), instances(std::move(instances_)) {
  std::sort(this->instances.begin(), this->instances.end(), cmpById);
}

int GgpInstanceItemModel::columnCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : static_cast<int>(Columns::NumberOfColumns);
}

QVariant GgpInstanceItemModel::data(const QModelIndex& index, int role) const {
  assert(index.isValid());
  assert(index.model() == this);
  assert(index.row() < instances.size());

  const auto& currentInstance = instances[index.row()];

  if (role == Qt::UserRole) return QVariant::fromValue(currentInstance);

  if (role != Qt::DisplayRole) return {};

  switch (static_cast<Columns>(index.column())) {
    case Columns::DisplayName:
      return currentInstance.displayName;
    case Columns::ID:
      return currentInstance.id;
    case Columns::IPAddress:
      return currentInstance.ipAddress;
    case Columns::LastUpdated:
      return currentInstance.lastUpdated.toString(Qt::TextDate);
    case Columns::Owner:
      return currentInstance.owner;
    case Columns::Pool:
      return currentInstance.pool;
    case Columns::NumberOfColumns:
      assert(false);
      return {};
  }

  assert(false);  // That means, someone (me?) forgot a column.
  return {};
}

QModelIndex GgpInstanceItemModel::index(int row, int col,
                                        const QModelIndex& parent) const {
  if (parent.isValid()) return {};
  if (row < 0 || row >= instances.size()) return {};
  if (col < 0 || col >= static_cast<int>(Columns::NumberOfColumns)) return {};

  return createIndex(row, col, nullptr);
}

QVariant GgpInstanceItemModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
  if (role != Qt::DisplayRole) return {};
  if (orientation != Qt::Horizontal) return {};
  if (section < 0 || section >= static_cast<int>(Columns::NumberOfColumns))
    return {};

  switch (static_cast<Columns>(section)) {
    case Columns::DisplayName:
      return QLatin1String("Display Name");
    case Columns::ID:
      return QLatin1String("ID");
    case Columns::IPAddress:
      return QLatin1String("IP Address");
    case Columns::LastUpdated:
      return QLatin1String("Last Updated");
    case Columns::Owner:
      return QLatin1String("Owner");
    case Columns::Pool:
      return QLatin1String("Pool");
    case Columns::NumberOfColumns:
      assert(false);
      return {};
  }

  assert(false);
  return {};
}

QModelIndex GgpInstanceItemModel::parent(const QModelIndex&) const {
  return {};
}

int GgpInstanceItemModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : instances.size();
}

void GgpInstanceItemModel::setInstances(std::vector<GgpInstance> instances_) {
  std::sort(instances_.begin(), instances_.end(), cmpById);

  auto begin1 = this->instances.begin();
  auto begin2 = instances_.begin();

  while (begin1 != this->instances.end() && begin2 != instances_.end()) {
    const auto currentRow = std::distance(this->instances.begin(), begin1);

    if (begin1->id == begin2->id) {
      if (*begin1 != *begin2) {
        *begin1 = *begin2;
        emit dataChanged(
            index(currentRow, 0, {}),
            index(currentRow, static_cast<int>(Columns::NumberOfColumns) - 1,
                  {}));
      }
      ++begin1;
      ++begin2;
    } else if (begin1->id < begin2->id) {
      beginRemoveRows({}, currentRow, currentRow);
      begin1 = this->instances.erase(begin1);
      endRemoveRows();
    } else {
      beginInsertRows({}, currentRow, currentRow);
      begin1 = this->instances.insert(begin1, *begin2);
      ++begin1;
      ++begin2;
      endInsertRows();
    }
  }

  if (begin1 == this->instances.end() && begin2 != instances_.end()) {
    beginInsertRows({}, this->instances.size(), instances_.size() - 1);
    std::copy(begin2, instances_.end(), std::back_inserter(this->instances));
    assert(this->instances.size() == instances_.size());
    endInsertRows();
  } else if (begin1 != this->instances.end() && begin2 == instances_.end()) {
    beginRemoveRows({}, instances_.size(), this->instances.size() - 1);
    this->instances.erase(begin1, this->instances.end());
    assert(this->instances.size() == instances_.size());
    endRemoveRows();
  }
}