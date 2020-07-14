#include "LiveFunctions.h"

#include <utility>

#include "TimeGraph.h"

std::pair<uint64_t, uint64_t> ComputeMinMaxTime(
    const std::vector<TextBox*> text_boxes) {
  uint64_t min_time = std::numeric_limits<uint64_t>::max();
  uint64_t max_time = std::numeric_limits<uint64_t>::min();
  for (size_t k = 0; k < text_boxes.size(); ++k) {
    min_time = std::min(min_time, text_boxes[k]->GetTimer().m_Start);
    max_time = std::max(max_time, text_boxes[k]->GetTimer().m_End);
  }
  return std::make_pair(min_time, max_time);
}

void LiveFunctions::Move() {
  auto min_max = ComputeMinMaxTime(current_textboxes_);
  GCurrentTimeGraph->Zoom(min_max.first, min_max.second);
  GCurrentTimeGraph->SetCurrentTextBoxes(current_textboxes_);
}

void LiveFunctions::OnAllNextButton() {
  std::vector<TextBox*> next_boxes;
  for (size_t k = 0; k < function_iterators_.size(); ++k) {
    TextBox* box = live_functions_data_view_.FindNext(
        *(function_iterators_[k]), current_textboxes_[k]->GetTimer().m_End);
    if (box == nullptr) {
      return;
    }
    next_boxes.push_back(box);
  }
  // We only want to commit to the new boxes when all boxes can be moved.
  current_textboxes_ = next_boxes;
  Move();
}

void LiveFunctions::OnAllPreviousButton() {
  std::vector<TextBox*> next_boxes;
  for (size_t k = 0; k < function_iterators_.size(); ++k) {
    TextBox* box = live_functions_data_view_.FindPrevious(
        *(function_iterators_[k]), current_textboxes_[k]->GetTimer().m_End);
    if (box == nullptr) {
      return;
    }
    next_boxes.push_back(box);
  }

  // We only want to commit to the new boxes when all boxes can be moved.
  current_textboxes_ = next_boxes;
  Move();
}

void LiveFunctions::OnNextButton(size_t index) {
  TextBox* text_box = live_functions_data_view_.FindNext(
      *(function_iterators_[index]),
      current_textboxes_[index]->GetTimer().m_End);
  // If text_box is nullptr, then we have reached the right end of the timeline.
  if (text_box != nullptr) {
    current_textboxes_[index] = text_box;
    Move();
  }
}
void LiveFunctions::OnPreviousButton(size_t index) {
  TextBox* text_box = live_functions_data_view_.FindPrevious(
      *(function_iterators_[index]),
      current_textboxes_[index]->GetTimer().m_Start);
  // If text_box is nullptr, then we have reached the left end of the timeline.
  if (text_box != nullptr) {
    current_textboxes_[index] = text_box;
    Move();
  }
}

void LiveFunctions::AddIterator(Function* function, TextBox* current_textbox) {
  function_iterators_.push_back(function);
  current_textboxes_.push_back(current_textbox);
  if (add_iterator_callback_) {
    add_iterator_callback_(function);
  }
  Move();
}