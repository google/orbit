#ifndef ORBIT_ORBITQT_GGPINSTANCEITEMMODEL_H
#define ORBIT_ORBITQT_GGPINSTANCEITEMMODEL_H

#include <vector>
#include <QObject>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractItemModel>
#include "ggpinstance.h"

class GgpInstanceItemModel : public QAbstractItemModel {
  std::vector<GgpInstance> instances;

 public:
  explicit GgpInstanceItemModel(std::vector<GgpInstance> instances = {},
                                QObject* parent = nullptr);

  void setInstances(std::vector<GgpInstance> instances);

  [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index,
                              int role = Qt::DisplayRole) const override;
  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const override;
  [[nodiscard]] QModelIndex index(
      int row, int col, const QModelIndex& parent = {}) const override;
  [[nodiscard]] QModelIndex parent(const QModelIndex& parent) const override;
  [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
};

#endif  // ORBIT_ORBITQT_GGPINSTANCEITEMMODEL_H