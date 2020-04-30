//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <QAbstractTableModel>
#include <memory>
#include <utility>

#include "DataView.h"

//-----------------------------------------------------------------------------
class OrbitTableModel : public QAbstractTableModel {
  Q_OBJECT
 public:
  explicit OrbitTableModel(DataView* data_view, bool alternate_row_color = true,
                           QObject* parent = nullptr);
  explicit OrbitTableModel(QObject* parent = nullptr);
  ~OrbitTableModel() override;

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
  DataView* GetDataView() { return m_DataView; }
  void SetDataView(DataView* model) { m_DataView = model; }
  bool IsSortingAllowed() { return GetDataView()->IsSortingAllowed(); }
  std::pair<int, Qt::SortOrder> GetDefaultSortingColumnAndOrder();

  void OnTimer();
  void OnFilter(const QString& a_Filter);
  void OnClicked(const QModelIndex& index);

 protected:
  DataView* m_DataView;
  bool m_AlternateRowColor;
};
