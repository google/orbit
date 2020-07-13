// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIVE_FUNCTIONS_H_
#define LIVE_FUNCTIONS_H_

#include <functional>

#include "LiveFunctionsDataView.h"
#include "OrbitFunction.h"
#include "TextBox.h"

class LiveFunctions {
 public:
  LiveFunctions() : live_functions_data_view_(this) 
  {}

  void OnNextButton(size_t index) {
      TextBox* text_box =
        live_functions_data_view_.JumpToNext(*(function_iterators_[index]), 
        current_textboxes_[index]->GetTimer().m_End);
      // If text_box is nullptr, then we have reached the right end of the timeline.
      if (text_box != nullptr) {
        current_textboxes_[index] = text_box;
      }
  }
  void OnPreviousButton(size_t index) {
      TextBox* text_box = 
        live_functions_data_view_.JumpToPrevious(*(function_iterators_[index]), 
        current_textboxes_[index]->GetTimer().m_Start);
      // If text_box is nullptr, then we have reached the left end of the timeline.
      if (text_box != nullptr) {
        current_textboxes_[index] = text_box;
      } 
  }

  LiveFunctionsDataView& GetDataView() {
      return live_functions_data_view_;
  }

  void OnDataChanged() {
      live_functions_data_view_.OnDataChanged();
  }

  void SetAddIteratorCallback(std::function<void(Function*)> callback) {
      add_iterator_callback_ = callback;
  }

  void AddIterator(Function* function, TextBox* current_textbox) {
    function_iterators_.push_back(function);
    current_textboxes_.push_back(current_textbox);
    if (add_iterator_callback_) {
      add_iterator_callback_(function);
    }
  }
 private:
  LiveFunctionsDataView live_functions_data_view_;
  std::vector<Function*> function_iterators_;
  std::vector<TextBox*> current_textboxes_;
  std::function<void(Function*)> add_iterator_callback_;
};

#endif  // LIVE_FUNCTIONS_H_