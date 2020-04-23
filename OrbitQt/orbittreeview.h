//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <QTimer>
#include <QTreeView>
#include <memory>

#include "orbitglwidget.h"
#include "orbittablemodel.h"

class OrbitTreeView : public QTreeView {
  Q_OBJECT
 public:
  explicit OrbitTreeView(QWidget* parent = nullptr);
  void Initialize(DataViewType a_Type);
  void SetDataModel(std::shared_ptr<DataView> a_Model);
  void OnFilter(const QString& a_Filter);
  void Select(int a_Row);
  void Refresh();
  void Link(OrbitTreeView* a_Link);
  void SetGlWidget(OrbitGLWidget* a_Link);
  void resizeEvent(QResizeEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  OrbitTableModel* GetModel() { return model_.get(); }
  std::string GetLabel();

 protected:
  void drawRow(QPainter* painter, const QStyleOptionViewItem& options,
               const QModelIndex& index) const override;

 signals:

 public slots:
  void columnResized(int column, int oldSize, int newSize);

 private slots:
  void OnSort(int a_Section, Qt::SortOrder a_Order);
  void OnTimer();
  void OnClicked(const QModelIndex& index);
  void ShowContextMenu(const QPoint& pos);
  void OnMenuClicked(const std::string& a_Action, int a_MenuIndex);
  void OnRangeChanged(int a_Min, int a_Max);

 private:
  std::unique_ptr<OrbitTableModel> model_;
  std::unique_ptr<QTimer> timer_;
  std::vector<OrbitTreeView*> links_;
  bool auto_resize_;
};
