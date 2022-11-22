// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/TimerInfosIterator.h"

TimerInfosIterator& TimerInfosIterator::operator++() {
  // Next timer in block:
  ++timer_index_;
  // Still inside block?
  if (timer_index_ < blocks_it_->size()) {
    return *this;
  }

  // Next block:
  timer_index_ = 0;
  ++blocks_it_;
  // Still inside the chain?
  if (blocks_it_ != (*chains_it_)->end()) {
    return *this;
  }

  // Next chain:
  ++chains_it_;
  // Still inside chains?
  if (chains_it_ != chains_end_it_) {
    blocks_it_ = (*chains_it_)->begin();
    return *this;
  }

  return *this;
}