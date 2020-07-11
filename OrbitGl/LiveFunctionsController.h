// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIVE_FUNCTIONS_H_
#define LIVE_FUNCTIONS_H_

#include <functional>

#include "absl/container/flat_hash_map.h"

#include "LiveFunctionsDataView.h"
#include "Profiling.h"
#include "TextBox.h"
#include "capture.pb.h"

class LiveFunctionsController {
 public:
  LiveFunctionsController() : live_functions_data_view_(this) {}

  LiveFunctionsDataView& GetDataView() { return live_functions_data_view_; }

  bool OnAllNextButton();
  bool OnAllPreviousButton();

  void OnNextButton(uint64_t id);
  void OnPreviousButton(uint64_t id);
  void OnDeleteButton(uint64_t id);

  void OnDataChanged() { live_functions_data_view_.OnDataChanged(); }

  void SetAddIteratorCallback(
      std::function<void(uint64_t, Function*)> callback) {
    add_iterator_callback_ = callback;
  }

  TickType GetCaptureMin();
  TickType GetCaptureMax();
  TickType GetStartTime(uint64_t index);

  void AddIterator(Function* function);

 private:
  void Move();

  LiveFunctionsDataView live_functions_data_view_;

  absl::flat_hash_map<uint64_t, Function*> function_iterators_;
  absl::flat_hash_map<uint64_t, const TextBox*> current_textboxes_;

  std::function<void(uint64_t, Function*)> add_iterator_callback_;

  uint64_t next_iterator_id_ = 0;

  uint64_t id_to_select_ = 0;
};

#endif  // LIVE_FUNCTIONS_H_