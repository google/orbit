#include "LiveFunctions.h"

#include "TimeGraph.h"

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

  uint64_t min_time = std::numeric_limits<uint64_t>::max();
  uint64_t max_time = std::numeric_limits<uint64_t>::min();
  for (size_t k = 0; k < current_textboxes_.size(); ++k) {
    current_textboxes_[k] = next_boxes[k];
    min_time = std::min(min_time, current_textboxes_[k]->GetTimer().m_Start);
    max_time = std::max(max_time, current_textboxes_[k]->GetTimer().m_Start);
  }

  GCurrentTimeGraph->Zoom(min_time, max_time);
  GCurrentTimeGraph->SetCurrentTextBoxes(next_boxes);
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

  uint64_t min_time = std::numeric_limits<uint64_t>::max();
  uint64_t max_time = std::numeric_limits<uint64_t>::min();
  for (size_t k = 0; k < current_textboxes_.size(); ++k) {
    current_textboxes_[k] = next_boxes[k];
    min_time = std::min(min_time, current_textboxes_[k]->GetTimer().m_Start);
    max_time = std::max(max_time, current_textboxes_[k]->GetTimer().m_Start);
  }

  GCurrentTimeGraph->Zoom(min_time, max_time);
  GCurrentTimeGraph->SetCurrentTextBoxes(next_boxes);
}

void LiveFunctions::OnNextButton(size_t index) {
  TextBox* text_box = live_functions_data_view_.FindNext(
      *(function_iterators_[index]),
      current_textboxes_[index]->GetTimer().m_End);
  // If text_box is nullptr, then we have reached the right end of the timeline.
  if (text_box != nullptr) {
    live_functions_data_view_.JumpToBox(text_box);
    current_textboxes_[index] = text_box;
    GCurrentTimeGraph->SetCurrentTextBoxes({text_box});
  }
}
void LiveFunctions::OnPreviousButton(size_t index) {
  TextBox* text_box = live_functions_data_view_.FindPrevious(
      *(function_iterators_[index]),
      current_textboxes_[index]->GetTimer().m_Start);
  // If text_box is nullptr, then we have reached the left end of the timeline.
  if (text_box != nullptr) {
    live_functions_data_view_.JumpToBox(text_box);
    current_textboxes_[index] = text_box;
    GCurrentTimeGraph->SetCurrentTextBoxes({text_box});
  }
}