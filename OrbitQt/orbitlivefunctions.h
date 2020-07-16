// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LIVE_FUNCTIONS_
#define ORBIT_LIVE_FUNCTIONS_

#include <QWidget>
#include <QLineEdit>

#include "absl/container/flat_hash_map.h"

#include "types.h"
#include "LiveFunctions.h"
#include "OrbitEventIterator.h"
#include "OrbitFunction.h"

namespace Ui {
class OrbitLiveFunctions;
}

class OrbitLiveFunctions : public QWidget {
  Q_OBJECT

 public:
  explicit OrbitLiveFunctions(QWidget* parent = nullptr);
  ~OrbitLiveFunctions() override;

  void Initialize(SelectionType selection_type,
                  FontType font_type, bool is_main_instance = true);
  void Refresh();
  void OnDataChanged();
  void SetFilter(const QString& a_Filter);
  void AddIterator(uint64_t id, Function* function);
  QLineEdit* GetFilterLineEdit();

 private:
  Ui::OrbitLiveFunctions* ui;
  LiveFunctions live_functions_;
  absl::flat_hash_map<uint64_t, OrbitEventIterator*> iterator_uis;
  OrbitEventIterator* all_events_iterator_;
};

#endif  // ORBIT_LIVE_FUNCTIONS_