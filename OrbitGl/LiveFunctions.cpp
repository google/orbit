#include "LiveFunctions.h"

#include "TimeGraph.h"

void LiveFunctions::OnNextButton(size_t index) {
  TextBox* text_box = live_functions_data_view_.JumpToNext(
      *(function_iterators_[index]),
      current_textboxes_[index]->GetTimer().m_End);
  // If text_box is nullptr, then we have reached the right end of the timeline.
  if (text_box != nullptr) {
    current_textboxes_[index] = text_box;
    GCurrentTimeGraph->SetCurrentTextBox(text_box);
  }
}
void LiveFunctions::OnPreviousButton(size_t index) {
  TextBox* text_box = live_functions_data_view_.JumpToPrevious(
      *(function_iterators_[index]),
      current_textboxes_[index]->GetTimer().m_Start);
  // If text_box is nullptr, then we have reached the left end of the timeline.
  if (text_box != nullptr) {
    current_textboxes_[index] = text_box;
    GCurrentTimeGraph->SetCurrentTextBox(text_box);
  }
}