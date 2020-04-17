//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <QWidget>
#include <memory>

namespace Ui {
class OrbitSamplingReport;
}

class OrbitSamplingReport : public QWidget {
  Q_OBJECT

 public:
  explicit OrbitSamplingReport(QWidget* parent = nullptr);
  ~OrbitSamplingReport() override;

  void Initialize(std::shared_ptr<class SamplingReport> a_Report);

 protected:
  void Refresh();

 private slots:
  void on_NextCallstackButton_clicked();
  void on_PreviousCallstackButton_clicked();

 private:
  Ui::OrbitSamplingReport* ui;
  std::shared_ptr<SamplingReport> m_SamplingReport;
};
