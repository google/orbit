// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIVE_FUNCTIONS_H_
#define LIVE_FUNCTIONS_H_

#include <functional>

#include "LiveFunctionsDataView.h"
#include "OrbitFunction.h"
#include "TextBox.h"
#include "absl/container/flat_hash_map.h"

class LiveFunctions {
 public:
  LiveFunctions() : live_functions_data_view_(this) {}

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

  void AddIterator(Function* function, TextBox* current_textbox);

 private:
  void Move();

  LiveFunctionsDataView live_functions_data_view_;

  absl::flat_hash_map<uint64_t, Function*> function_iterators_;
  absl::flat_hash_map<uint64_t, TextBox*> current_textboxes_;

  std::function<void(uint64_t, Function*)> add_iterator_callback_;

  uint64_t next_id = 0;
};

#endif  // LIVE_FUNCTIONS_H_