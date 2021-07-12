// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/TimerChain.h"

#include <algorithm>

#include "ClientData/TextBox.h"
#include "capture_data.pb.h"

bool orbit_client_data::TimerBlock::Intersects(uint64_t min, uint64_t max) const {
  return (min <= max_timestamp_ && max >= min_timestamp_);
}

orbit_client_data::TimerChain::~TimerChain() {
  // Find last block in chain
  while (current_->next_ != nullptr) {
    current_ = current_->next_;
  }

  orbit_client_data::TimerBlock* prev = current_;
  while (prev != nullptr) {
    prev = current_->prev_;
    delete current_;
    current_ = prev;
  }
}

orbit_client_data::TimerBlock* orbit_client_data::TimerChain::GetBlockContaining(
    const orbit_client_data::TextBox* element) const {
  orbit_client_data::TimerBlock* block = root_;
  while (block != nullptr) {
    uint32_t size = block->size();
    if (size != 0) {
      orbit_client_data::TextBox* begin = &block->data_[0];
      orbit_client_data::TextBox* end = &block->data_[size - 1];
      if (begin <= element && end >= element) {
        return block;
      }
    }
    block = block->next_;
  }

  return nullptr;
}

orbit_client_data::TextBox* orbit_client_data::TimerChain::GetElementAfter(
    const orbit_client_data::TextBox* element) const {
  orbit_client_data::TimerBlock* block = GetBlockContaining(element);
  if (block != nullptr) {
    orbit_client_data::TextBox* begin = &block->data_[0];
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

orbit_client_data::TextBox* orbit_client_data::TimerChain::GetElementBefore(
    const orbit_client_data::TextBox* element) const {
  orbit_client_data::TimerBlock* block = GetBlockContaining(element);
  if (block != nullptr) {
    orbit_client_data::TextBox* begin = &block->data_[0];
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
