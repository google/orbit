// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_SAMPLING_REPORT_H_
#define ORBIT_QT_ORBIT_SAMPLING_REPORT_H_

#include <QObject>
#include <QString>
#include <QWidget>
#include <memory>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "DataViews/DataView.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/SamplingReport.h"
#include "OrbitQt/orbitdataviewpanel.h"

class SamplingReport;

namespace Ui {
class OrbitSamplingReport;
}

class OrbitSamplingReport : public QWidget {
  Q_OBJECT

 public:
  explicit OrbitSamplingReport(QWidget* parent = nullptr);
  ~OrbitSamplingReport() override;

  void Initialize(OrbitApp* app, orbit_data_views::DataView* callstack_data_view,
                  const orbit_client_data::CallstackData* callstack_data,
                  const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data);

  void UpdateReport(
      const orbit_client_data::CallstackData* callstack_data,
      const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data);

  void Deinitialize();

  void RefreshCallstackView();
  void RefreshTabs();

  [[nodiscard]] bool HasSamples() const {
    return sampling_report_ != nullptr && sampling_report_->HasSamples();
  }

  void SetInspection();

 signals:
  void LeaveCallstackInspectionClicked();

  // TODO(https://github.com/google/orbit/issues/4589): Connect slots via code and not via UI files,
  // and remove the "public slots" specifier
 private slots:
  void on_NextCallstackButton_clicked();
  void on_PreviousCallstackButton_clicked();
  void OnCurrentThreadTabChanged(int current_tab_index);

  // TODO(https://github.com/google/orbit/issues/4589): Remove redundant "private" once slots is not
  // needed anymore above.
 private:  // NOLINT(readability-redundant-access-specifiers)
  Ui::OrbitSamplingReport* ui_;
  std::shared_ptr<SamplingReport> sampling_report_;
  std::vector<OrbitDataViewPanel*> orbit_data_views_;
};

#endif  // ORBIT_QT_ORBIT_SAMPLING_REPORT_H_
