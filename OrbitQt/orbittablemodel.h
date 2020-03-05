//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <QAbstractTableModel>
#include <memory>

#include "../OrbitGl/DataView.h"

//-----------------------------------------------------------------------------
class OrbitTableModel : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit OrbitTableModel(DataViewType a_Type, QObject* parent = 0);
  explicit OrbitTableModel(QObject* parent = 0);
  virtual ~OrbitTableModel();

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

  int GetUpdatePeriodMs() { return m_DataView->GetUpdatePeriodMs(); }
  int GetSelectedIndex() { return m_DataView->GetSelectedIndex(); }
  QModelIndex CreateIndex(int a_Row, int a_Column) {
    return createIndex(a_Row, a_Column);
  }
  std::shared_ptr<DataView> GetDataView() { return m_DataView; }
  void SetDataView(std::shared_ptr<DataView> a_Model) { m_DataView = a_Model; }

  void OnTimer();
  void OnFilter(const QString& a_Filter);
  void OnClicked(const QModelIndex& index);

 protected:
  std::shared_ptr<DataView> m_DataView;
  bool m_AlternateRowColor;
};
