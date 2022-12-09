// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_LIVE_FUNCTIONS_H_
#define ORBIT_QT_ORBIT_LIVE_FUNCTIONS_H_

#include <absl/container/flat_hash_map.h>
#include <stdint.h>

#include <QLineEdit>
#include <QObject>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ClientData/FunctionInfo.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeStatsCollection.h"
#include "OrbitGl/LiveFunctionsController.h"
#include "OrbitQt/orbiteventiterator.h"
#include "OrbitQt/types.h"
#include "Statistics/Histogram.h"

namespace Ui {
class OrbitLiveFunctions;
}

class OrbitApp;

class OrbitLiveFunctions : public QWidget {
  Q_OBJECT

 public:
  explicit OrbitLiveFunctions(QWidget* parent = nullptr);
  ~OrbitLiveFunctions() override;

  void Initialize(OrbitApp* app, SelectionType selection_type, FontType font_type);
  void Deinitialize();
  void Refresh();
  void OnDataChanged();
  void OnRowSelected(std::optional<int> row);
  void Reset();
  void SetFilter(const QString& filter);
  void AddIterator(uint64_t id, const orbit_client_data::FunctionInfo* function);
  QLineEdit* GetFilterLineEdit();
  std::optional<LiveFunctionsController*> GetLiveFunctionsController() {
    return live_functions_ ? &live_functions_.value() : nullptr;
  }
  void ShowHistogram(const std::vector<uint64_t>* data, std::string scope_name,
                     std::optional<orbit_client_data::ScopeId> scope_id);
  void SetScopeStatsCollection(
      std::shared_ptr<const orbit_client_data::ScopeStatsCollection> scope_collection);

 signals:
  void SignalSelectionRangeChange(std::optional<orbit_statistics::HistogramSelectionRange>) const;

 private:
  Ui::OrbitLiveFunctions* ui_;
  std::optional<LiveFunctionsController> live_functions_;
  absl::flat_hash_map<uint64_t, OrbitEventIterator*> iterator_uis_;
  OrbitEventIterator* all_events_iterator_ = nullptr;
};

#endif  // ORBIT_QT_ORBIT_LIVE_FUNCTIONS_H_
