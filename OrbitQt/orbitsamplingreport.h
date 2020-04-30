//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <QWidget>
#include <memory>

#include "CallStackDataView.h"

namespace Ui {
class OrbitSamplingReport;
}

class OrbitSamplingReport : public QWidget {
  Q_OBJECT

 public:
  explicit OrbitSamplingReport(QWidget* parent = nullptr);
  ~OrbitSamplingReport() override;

  void Initialize(DataView* callstack_data_view,
                  std::shared_ptr<class SamplingReport> report);

 protected:
  void Refresh();

 private slots:
  void on_NextCallstackButton_clicked();
  void on_PreviousCallstackButton_clicked();

 private:
  Ui::OrbitSamplingReport* ui;
  std::shared_ptr<SamplingReport> m_SamplingReport;
};
