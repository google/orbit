// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_LIVE_FUNCTIONS_H_
#define ORBIT_QT_ORBIT_LIVE_FUNCTIONS_H_

#include <stdint.h>

#include <QLineEdit>
#include <QObject>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <optional>

#include "LiveFunctionsController.h"
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"
#include "orbiteventiterator.h"
#include "types.h"

namespace Ui {
class OrbitLiveFunctions;
}

class OrbitApp;

class OrbitLiveFunctions : public QWidget {
  Q_OBJECT

 public:
  explicit OrbitLiveFunctions(QWidget* parent = nullptr);
  ~OrbitLiveFunctions() override;

  void Initialize(OrbitApp* app, orbit_metrics_uploader::MetricsUploader* metrics_uploader,
                  SelectionType selection_type, FontType font_type, bool is_main_instance = true);
  void Deinitialize();
  void Refresh();
  void OnDataChanged();
  void OnRowSelected(std::optional<int> row);
  void Reset();
  void SetFilter(const QString& a_Filter);
  void AddIterator(uint64_t id, const orbit_client_protos::FunctionInfo* function);
  QLineEdit* GetFilterLineEdit();
  std::optional<LiveFunctionsController*> GetLiveFunctionsController() {
    return live_functions_ ? &live_functions_.value() : nullptr;
  }

 private:
  Ui::OrbitLiveFunctions* ui;
  std::optional<LiveFunctionsController> live_functions_;
  absl::flat_hash_map<uint64_t, OrbitEventIterator*> iterator_uis;
  OrbitEventIterator* all_events_iterator_;
};

#endif  // ORBIT_QT_ORBIT_LIVE_FUNCTIONS_H_
