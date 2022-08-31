// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/TimerChain.h"

#include <algorithm>

#include "ClientProtos/capture_data.pb.h"

using orbit_client_protos::TimerInfo;

namespace orbit_client_data {

bool TimerBlock::Intersects(uint64_t min, uint64_t max) const {
  return (min <= max_timestamp_ && max >= min_timestamp_);
}

const orbit_client_protos::TimerInfo* TimerBlock::LowerBound(uint64_t min_ns) const {
  auto it = std::lower_bound(data_.begin(), data_.end(), min_ns,
                             [](const orbit_client_protos::TimerInfo& timer_info, uint64_t value) {
                               return timer_info.end() < value;
                             });
  if (it == data_.end()) return nullptr;
  return &*it;
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

const TimerBlock* TimerChain::GetBlockContaining(const TimerInfo& element) const {
  const TimerBlock* block = root_;
  while (block != nullptr) {
    uint32_t size = block->size();
    if (size != 0) {
      const TimerInfo* begin = &block->data_[0];
      const TimerInfo* end = &block->data_[size - 1];
      // TODO (http://b/194268700): Don't compare pointers in TimerChain as it is an undefined
      // behavior
      if (begin <= &element && end >= &element) {
        return block;
      }
    }
    block = block->next_;
  }

  return nullptr;
}

const TimerInfo* TimerChain::GetElementAfter(const TimerInfo& element) const {
  const TimerBlock* block = GetBlockContaining(element);
  if (block != nullptr) {
    const TimerInfo* begin = &block->data_[0];
    uint32_t index = &element - begin;
    if (index < block->size() - 1) {
      return &block->data_[++index];
    }
    if (block->next_ != nullptr && block->next_->size() != 0) {
      return &block->next_->data_[0];
    }
  }
  return nullptr;
}

const TimerInfo* TimerChain::GetElementBefore(const TimerInfo& element) const {
  const TimerBlock* block = GetBlockContaining(element);
  if (block != nullptr) {
    const TimerInfo* begin = &block->data_[0];
    uint32_t index = &element - begin;
    if (index > 0) {
      return &block->data_[--index];
    }
    if (block->prev_ != nullptr) {
      return &block->prev_->data_[block->prev_->size() - 1];
    }
  }
  return nullptr;
}

}  // namespace orbit_client_data
