#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class OrbitTreeItem;

class OrbitTreeModel : public QAbstractItemModel {
  Q_OBJECT

 public:
  explicit OrbitTreeModel(const QString& data, QObject* parent = nullptr);
  ~OrbitTreeModel() override;

  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  void Filter(const std::string& a_Filter);

 private:
  void setupModelData(const QStringList& lines, OrbitTreeItem* parent);

  OrbitTreeItem* rootItem;
};
