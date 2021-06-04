// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TimerChain.h"

#include <algorithm>

#include "capture_data.pb.h"

bool TimerBlock::Intersects(uint64_t min, uint64_t max) const {
  return (min <= max_timestamp_ && max >= min_timestamp_);
}

TimerChain::~TimerChain() {
  // Find last block in chain
  while (current_->next_ != nullptr) {
    current_ = current_->next_;
  }

  TimerBlock* prev = current_;
  while (prev != nullptr) {
    prev = current_->prev_;
    delete current_;
    current_ = prev;
  }
}

TimerBlock* TimerChain::GetBlockContaining(const TextBox* element) const {
  TimerBlock* block = root_;
  while (block != nullptr) {
    uint32_t size = block->size();
    if (size != 0) {
      TextBox* begin = &block->data_[0];
      TextBox* end = &block->data_[size - 1];
      if (begin <= element && end >= element) {
        return block;
      }
    }
    block = block->next_;
  }

  return nullptr;
}

TextBox* TimerChain::GetElementAfter(const TextBox* element) const {
  TimerBlock* block = GetBlockContaining(element);
  if (block != nullptr) {
    TextBox* begin = &block->data_[0];
    uint32_t index = element - begin;
    if (index < block->size() - 1) {
      return &block->data_[++index];
    }
    if (block->next_ != nullptr && block->next_->size() != 0) {
      return &block->next_->data_[0];
    }
  }
  return nullptr;
}

TextBox* TimerChain::GetElementBefore(const TextBox* element) const {
  TimerBlock* block = GetBlockContaining(element);
  if (block != nullptr) {
    TextBox* begin = &block->data_[0];
    uint32_t index = element - begin;
    if (index > 0) {
      return &block->data_[--index];
    }
    if (block->prev_ != nullptr) {
      return &block->prev_->data_[block->prev_->size() - 1];
    }
  }
  return nullptr;
}
