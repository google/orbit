// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_SAMPLING_REPORT_H_
#define ORBIT_QT_ORBIT_SAMPLING_REPORT_H_

#include <QWidget>
#include <memory>

#include "CallStackDataView.h"
#include "orbitdataviewpanel.h"

namespace Ui {
class OrbitSamplingReport;
}

class OrbitSamplingReport : public QWidget {
  Q_OBJECT

 public:
  explicit OrbitSamplingReport(QWidget* parent = nullptr);
  ~OrbitSamplingReport() override;

  void Initialize(DataView* callstack_data_view,
                  const std::shared_ptr<class SamplingReport>& report);

  void RefreshCallstackView();
  void RefreshTabs();

 private slots:
  void on_NextCallstackButton_clicked();
  void on_PreviousCallstackButton_clicked();

 private:
  Ui::OrbitSamplingReport* ui;
  std::shared_ptr<SamplingReport> m_SamplingReport;
  std::vector<OrbitDataViewPanel*> m_OrbitDataViews;
};

#endif  // ORBIT_QT_ORBIT_SAMPLING_REPORT_H_
