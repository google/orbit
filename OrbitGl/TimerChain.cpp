#include "TimerChain.h"

#include <vector>
#include "TextBox.h"

TimerBlock::TimerBlock()
    : size_(0),
      min_timestamp_(std::numeric_limits<uint64_t>::max()),
      max_timestamp_(0) {}

void TimerBlock::Add(const TextBox& box) {
  if (size_ == kMaxSize) {
    return;
  }

  assert(size_ < kMaxSize);
  data_[size_] = box;
  if (box.GetTimer().m_Start < min_timestamp_) {
    min_timestamp_ = box.GetTimer().m_Start;
  }
  if (box.GetTimer().m_End > max_timestamp_) {
    max_timestamp_ = box.GetTimer().m_End;
  }
  ++size_;
}

bool TimerBlock::Intersects(uint64_t min, uint64_t max) {
  return (min <= max_timestamp_ && max >= min_timestamp_);
}

TimerChain::TimerChain() { blocks_.push_back(TimerBlock()); }

void TimerChain::push_back(const TextBox& box) {
  if (!(blocks_.end() - 1)->AtCapacity()) {
    TimerBlock& current = *(blocks_.end() - 1);
    current.Add(box);
  } else {
    blocks_.push_back(TimerBlock());
    TimerBlock& current = *(blocks_.end() - 1);
    current.Add(box);
  }
}

int TimerChain::GetBlockContaining(const TextBox* element) {
  for (int k = 0; k < blocks_.size(); ++k) {
    auto& block = blocks_[k];
    int size = block.size();
    if (size > 0) {
      TextBox* begin = &block[0];
      TextBox* end = &block[size - 1];
      if (begin <= element && end >= element) {
        return k;
      }
    }
  }
  return blocks_.size();
}

TextBox* TimerChain::GetElementAfter(const TextBox* element) {
  int k = GetBlockContaining(element);
  if (k < blocks_.size()) {
    auto& block = blocks_[k];
    TextBox* begin = &block[0];
    int index = element - begin;
    if (index < block.size() - 1) {
      return &block[index + 1];
    } else if (k + 1 < blocks_.size()) {
      auto& next_block = blocks_[k+1];
      return &next_block[0];
    }
  }
  return nullptr;
}

TextBox* TimerChain::GetElementBefore(const TextBox* element) {
  int k = GetBlockContaining(element);
  if (k < blocks_.size()) {
    auto& block = blocks_[k];
    TextBox* begin = &block[0];
    int index = element - begin;
    if (index > 0) {
      return &block[index - 1];
    } else if (k > 0) {
      auto& previous_block = blocks_[k - 1];
      return &previous_block[previous_block.size() - 1];
    }
  }
  return nullptr;
}