//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <QWidget>

#include "orbittablemodel.h"
#include "types.h"

namespace Ui {
class OrbitDataViewPanel;
}

class OrbitDataViewPanel : public QWidget {
  Q_OBJECT

 public:
  explicit OrbitDataViewPanel(QWidget* parent = nullptr);
  ~OrbitDataViewPanel() override;

  void Initialize(DataView* data_view, SelectionType selection_type,
                  FontType font_type, bool is_main_instance = true);
  void Link(OrbitDataViewPanel* a_Panel);
  void Refresh();
  void SetDataModel(DataView* model);
  void SetFilter(const QString& a_Filter);
  void Select(int a_Row);
  class OrbitTreeView* GetTreeView();

 private slots:
  void on_FilterLineEdit_textEdited(const QString& a_Text);

 private:
  Ui::OrbitDataViewPanel* ui;
};
